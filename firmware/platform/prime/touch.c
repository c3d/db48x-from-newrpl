/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

// HP Prime touchscreen driver for Novatek NT11002

// From datasheet:
// Minimum time for clock low or clock high is 1250 ns

// Minimum time for Start = 600 ns

// Lines used by the touchscreen:

// GPE14 is configured as IICSCL
// GPE15 is configured as IICSDA
// GPF2 is configured as EINT[2] with which the slave signals to the master that data is available (INT line of NT11002)
// GPF0 probably VCC
// Where is the RESET pin for the Goodix controller?

#define NT11002_EVENT_START 1
#define NT11002_EVENT_CONTINUE 2
#define NT11002_EVENT_END 3
#define NT11002_EVENT_MASK 0x07
#define NT11002_TRACK_ID_MASK 0xf8

#define FINGER_STATE_START  128

extern unsigned int cpu_getPCLK();


typedef struct ts_t {
    uint8_t addr;
    void (*reset)(struct ts_t *ts);
    void (*get_properties)(struct ts_t *ts);

    // Returns 0 if no new valid data available
    int (*get_data)(struct ts_t *ts);

    int adjust_width;
    int adjust_height;
    int detected;
} ts_t;

volatile int ts_status SYSTEM_GLOBAL;
uint8_t ts_buffer[256] SYSTEM_GLOBAL;
ts_t touchscreen SYSTEM_GLOBAL;

static inline void i2c_wait_for_interrupt(void)
{
    while (! (*IICCON0 & 0x10));
}

static inline void i2c_clear_interrupt(void)
{
    *IICCON0 &= ~0x10;
}

static inline void i2c_ack(void)
{
    *IICCON0 |= 0x80;
}

static inline void i2c_nack(void)
{
    *IICCON0 &= ~0x80;
}

// NOTE is identically implemented in nand.c
// count = 80 is 1µs
static void __attribute__ ((noinline)) busy_wait(unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i) {
        // asm statement with data dependency and potential side effect
        // can't be optimized away
        __asm__ volatile("" : "+g" (i) : :);
    }
}

// Atomically writes and then reads data.
// Bytes to write start at data[0] and are overwritten by reads which start
// also at data[0]
static void i2c_write_read(uint8_t const addr, uint8_t * const data, int const writenum, int const readnum)
{
    int counter;
    uint8_t *pointer;


    if (writenum != 0) {
        // Set Master Transmit mode, STOP signal, enable Rx/Tx
        *IICSTAT0 = 0xd0;

        //Enable releasing the SDA line so slave can send ACK
        i2c_ack();

        // Write slave address
        *IICDS0 = (addr << 1) & 0xff;

        i2c_clear_interrupt();

        // Start write
        *IICSTAT0 = 0xf0;

        i2c_wait_for_interrupt();

        counter = writenum;
        pointer = data;
        while (counter > 0) {
            *IICDS0 = *pointer;
            ++pointer;
            --counter;

            i2c_clear_interrupt();
            i2c_wait_for_interrupt();
        }

        // Set Master Transmit mode, STOP signal, enable Rx/Tx
        *IICSTAT0 = 0xd0;
        i2c_clear_interrupt();

        while(*IICSTAT0&0x20);  // Wait for the STOP condition to make the bus free
    }


    if (readnum != 0) {

        // Make sure ACK generation is enabled
        i2c_ack();

        //  Set Master Receive mode, STOP signal, enable Rx/Tx
        *IICSTAT0 = 0x90;

        // Write slave address
        *IICDS0 = (addr << 1) & 0xff;

        i2c_clear_interrupt();

        // Start read
        *IICSTAT0 = 0xb0;

        i2c_wait_for_interrupt();   // This interrupt signals the ADDR was transmitted

        i2c_clear_interrupt();      // Start the first byte receive cycle

        i2c_wait_for_interrupt();   // This interrupt will happen after the first byte was received

        counter = readnum;
        pointer = data;
        while(counter > 0) {
            *pointer = *IICDS0;
            ++pointer;
            --counter;

            if (counter == 0) {
                i2c_nack();
            }

            i2c_clear_interrupt();
            i2c_wait_for_interrupt();
        }

        // STOP transmission
        *IICSTAT0 &= ~0x20;

        i2c_clear_interrupt();

        while(*IICSTAT0&0x20);  // Wait for the STOP condition to make the bus free
    }


}

