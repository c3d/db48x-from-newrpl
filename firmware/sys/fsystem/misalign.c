/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

// MISALIGNED READ/WRITE 16/32-BIT WORDS

unsigned int ReadInt32(unsigned char *ptr)
{
    return (ptr[0]) | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
}

void WriteInt32(unsigned char *ptr, unsigned int value)
{
    *ptr++ = value & 0xff;
    *ptr++ = (value >> 8) & 0xff;
    *ptr++ = (value >> 16) & 0xff;
    *ptr = (value >> 24) & 0xff;
}

unsigned int ReadInt16(unsigned char *ptr)
{
    return (ptr[0]) | (ptr[1] << 8);
}

void WriteInt16(unsigned char *ptr, unsigned int value)
{
    *ptr++ = value & 0xff;
    *ptr = (value >> 8) & 0xff;
}
