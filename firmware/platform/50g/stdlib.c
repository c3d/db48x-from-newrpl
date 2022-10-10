/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <host/pc/stdlib.c>
#include <recorder.h>
#include <stdio.h>

FLIGHT_RECORDER(stdlib_error, "Erroneous calls to stdlib stubs, set to 2 to abort");
#define ABORT() do  { if (RECORDER_TWEAK(stdlib_error) == 2) abort(); } while(0)

// Stubs for functions we should never call, even indirectly
void *_sbrk(int incr)
{
    record(stdlib_error, "sbrk(%d) will return NULL", incr);
    ABORT();
    return NULL;
}

ssize_t _read(int fildes, void *buf, size_t nbytes)
{
    record(stdlib_error, "read(%d,%p,%u) will return 0", fildes, buf, nbytes);
    ABORT();
    return 0;
}

ssize_t _write(int fildes, const void *buf, size_t nbytes)
{
    record(stdlib_error, "write(%d,%p,%u) will return 0", fildes, buf, nbytes);
    ABORT();
    return 0;
}

int _close(int fildes)
{
    record(stdlib_error, "close(%d) will return 0", fildes);
    ABORT();
    return 0;
}

off_t _lseek(int fildes, off_t offset, int whence)
{
    record(stdlib_error, "lseek(%d, %u, %d) will return 0", fildes, offset, whence);
    ABORT();
    return 0;
}

struct stat;
int _fstat(int fildes, struct stat *buf)
{
    record(stdlib_error, "fstat(%d, %p) will return 0", fildes, buf);
    ABORT();
    return 0;
}

int _isatty(int fd)
{
    record(stdlib_error, "isatty(%d) will return 0", fd);
    ABORT();
    return 0;
}
