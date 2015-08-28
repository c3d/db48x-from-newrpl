/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


unsigned int __battery;


void bat_read()
{
    __battery=0x330;    // DUMMY VALUE FOR WELL CHARGED BATTERIES
}


// SETUP ADC CONVERTERS TO READ BATTERY VOLTAGE
// DUMMY FUNCTION ON A PC
void bat_setup()
{
    __battery=0x330; // DUMMY VALUE FOR WELL CHARGED BATTERIES
}