static void i2c_read(unsigned int command, uint8_t addr, uint8_t *buffer, int len)
{
    // Special case of 16-bit commands
    if(command&0xff00) {
        buffer[0]=(command&0xff00)>>8;
        buffer[1]=command&0xff;
        i2c_write_read(addr, buffer, 2, len);
    }
    // Typical case of 8-bit commands
    else {
        buffer[0]=command&0xff;
        i2c_write_read(addr, buffer, 1, len);
    }
}

static void i2c_write(uint8_t addr, uint8_t *buffer, int len)
{
    i2c_write_read(addr, buffer, len, 0);
}

static void nt11002_reset(ts_t *ts)
{
    *GPFDAT |= 1;
    busy_wait(5.0 * 1000 * 80);
    *GPFDAT &= ~1;
    busy_wait(2.5 * 1000 * 80);
    *GPFDAT |= 1;
    busy_wait(24 * 1000 * 80); // FIXME maybe this can be reduced

    ts->addr = 0x01;
}

static void nt11002_get_properties(ts_t *ts)
{
    // Reading device properties
    for(int k=0;k<15;++k) ts_buffer[k]=0;

    i2c_read(0x78, ts->addr, ts_buffer, 15);
    // -> 1a e5 0e 0a 01 c0 01 40 01 04 00 00 00 00 00



    ts->adjust_width = (ts_buffer[4] << 8) | (ts_buffer[5]);
    ts->adjust_height = (ts_buffer[6] << 8) | (ts_buffer[7]);

    // If there was no answer, the buffer will contain all the same bytes
    if(ts->adjust_width == ts->adjust_height) ts->detected = 0;
    else ts->detected = 1;

    ts->adjust_width = ( LCD_W * 1024 ) / ts->adjust_width;
    ts->adjust_height = ( LCD_H * 1024 ) / ts->adjust_height;
    // max_fingers = buffer[9];
}

extern void keyb_irq_postmsg(unsigned int msg);

static int nt11002_get_data(ts_t *ts)
{
    int result = 0;

    i2c_read(0x00, ts->addr, (uint8_t *)&ts_buffer, 1+6*TS_FINGERS);
    for (int f = 0; f < TS_FINGERS; ++f) {
        uint8_t *finger = ts_buffer + f * 6;

        uint8_t track_id = ((finger[0] & NT11002_TRACK_ID_MASK) >> 3);     // FINGER ID, First finger = 1 not zero
        uint8_t event = finger[0] & NT11002_EVENT_MASK;                    // EVENT, 1 = Down, 2 = Move, 3 = Up
        int x = (((int)finger[1]) << 4) | (finger[3] >> 4);
        int y = (((int)finger[2]) << 4) | (finger[3] & 0x0f);
        x = ( x * ts->adjust_width ) >> 10;
        y = ( y * ts->adjust_height) >> 10;

        if((track_id>0)&&(track_id<=TS_FINGERS)) {
            keyb_postmsg(KM_MAKETOUCHMSG((((int)event)<<30),((int)track_id),x,y));
            result = 1; // Return 1 if there were any valid touch events
        }

    }

    return result;
}


static void gt9137_reset(ts_t *ts)
{

    // Cut power to the chip
    *GPFDAT &= ~1;

    // Set INT (GPF2) as Output LOW
    *GPFCON = (*GPFCON & ~0x30) | 0x10;
    *GPFDAT &= ~0x4;

    // Power up chip
    *GPFDAT |= 1;

    busy_wait(11 * 1000 * 80);  // Wait 11 ms (min. 10ms+100 us)

    // DRIVE RESET LINE UP (BUT WHERE IS IT?)

    busy_wait(55 * 1000 * 80); // Wait 50 ms + 5ms

    // Set INT (GPF2) as EINT2
    *GPFCON = (*GPFCON & ~0x30) | 0x20;


    // And we should be ready to go at address 0x5d
    ts->addr = 0x5d;
    ts_buffer[FINGER_STATE_START]=0;


}

