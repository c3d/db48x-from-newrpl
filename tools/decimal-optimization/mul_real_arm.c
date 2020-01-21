
#include <newrpl.h>

// OPTIMIZED REAL NUMBER MULTIPLICATION LOOP

// RESTRICTIONS:
// len HAS THE LOWEST 16 BITS = blen, HIGHER 16 BITS = alen

void mul_real_opt(BINT * rdata, BINT * adata, BINT * bdata, UBINT len)
        __attribute__((naked));
void mul_real_opt(BINT * rdata, BINT * adata, BINT * bdata, UBINT len)
{
    asm volatile ("push {r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");

    // INITIALIZE END POINTERS

    asm volatile ("mov r14,#0xab00000000");     // r14 = 2882303762
    asm volatile ("mov r11,r3,lsr #16");        // R11=blen
    asm volatile ("mov r4,#0"); // R4 = i
    asm volatile ("orr r14,r14,#0xcc0000");
    asm volatile ("sub r10,r3,r11,asr #16");    // R10=alen
    asm volatile ("mov r12,#0xfa000000");       // R12 = 4194967296
    asm volatile ("mov r11,r11,lsl #2");        // COUNT IN OFFSET, NOT WORDS
    asm volatile ("mov r10,r10,lsl #2");
    asm volatile ("orr r12,r12,#0xa1000");
    asm volatile ("orr r14,r14,#0x7700");
    asm volatile ("orr r12,r12,#0xf00");
    asm volatile ("orr r14,r14,#0x12");

    asm volatile (".Lloop_i:");

    asm volatile ("cmp r4,r11 \n\t" "bge .Lendloop_i \n\t");    // while (i<blen) {

    asm volatile ("mov r5,#0"); // R5=j

    asm volatile ("ldr r7,[r2,r4]");    // R7= b->data[i]

    asm volatile (".Lloop_j:");
    asm volatile ("cmp r5,r10 \n\t" "bge .Lendloop_j \n\t");    // while (j<alen) {
    asm volatile ("ldr r6,[r1,r5]");    // R6= a->data[j]
    asm volatile ("umull r8,r9,r6,r7"); // a->data[j]*b->data[i]
    asm volatile ("mov r9,r9,lsl #6");
    asm volatile ("orr r9,r8,lsr #26");
    asm volatile ("umull r6,r9,r14,r9");        //  R9=hi32_1=(((tmp1.w32[1]<<6)|(tmp1.w32[0]>>26))*2882303762ULL)>>32;
    asm volatile ("mla r8,r9,r12,r8");  //  R8=lo32_1=tmp1.w+hi32_1*4194967296U;
    asm volatile ("ldmia r0,{r3,r6}");
    asm volatile ("add r8,r8,r3");
    asm volatile ("add r9,r9,r6");
    asm volatile ("stmia r0,{r8,r9}");
    asm volatile ("add r5,r5,#4");
    asm volatile ("add r0,r0,#4");
    asm volatile ("b .Lloop_j");
    asm volatile (".Lendloop_j:");

    asm volatile ("sub r0,r0,r10");
    asm volatile ("add r4,r4,#4");
    asm volatile ("add r0,r0,#4");
    asm volatile ("tst r4,#0x1f");
    asm volatile ("bne .Lloop_i");

    // DO CARRY CORRECTION LOOP
    asm volatile ("push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");
    asm volatile ("mov r1,r10,lsr #2");
    asm volatile ("sub r0,r0,#32");
    asm volatile ("add r1,r10,#9");
    asm volatile ("bl carry_correct_pos");
    asm volatile ("pop {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");

    asm volatile ("b .Lloop_i");

    asm volatile (".Lendloop_i:");

    /*
       // SINGLE WORD LOOP
       while(i<b->len) {
       j=0;
       while(j<a->len) {
       //add_single64(result->data+i+j,a->data[j]*(UBINT64)b->data[i]);

       // UNROLLED add_single64()
       // NUMBER IS GUARANTEED TO BE POSITIVE
       tmp1.w=a->data[j]*(UBINT64)b->data[i];
       hi32_1=(((tmp1.w32[1]<<6)|(tmp1.w32[0]>>26))*2882303762ULL)>>32;
       lo32_1=tmp1.w+hi32_1*4194967296U;
       result->data[i+j]+=lo32_1;
       result->data[i+j+1]+=hi32_1;

       ++j;
       }
       if((i!=0)&&!(i&7)) carry_correct_pos(result->data+i-8,a->len+9);
       ++i;
       }

     */

    asm volatile ("pop {r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");
    return;

}

void _boot()
{
    BINT result[10];
    BINT a[4] = { 12345678, 0, 0, 0 };
    BINT b[4] = { 1, 1, 0, 0 };
    UBINT len = 0x10002;

    mul_real_opt(result, a, b, len);
}
