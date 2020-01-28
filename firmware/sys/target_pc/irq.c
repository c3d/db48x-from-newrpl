/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// IRQ AND CPU LOW LEVEL

// NOT NEEDED FOR NOW

unsigned int __saveint;

void qt_mutex_lock(void);
void qt_mutex_unlock(void);

void mutex_lock(void)
{
	qt_mutex_lock();
}

void mutex_unlock(void)
{
	qt_mutex_unlock();
}
