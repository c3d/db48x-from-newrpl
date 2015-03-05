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



