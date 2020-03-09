/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

void usb_mutex_lock_implementation(void);
void usb_mutex_unlock_implementation(void);

void usb_mutex_lock(void)
{
	usb_mutex_lock_implementation();
}

void usb_mutex_unlock(void)
{
	usb_mutex_unlock_implementation();
}