static void gt9137_get_properties(ts_t *ts)
{
    // Reading device properties
    for(int k=0;k<11;++k) ts_buffer[k]=0;

    i2c_read(0x8140, ts->addr, ts_buffer, 11);

    ts->adjust_width = (ts_buffer[7] << 8) | (ts_buffer[6]);
    ts->adjust_height = (ts_buffer[9] << 8) | (ts_buffer[8]);

    // If there was no answer, the buffer will contain all the same bytes
    if(ts->adjust_width == ts->adjust_height) ts->detected = 0;
    else ts->detected = 1;

    ts->adjust_width = ( LCD_W * 1024 ) / ts->adjust_width;
    ts->adjust_height = ( LCD_H * 1024 ) / ts->adjust_height;
    // max_fingers = buffer[9];
}

static int gt9137_get_data(ts_t *ts)
{
    int result = 0;

    i2c_read(0x814E, ts->addr, (uint8_t *)&ts_buffer, 1);
    if(ts_buffer[0]&0x80) {    // Check if buffer status indicates data is ready
        int nfingers = ts_buffer[0]&0xf;
        if(nfingers > TS_FINGERS) nfingers = TS_FINGERS;

        if(nfingers > 0) {

        i2c_read(0x814f, ts->addr, (uint8_t *)&ts_buffer, nfingers*8);

        for (int f = 0; f < TS_FINGERS; ++f) {

            uint8_t *finger = ts_buffer + f * 8;

            uint8_t track_id = (f>=nfingers)? f:finger[0];

            // Keep state of the fingers to simulate the same events as the NT11002

            uint8_t state = ts_buffer[FINGER_STATE_START] & (1<<track_id);

            if(f>=nfingers) {

                if(state) {
                // This finger was lifted

                uint8_t event = 3;                    // EVENT, 1 = Down, 2 = Move, 3 = Up

                int x = ts_buffer[FINGER_STATE_START+1+track_id*4]  | ((int)ts_buffer[FINGER_STATE_START+2+track_id*4] << 8);
                int y = ts_buffer[FINGER_STATE_START+3+track_id*4]  | ((int)ts_buffer[FINGER_STATE_START+4+track_id*4] << 8);

                x = ( x * ts->adjust_width ) >> 10;
                y = ( y * ts->adjust_height) >> 10;

                keyb_postmsg(KM_MAKETOUCHMSG((((int)event)<<30),((int)(track_id+1)),x,y));

                // And clear the state, this finger was lifted

                ts_buffer[FINGER_STATE_START] ^= state;

                }

                // Otherwise do nothing

            }
            else {

            // This is an actual new event from the touchscreen
            uint8_t event = (state? 2:1);                    // EVENT, 1 = Down, 2 = Move, 3 = Up


            // Save state and last known coordinate to simulate the UP event
            ts_buffer[FINGER_STATE_START] |= (1<<track_id);

            ts_buffer[FINGER_STATE_START+1+track_id*4]=finger[1];
            ts_buffer[FINGER_STATE_START+2+track_id*4]=finger[2];
            ts_buffer[FINGER_STATE_START+3+track_id*4]=finger[3];
            ts_buffer[FINGER_STATE_START+4+track_id*4]=finger[4];

            int x = finger[1]  | ((int)finger[2] << 8);
            int y = finger[3]  | ((int)finger[4] << 8);

            // There's also touch point size information, but not sure if it would be useful or not

            x = ( x * ts->adjust_width ) >> 10;
            y = ( y * ts->adjust_height) >> 10;

            keyb_postmsg(KM_MAKETOUCHMSG((((int)event)<<30),((int)(track_id+1)),x,y));
            }

        }
        }

        else {

            for (int f = 0; f < TS_FINGERS; ++f) {

                uint8_t track_id = f;

                uint8_t state = ts_buffer[FINGER_STATE_START] & (1<<track_id);

                    if(state) {
                    // This finger was lifted

                    uint8_t event = 3;                    // EVENT, 1 = Down, 2 = Move, 3 = Up

                    int x = ts_buffer[FINGER_STATE_START+1+track_id*4]  | ((int)ts_buffer[FINGER_STATE_START+2+track_id*4] << 8);
                    int y = ts_buffer[FINGER_STATE_START+3+track_id*4]  | ((int)ts_buffer[FINGER_STATE_START+4+track_id*4] << 8);

                    x = ( x * ts->adjust_width ) >> 10;
                    y = ( y * ts->adjust_height) >> 10;

                    keyb_postmsg(KM_MAKETOUCHMSG((((int)event)<<30),((int)(track_id+1)),x,y));

                    // And clear the state, this finger was lifted

                    ts_buffer[FINGER_STATE_START] ^= state;

                    }

                    // Otherwise do nothing



            }


        }
    }
    ts_buffer[0]=0x81;
    ts_buffer[1]=0x4e;
    ts_buffer[2]=0;    // Clear buffer status bit
    i2c_write(ts->addr,(uint8_t *)&ts_buffer,3);
    return result;
}




