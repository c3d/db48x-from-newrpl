/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "target_prime1.h"

// NOTE is identically implemented in nand.c
static void uart_busy_wait(unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i) {
        // asm statement with data dependency and potential side effect
        // can't be optimized away
        __asm__ volatile("" : "+g" (i) : :);
    }
}

void uart_init(void)
{
    // Enable RXD[0] and TXD[0]
    *GPHCON &= ~0xf;
    *GPHCON |= 0xa;

    *UFCON0 = 0; // No FIFO
    *UMCON0 = 0; // No AFC
    *ULCON0 = 3; // 8N1
    *UCON0 = 0x245; // Polling mode; generate error status interrupt
    *UBRDIV0 = 26; // Baud rate 115.2k; FIXME PRIME.OS uses 35 to get same value 
    *UDIVSLOT0 = 0;

    uart_busy_wait(500);
}

static void uart_write_char(char character)
{
    // Wait until Transmit buffer is empty
    while ((*UTRSTAT0 & 2) == 0);
    uart_busy_wait(100);
    *UTXH0 = character;
}

static void uart_write_string(const char *string)
{
    while (*string != 0) {
        uart_write_char(*string);
        ++string;
    }
    uart_busy_wait(100);
}

static void tohex(uint32_t value, char *buffer)
{
    buffer[8] = 0;

    for (int i = 7; i >= 0; --i) {
        buffer[i] = "0123456789ABCDEF"[value % 16];
        value = value / 16;
    }
}

void debug_print_hex(const char *key, uint32_t value)
{
    char value_string[9];
    tohex(value, value_string);
    uart_write_string(key);
    uart_write_string(":0x");
    uart_write_string(value_string);
    // write carriage return and newline to be compatible to BL1
    uart_write_string("\r\n");
}

void debug_print(const char *string)
{
    uart_write_string(string);
}
