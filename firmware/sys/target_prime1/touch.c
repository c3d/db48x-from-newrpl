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

#define NT11002_EVENT_START 1
#define NT11002_EVENT_CONTINUE 2
#define NT11002_EVENT_END 3
#define NT11002_EVENT_MASK 0x07
#define NT11002_TRACK_ID_MASK 0xf8

extern unsigned int __cpu_getPCLK();

typedef struct {
    int x, y;
} ts_finger_t;

typedef struct {
    ts_finger_t finger[TS_FINGERS];
} ts_fingers_t;

volatile ts_fingers_t ts_fingers __SYSTEM_GLOBAL__;
volatile int ts_status __SYSTEM_GLOBAL__;

typedef struct ts_t {
    uint8_t addr;
    void (*reset)(struct ts_t const *ts);
    void (*get_properties)(struct ts_t *ts);

    // Returns 0 if no new valid data available
    int (*get_data)(struct ts_t const *ts);

    int resolution_width;
    int resolution_height;
} ts_t;

static ts_t nt11002;
static ts_t *touchscreen;

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
    }

    if (readnum != 0) {
        i2c_ack();

        if (writenum == 0) {
            //  Set Master Receive mode, STOP signal, enable Rx/Tx
            *IICSTAT0 = 0x90;
        }

        // Write slave address
        *IICDS0 = (addr << 1) & 0xff;

        i2c_clear_interrupt();

        // Start read
        *IICSTAT0 = 0xb0;

        i2c_wait_for_interrupt();

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
    }

    // STOP transmission
    *IICSTAT0 &= ~0x20;

    i2c_clear_interrupt();
    busy_wait(2.0 * 80);
}

static void i2c_read(uint8_t command, uint8_t addr, uint8_t *buffer, int len)
{
    buffer[0] = command;
    i2c_write_read(addr, buffer, 1, len);
}

static void i2c_write(uint8_t addr, uint8_t *buffer, int len)
{
    i2c_write_read(addr, buffer, len, 0);
}

static void nt11002_reset(ts_t const *ts)
{
    *GPFDAT |= 1;
    busy_wait(5.0 * 1000 * 80);
    *GPFDAT &= ~1;
    busy_wait(2.5 * 1000 * 80);
    *GPFDAT |= 1;
    busy_wait(24 * 1000 * 80); // FIXME maybe this can be reduced
}

static void nt11002_get_properties(ts_t *ts)
{
    uint8_t buffer[15];

    // First part of this code is just copying what Prime firmware does.

    i2c_read(0x78, ts->addr, buffer, 2);
    // -> 1a e5

    buffer[0] = 0xff;
    buffer[1] = 0x0f;
    buffer[2] = 0xff;
    i2c_write(ts->addr, buffer, 3);

    buffer[0] = 0x00;
    buffer[1] = 0xe1;
    i2c_write(ts->addr, buffer, 2);

    busy_wait(1.0 * 1000 * 1000 * 80);

    buffer[0] = 0xff;
    buffer[1] = 0x0a;
    buffer[2] = 0x0d;
    i2c_write(ts->addr, buffer, 3);

    i2c_read(0x00, ts->addr, buffer, 2);
    // -> 6a 6f

    busy_wait(5.5 * 1000 * 80);

    *GPFDAT &= ~1;
    busy_wait(2.75 * 1000 * 80);
    *GPFDAT |= 1;

    busy_wait(17 * 1000 * 80);

    // Reading device properties

    i2c_read(0x78, ts->addr, buffer, 15);
    // -> 1a e5 0e 0a 01 c0 01 40 01 04 00 00 00 00 00

    ts->resolution_width = (buffer[4] << 8) | (buffer[5]);
    ts->resolution_height = (buffer[6] << 8) | (buffer[7]);
    // max_fingers = buffer[9];
}

static int nt11002_get_data(ts_t const *ts)
{
    int result = 0;
    uint8_t buffer[TS_FINGERS * 6];
    i2c_read(0x00, ts->addr, (uint8_t *)&ts_fingers, sizeof(buffer));
    for (int f = 0; f < TS_FINGERS; ++f) {
        uint8_t *finger = buffer + f * 6;
        uint8_t track_id = ((finger[0] & NT11002_TRACK_ID_MASK) >> 3) - 1;
        uint8_t event = finger[0] & NT11002_EVENT_MASK;
        if (track_id != f) {
            // No event for this finger
            ts_fingers.finger[f].x = TS_FINGER_INVALID;
            ts_fingers.finger[f].y = TS_FINGER_INVALID;
        } else if (event == NT11002_EVENT_START) {
            // Only register first finger contact in a row of events
            int x = ((int)finger[1] << 4) | (finger[3] >> 4);
            int y = ((int)finger[2] << 4) | (finger[3] & 0x0f);
            ts_fingers.finger[f].x = x * SCREEN_WIDTH / ts->resolution_width;
            ts_fingers.finger[f].y = y * SCREEN_HEIGHT / ts->resolution_height;
            result = 1;
        } // Ignore if it's not the first event for this finger
    }
    return result;
}

// IRQ handler
// Read all registers from the touchscreen
static void ts_update()
{
    if (touchscreen->get_data(touchscreen)) {
        if (ts_status & TS_STAT_DATAREADY) {
            ts_status |= TS_STAT_DATAMISSED;
        }
        ts_status |= TS_STAT_DATAREADY;
    }
}

void ts_init()
{
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
    nt11002.addr = 0x01;
    nt11002.reset = nt11002_reset;
    nt11002.get_properties = nt11002_get_properties;
    nt11002.get_data = nt11002_get_data;

    // FIXME Is there another model? implement controller and selection
    touchscreen = &nt11002;

    touchscreen->reset(touchscreen);
    touchscreen->get_properties(touchscreen);

    // Initialize finger data
    for (int i = 0; i < TS_FINGERS; ++i) {
        ts_fingers.finger[i].x = TS_FINGER_INVALID;
        ts_fingers.finger[i].y = TS_FINGER_INVALID;
    }

    __irq_addhook(2, &ts_update);
    __irq_unmask(2);
}

extern INTERRUPT_TYPE __cpu_intoff();
extern void __cpu_inton(INTERRUPT_TYPE);

uint32_t ts_get_finger(void)
{
    uint32_t result;
    INTERRUPT_TYPE intsave;

    intsave = __cpu_intoff();

    if ((ts_status & TS_STAT_DATAREADY) && (ts_fingers.finger[0].x != TS_FINGER_INVALID)) {
        result = ts_fingers.finger[0].x << 16 | ts_fingers.finger[0].y;
        ts_status &= ~(TS_STAT_DATAREADY | TS_STAT_DATAMISSED);
    } else {
        result = TS_FINGER_INVALID;
    }

    __cpu_inton(intsave);

    return result;
}
