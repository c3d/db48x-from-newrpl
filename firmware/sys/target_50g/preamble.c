char kinpo_preamble[16] __attribute__( (section (".text")))
={'K','I','N','P','O','U','P','D','A','T','E','I','M','A','G','E'} ;
unsigned int filler[2] __attribute__( (section (".text"))) = { 0,0 };

extern void startup(int);

void _boot(int prevstate) __attribute__ ((naked));
void _boot(int prevstate)
{
    asm volatile ("b startup");
}

