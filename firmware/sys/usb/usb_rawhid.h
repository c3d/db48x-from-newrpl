






/*
* Copyright (c) 2014-2016, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#ifndef usb_serial_h__
#define usb_serial_h__

#include <newrpl.h>

void usb_init(void);			// initialize everything
BYTE usb_configured(void);		// is the USB port configured
BINT usb_rawhid_recv(BYTEPTR buffer, BYTE timeout);  // receive a packet, with timeout
BINT usb_rawhid_send(const BYTEPTR buffer, BYTE timeout); // send a packet, with timeout

// This file does not include the HID debug functions, so these empty
// macros replace them with nothing, so users can compile code that
// has calls to these functions.
#define usb_debug_putchar(c)
#define usb_debug_flush_output()


// standard control endpoint request types
#define GET_STATUS			0
#define CLEAR_FEATURE			1
#define SET_FEATURE			3
#define SET_ADDRESS			5
#define GET_DESCRIPTOR			6
#define GET_CONFIGURATION		8
#define SET_CONFIGURATION		9
#define GET_INTERFACE			10
#define SET_INTERFACE			11
// HID (human interface device)
#define HID_GET_REPORT			1
#define HID_GET_IDLE			2
#define HID_GET_PROTOCOL		3
#define HID_SET_REPORT			9
#define HID_SET_IDLE			10
#define HID_SET_PROTOCOL		11
#endif
