
#define LCD_TARGET_FREQ 500000
#define LCD_W 160
#define HOZVAL ((LCD_W>>2)-1)

// SIMULATED SYSTEM REGISTERS
int __lcd_mode=-1;
unsigned int *__lcd_buffer;
// SIMULATED SCREEN MEMORY
char PhysicalScreen[8192];
char ExceptionScreen[8192];



int __lcd_contrast __attribute__ ((section (".system_globals")));

void __lcd_fix()
{
}

void lcd_sync()
{
}

void lcd_off()
{
// TODO: TURN LCD OFF IN QT EMULATED SCREEN
}

void lcd_on()
{
    //TODO: TURN LCD ON IN QT EMULATED SCREEN

}

void lcd_save(unsigned int *buf)
{
    *buf=__lcd_mode;
    buf[1]=(unsigned int)__lcd_buffer;
}

void lcd_restore(unsigned int *buf)
{
    __lcd_buffer=(unsigned int *)buf[1];
    __lcd_mode=*buf;
}






void lcd_setcontrast(int level)
{
    int value;
if(level>15 || level<0) level=7;

// TODO: ADJUST CONTRAST IN EMULATED SCREEN
}

// SETS VIDEO MODE AND RETURNS WIDTH OF THE SCREEN IN BYTES

int lcd_setmode(int mode, int *physbuf)
{

// mode=0 -> Mono
//     =1 -> 4-gray
//     =2 -> 16-gray
// physbuf MUST be the physical address

int height=80/*(lcdreg[3])>>8*/, pagewidth=LCD_W>>(4-mode);

__lcd_mode=mode;
__lcd_buffer=physbuf;
// TODO: POINT THE QT SCREEN INTO THE GIVEN BUFFER
lcd_setcontrast(__lcd_contrast);

return pagewidth<<1;
}