// IRQ handler
// Read all registers from the touchscreen
static void ts_update()
{
    // Make sure the system was initialized before calling this function
    if(!(ts_status&TS_STAT_INIT)) return;

    touchscreen.get_data(&touchscreen);
}

void ts_init()
{
    // Make sure EINT2 is masked
    irq_mask(2);

    // Set GPE14 and 15 for IIC function
    *GPECON = (*GPECON & ~0xf0000000) | 0xA0000000;
    // Disable Pull ups/down on both lines
    *GPEUDP &= ~0xf0000000;

    // Set GPF0 as output, GPF2 as EINT2
    *GPFCON = (*GPFCON & ~0x33) | 0x21;
    // Disable pull up/down
    *GPFUDP &= ~0x33;

    // Trigger EINT2 on falling edge
    *EXTINT0 = (*EXTINT0 & ~0x0700) | 0x0200;


    // With PCLK = 50 MHz, use IICCLK = PCLK/16 = 3.125 MHz
    // and Tx Clock = IICCLK / 8 = 390 kHz (NT11002 datasheet says Max = 400 kHz)
    // Enable ACK bit
    *IICCON0 = 0xA7;

    *IICADD0 = 0x10;

    // Slave receive mode, enable Rx/Tx
    *IICSTAT0 = 0x10;

    // Disable filter
    *IICLC0 = 0x0;

    ts_status=0;

    // Initialize structure for NT11002
    touchscreen.detected = 0;
    touchscreen.reset = &nt11002_reset;
    touchscreen.get_properties = &nt11002_get_properties;
    touchscreen.get_data = &nt11002_get_data;

    touchscreen.reset(&touchscreen);
    touchscreen.get_properties(&touchscreen);

    if(!touchscreen.detected) {
        // Failed to detect the controller, try with the Goodix GT9137

        // Initialize structure for GT9137
        touchscreen.detected = 0;
        touchscreen.reset = &gt9137_reset;
        touchscreen.get_properties = &gt9137_get_properties;
        touchscreen.get_data = &gt9137_get_data;

        touchscreen.reset(&touchscreen);
        touchscreen.get_properties(&touchscreen);


    }

    if(touchscreen.detected) {
        ts_status = TS_STAT_INIT;
        irq_addhook(2, &ts_update);
        irq_clrpending(2);
        irq_unmask(2);
    }
}

extern INTERRUPT_TYPE cpu_intoff_nosave();
extern void cpu_inton_nosave(INTERRUPT_TYPE);

