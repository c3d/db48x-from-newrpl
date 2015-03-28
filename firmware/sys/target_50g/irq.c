#include <ui.h>

#define HWREG(base,off) ( (volatile unsigned int *) (((int)base+(int)off)))

// IRQ AND CPU LOW LEVEL
unsigned int irq_table[32] __attribute__ ((section (".system_globals")));
unsigned int __saveint __attribute__ ((section (".system_globals")));





void __irq_dummy(void)
{
	return;
}

void __irq_service() __attribute__ ((naked));
void __irq_service()
{
	asm volatile ("stmfd sp!, {r0-r12,lr}");
    asm volatile ("mov r0,sp");
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0x1f");
    asm volatile ("msr cpsr_all,r1");   // SWITCH TO SYSTEM MODE
    asm volatile ("stmfd r0!,{sp,lr}"); // SAVE REGISTERS THAT WERE BANKED
    asm volatile ("stmfd sp!,{ r0 }");  // SAVE IRQ STACK PTR
    (*( (__interrupt__) (irq_table[*HWREG(INT_REGS,0x14)]))) ();
	// CLEAR INTERRUPT PENDING FLAG
	register unsigned int a=1<<(*HWREG(INT_REGS,0x14));
	*HWREG(INT_REGS,0x0)=a;
	*HWREG(INT_REGS,0x10)=a;
	
    asm volatile ("ldmia sp!, { r0 }"); // GET IRQ STACK PTR
    asm volatile ("ldmia r0!, { sp, lr }"); // RESTORE USER STACK AND LR
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xd");
    asm volatile ("msr cpsr_all,r1");   // SWITCH BACK TO IRQ MODE
    asm volatile ("ldmia sp!, {r0-r12,lr}");    // RESTORE ALL OTHER REGISTERS BACK
	asm volatile ("subs pc,lr,#4");
}


void __irq_install()
{
int f;

// MASK ALL INTERRUPTS, CLEAR ALL PENDING REQUESTS
*HWREG(INT_REGS,0x8)=0xffffffff;
*HWREG(INT_REGS,0)=0xffffffff;
*HWREG(INT_REGS,0x10)=0xffffffff;


// SET ALL IRQ SERVICES TO DUMMY
for(f=0;f<32;++f)
{
    irq_table[f]=(unsigned int)&__irq_dummy;
}
	
// HOOK INTERRUPT SERVICE ROUTINE
// ORIGINAL SAVED W/EXCEPTION HANDLERS
*( (unsigned int *) 0x08000018)=(unsigned int)&__irq_service;
	
}

void __irq_addhook(int service_number,__interrupt__ serv_routine)
{
	if(service_number<0 || service_number>31) return;
    irq_table[service_number]=(unsigned int)serv_routine;

}

void __irq_releasehook(int service_number)
{
	if(service_number<0 || service_number>31) return;
    irq_table[service_number]=(unsigned int)&__irq_dummy;
}

void __irq_mask(int service_number)
{
    *HWREG(INT_REGS,0x8)|=1<<service_number;
}

void __irq_unmask(int service_number)
{
    *HWREG(INT_REGS,0x8)&=~(1<<service_number);
}
