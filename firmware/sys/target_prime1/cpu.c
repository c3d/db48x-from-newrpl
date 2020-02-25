/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

INTERRUPT_TYPE __saveint;

void cpu_intoff()
{
	if (!__saveint.mask1 && !__saveint.mask2) {
		__saveint.mask1 = *INTMSK1;
		__saveint.mask2 = *INTMSK2;
		*INTMSK1 = 0xffffffff;
		*INTMSK2 = 0xffffffff;
	}
}

INTERRUPT_TYPE __cpu_intoff()
{
	INTERRUPT_TYPE previous;
	previous.mask1 = *INTMSK1;
	previous.mask2 = *INTMSK2;

	*INTMSK1 = 0xffffffff;
	*INTMSK2 = 0xffffffff;

	return previous;	
}

void cpu_inton()
{
	if (__saveint.mask1 || __saveint.mask2) {
		*INTMSK1 = __saveint.mask1;
		*INTMSK2 = __saveint.mask2;
	}
}

void __cpu_inton(INTERRUPT_TYPE state)
{
	*INTMSK1 = state.mask1;
	*INTMSK2 = state.mask2;
}