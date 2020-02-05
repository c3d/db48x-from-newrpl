
#include <newrpl.h>

// OPTIMIZED REAL NUMBER MULTIPLICATION LOOP

// RESTRICTIONS:
// len HAS THE LOWEST 16 BITS = blen, HIGHER 16 BITS = alen
extern const unsigned char const carry_table[];

void mul_real_arm(BINT * rdata, BINT * adata, BINT * bdata, UBINT len)
        __attribute__((naked));
__ARM_MODE__ void mul_real_arm(BINT * rdata, BINT * adata, BINT * bdata,
        UBINT len)
{
    asm volatile ("push {r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");

    // INITIALIZE END POINTERS

    asm volatile ("mov r14,#0xab000000");       // r14 = 2882303762
    asm volatile ("mov r11,r3,lsr #16");        // R11=alen
    asm volatile ("mov r4,#0"); // R4 = i
    asm volatile ("orr r14,r14,#0xcc0000");
    asm volatile ("sub r10,r3,r11,asl #16");    // R10=blen
    asm volatile ("mov r12,#0xfa000000");       // R12 = 4194967296
    asm volatile ("mov r11,r11,lsl #2");        // COUNT IN OFFSET, NOT WORDS
    asm volatile ("mov r10,r10,lsl #2");
    asm volatile ("orr r12,r12,#0xa1000");
    asm volatile ("orr r14,r14,#0x7700");
    asm volatile ("orr r12,r12,#0xf00");
    asm volatile ("orr r14,r14,#0x12");

    asm volatile (".Lloop_i:");

    asm volatile ("cmp r4,r10 \n\t" "bge .Lendloop_i \n\t");    // while (i<blen) {

    asm volatile ("mov r5,#0"); // R5=j

    asm volatile ("ldr r7,[r2,r4]");    // R7= b->data[i]

    asm volatile (".Lloop_j:");
    asm volatile ("cmp r5,r11 \n\t" "bge .Lendloop_j \n\t");    // while (j<alen) {
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

    asm volatile ("sub r0,r0,r11");
    asm volatile ("add r4,r4,#4");
    asm volatile ("add r0,r0,#4");
    asm volatile ("tst r4,#0x1f");
    asm volatile ("bne .Lloop_i");

    // DO CARRY CORRECTION LOOP
    asm volatile ("push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");
    asm volatile ("sub r0,r0,#32");     // R0 = SOURCE PTR
    asm volatile ("add r1,r11,#40");    // R1 = NWORDS*4
    asm volatile ("add r2,r0,r1");      // R2 = END PTR

    // THE NEXT 2 LINES ARE VOODOO TO MAKE GCC PUT carry_table IN R3
    register char *carryptr asm("r3");
    asm volatile ("":"=r" (carryptr):"0"(carry_table));

    // R3 = POINTER TO THE CARRY TABLE

//    BINT *end=start+nwords-1;
//    BINT carry=0,word;

    asm volatile ("mov r12,#0");        // R12 = CARRY
    asm volatile ("mov r7,#0x5f00000"); // R7 = 100000000
    asm volatile ("orr r7,#0x5e000");   // R7 = 100000000
    asm volatile ("orr r7,#0x100");     // R7 = 100000000
    asm volatile ("sub r8,r12,r7");     // R8 = -1E8

    // while(start<end) { -- PROCESS TWO WORDS AT ONCE
    asm volatile ("cmp r0,r2");

    asm volatile ("bge .Lmendccloop");

    asm volatile (".Lmccloop:");

    asm volatile ("ldm r0,{r6,r10}");   // READ TWO WORDS

    asm volatile ("add r6,r6,r12");     // ADD THE CARRY, AND UPDATE STATUS BITS IF IT'S NEGATIVE
    asm volatile ("mov r12,#0");        // START WITH A NO CARRY

    // FROM HERE PROCEED AS IF THE WORD IS POSITIVE
    asm volatile ("ldrb r5,[r3,r6,lsr #26]");   //  NEW CARRY
    asm volatile ("ldrb r11,[r3,r10,lsr #26]"); //  NEW CARRY
    asm volatile ("mla r6,r8,r5,r6");   //  NUMBER-=CARRY*100000000
    asm volatile ("mla r10,r8,r11,r10");        //  NUMBER-=CARRY*100000000
    asm volatile ("add r12,r12,r11");
    asm volatile ("cmp r6,r7"); // IT COULD STILL BE >1E8
    asm volatile ("addge r10,r10,#1");  // INCREASE CARRY
    asm volatile ("subge r6,r6,r7");    // AND SUBTRACT 1E8

    asm volatile ("add r10,r10,r5");    // ADD CARRY
    asm volatile ("cmp r10,r7");        // IT COULD STILL BE >1E8
    asm volatile ("addge r12,r12,#1");  // INCREASE CARRY
    asm volatile ("subge r10,r10,r7");

    asm volatile ("stm r0!,{r6,r10}");

    asm volatile ("cmp r0,r2");
    asm volatile ("blt .Lmccloop");

    asm volatile (".Lmendccloop:");

    // LAST WORD

    asm volatile ("bgt .Lmccjustreturn");

    asm volatile ("ldr r6,[r0]");       // READ ONE WORD
    asm volatile ("add r6,r6,r12");     // ADD THE CARRY

    // FROM HERE PROCEED AS IF THE WORD IS POSITIVE
    asm volatile ("ldrb r12,[r3,r6,lsr #26]");  //  NEW CARRY
    asm volatile ("mla r6,r8,r12,r6");  //  NUMBER-=CARRY*100000000
    asm volatile ("cmp r6,r7"); // IT COULD STILL BE >1E8
    asm volatile ("addge r12,r12,#1");  // INCREASE CARRY
    asm volatile ("subge r6,r6,r7");    // AND SUBTRACT 1E8

    asm volatile ("str r6,[r0]");

    asm volatile (".Lmccjustreturn:");

    asm volatile ("cmp r12,#0");

    // STORE LEFT CARRY IF ANY
    asm volatile ("ldrne r6,[r0,#4]");
    asm volatile ("addne r6,r6,r12");
    asm volatile ("strne r6,[r0,#4]");

    asm volatile ("pop {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");

    // ********* END CARRY CORRECTION LOOP

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
    asm volatile ("bx lr");

}

BINT carry_correct_arm(BINT * start, BINT * dest, BINT * end, char *carry_table)
        __attribute__((naked));
__ARM_MODE__ BINT carry_correct_arm(BINT * start, BINT * dest, BINT * end,
        char *carry_table)
{

    asm volatile ("push {r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");

    // R0 = SOURCE PTR
    // R1 = DESTINATION POINTER
    // R2 = END PTR
    // R3 = POINTER TO THE CARRY TABLE

//    BINT *end=start+nwords-1;
//    BINT carry=0,word;

    asm volatile ("mov r12,#0");        // R12 = CARRY
    asm volatile ("mov r7,#0x5f00000"); // R7 = 100000000
    asm volatile ("orr r7,#0x5e000");   // R7 = 100000000
    asm volatile ("orr r7,#0x100");     // R7 = 100000000
    asm volatile ("sub r8,r12,r7");     // R8 = -1E8
    asm volatile ("mov r9,#0x83000000");
    asm volatile ("orr r9,r9,#0x210000");
    asm volatile ("orr r9,r9,#0x5600"); // R9=22*1E8

    // IMPROVEMENT, SKIP WORDS UNTIL WE ACTUALLY NEED CARRY CORRECTION
    asm volatile (".skipccloop:");
    asm volatile ("cmp r0,r2");
    asm volatile ("bge .Lendccloop");
    asm volatile ("ldr r6,[r0],#4");
    asm volatile ("cmp r6,#0");
    asm volatile ("sublt r0,r0,#4");
    asm volatile ("blt .Lccloop");      // STOP SKIPPING IF WORD IS NEGATIVE
    asm volatile ("cmp r6,r7");
    asm volatile ("subge r0,r0,#4");
    asm volatile ("bge .Lccloop");      // STOP SKIPPING IF WORD IS NEGATIVE
    asm volatile ("str r6,[r1],#4");
    asm volatile ("b .skipccloop");

    // while(start<end) { -- PROCESS TWO WORDS AT ONCE
    asm volatile ("cmp r0,r2");

    asm volatile ("bge .Lendccloop");

    asm volatile (".Lccloop:");

    asm volatile ("ldm r0!,{r6,r10}");  // READ TWO WORDS

    asm volatile ("add r6,r6,r12");     // ADD THE CARRY, AND UPDATE STATUS BITS IF IT'S NEGATIVE
    asm volatile ("sub r10,r10,#22");   // AND SUBTRACT 22 CARRIES FROM THE UPPER WORD
    asm volatile ("adds r6,r6,r9");     // ADD 22E8 TO MAKE SURE IT'S POSITIVE
    asm volatile ("addcs r6,r6,r8");    // IF THERE WAS CARRY, SUBTRACT 1E8
    asm volatile ("addcs r10,r10,#1");  // AND RETURN THE CARRY TO r10
    asm volatile ("mvn r12,#21");       // START WITH A -22 CARRY
    asm volatile ("adds r10,r10,r9");   // ADD 22E8 TO MAKE SURE IT'S POSITIVE
    asm volatile ("addcs r10,r10,r8");  // IF THERE WAS CARRY, SUBTRACT 1E8
    asm volatile ("addcs r12,r12,#1");  // AND RETURN THE CARRY TO r10

    // FROM HERE PROCEED AS IF THE WORD IS POSITIVE
    asm volatile ("ldrb r5,[r3,r6,lsr #26]");   //  NEW CARRY
    asm volatile ("ldrb r11,[r3,r10,lsr #26]"); //  NEW CARRY
    asm volatile ("mla r6,r8,r5,r6");   //  NUMBER-=CARRY*100000000
    asm volatile ("mla r10,r8,r11,r10");        //  NUMBER-=CARRY*100000000
    asm volatile ("add r12,r12,r11");
    asm volatile ("cmp r6,r7"); // IT COULD STILL BE >1E8
    asm volatile ("addge r10,r10,#1");  // INCREASE CARRY
    asm volatile ("subge r6,r6,r7");    // AND SUBTRACT 1E8

    asm volatile ("add r10,r10,r5");    // ADD CARRY
    asm volatile ("cmp r10,r7");        // IT COULD STILL BE >1E8
    asm volatile ("addge r12,r12,#1");  // INCREASE CARRY
    asm volatile ("subge r10,r10,r7");

    asm volatile ("stm r1!,{r6,r10}");

    asm volatile ("cmp r0,r2");
    asm volatile ("blt .Lccloop");

    asm volatile (".Lendccloop:");

    // LAST WORD

    asm volatile ("bgt .Lccjustreturn");

    asm volatile ("ldr r6,[r0]");       // READ ONE WORD
    asm volatile ("add r6,r6,r12");     // ADD THE CARRY
    asm volatile ("mvn r12,#21");       // START WITH A -22 CARRY
    asm volatile ("adds r6,r6,r9");     // ADD 22E8 TO MAKE SURE IT'S POSITIVE
    asm volatile ("addcs r6,r6,r8");    // IF THERE WAS CARRY, SUBTRACT 1E8
    asm volatile ("addcs r12,r12,#1");  // AND RETURN THE CARRY

    // FROM HERE PROCEED AS IF THE WORD IS POSITIVE
    asm volatile ("ldrb r11,[r3,r6,lsr #26]");  //  NEW CARRY
    asm volatile ("mla r6,r8,r11,r6");  //  NUMBER-=CARRY*100000000
    asm volatile ("add r12,r12,r11");
    asm volatile ("cmp r6,r7"); // IT COULD STILL BE >1E8
    asm volatile ("addge r12,r12,#1");  // INCREASE CARRY
    asm volatile ("subge r6,r6,r7");    // AND SUBTRACT 1E8

    asm volatile ("str r6,[r1]");

    asm volatile (".Lccjustreturn:");

    asm volatile ("mov r0,r12");
    asm volatile ("pop {r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");
    asm volatile ("bx lr");
}

// SHIFT AND MULTIPLY IN ONE OPERATION:
// PERFORMS result+=n1start*mul*10^shift;
// mul IS A SMALL CONSTANT BETWEEN 1-31
// shift IS IN THE RANGE 0-7
// shift_mul = 16 higher bits = shift, lower 16 bits = mul

extern const BINT const shiftmul_K1[];
extern const BINT const shiftmul_K2[];

void add_long_mul_shift_arm(BINT * result, BINT * n1start, BINT nwords,
        BINT shift_mul) __attribute__((naked));
__ARM_MODE__ void add_long_mul_shift_arm(BINT * result, BINT * n1start,
        BINT nwords, BINT shift_mul)
{

    //  r0 = result
    //  r1 = n1start
    //  r2 = nwords
    //  r3 = (shift<<16) | mul

    asm volatile ("push {r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");

    /*
       BINT K1,K2;
       BINT hi,lo,hi2,lo2,hi3,lo3;

       K1=shiftmul_K1[((mul-1)<<3)+shift];
       K2=shiftmul_K2[((mul-1)<<3)+shift];
     */
    asm volatile ("mov r4,r3,lsr #16");
    asm volatile ("sub r3,r3,r4,lsl #16");
    asm volatile ("sub r3,r3,#1");
    asm volatile ("add r6,r4,r3,lsl #3");

    // THE NEXT 2 LINES ARE VOODOO TO MAKE GCC PUT THE ADDRESS IN R3
    register char *shiftptr asm("r3");
    asm volatile ("":"=r" (shiftptr):"0"(shiftmul_K1));

    // R4 = K1
    asm volatile ("ldr r4,[r3,r6,lsl #2]");

    // R5 = K2
    asm volatile ("":"=r" (shiftptr):"0"(shiftmul_K2));
    asm volatile ("ldr r5,[r3,r6,lsl #2]");

    // while(nwords>=3) {

    asm volatile ("cmp r2,#2");
    asm volatile ("ble .singleloop");

    asm volatile (".multiloop:");

    //hi=(n1start[0] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION
    //hi2=(n1start[1] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION
    //hi3=(n1start[2] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION

    asm volatile ("ldmia r1!,{r6,r7,r8}");
    asm volatile ("umull r9,r10,r6,r4");
    asm volatile ("umull r11,r12,r7,r4");
    asm volatile ("umull r3,r14,r8,r4");
    asm volatile ("mul r6,r5,r6");
    asm volatile ("mul r7,r5,r7");
    asm volatile ("mul r8,r5,r8");
    asm volatile ("lsr r9,r9,#24");
    asm volatile ("lsr r11,r11,#24");
    asm volatile ("lsr r3,r3,#24");
    asm volatile ("orr r9,r9,r10,lsl #8");      // R9 = hi
    asm volatile ("orr r11,r11,r12,lsl #8");    // R11 = hi2
    asm volatile ("orr r3,r3,r14,lsl #8");      // R3 = hi3
    asm volatile ("mov r14,#0xfa000000");       // R14 = -100000000
    asm volatile ("orr r14,#0xa1000");  // R14 = -100000000
    asm volatile ("orr r14,#0xf00");    // R14 = -100000000
    asm volatile ("mla r6,r9,r14,r6");
    asm volatile ("mla r7,r11,r14,r7");
    asm volatile ("mla r8,r3,r14,r8");
//        lo=n1start[0]*K2-hi*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
//        lo2=n1start[1]*K2-hi2*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
//        lo3=n1start[2]*K2-hi3*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW

    // HERE WE HAVE R6,R7,R8 =lo,lo2,lo3
    // R9, R11, R3 = hi,hi2,hi3

    // R10,R12,R14 = FREE FOR USE

//        result[0]+=lo;
//        result[1]+=hi+lo2;
//        result[2]+=hi2+lo3;
//        result[3]+=hi3;

    asm volatile ("add r7,r7,r9");
    asm volatile ("add r8,r8,r11");

    // NOW R9 AND R11 ARE FREE

    asm volatile ("ldmia r0,{r9,r10,r11,r12}");
    asm volatile ("add r6,r6,r9");
    asm volatile ("add r7,r7,r10");
    asm volatile ("add r8,r8,r11");
    asm volatile ("add r9,r3,r12");
    asm volatile ("stmia r0!,{r6,r7,r8,r9}");

    asm volatile ("sub r2,r2,#3");
    asm volatile ("sub r0,r0,#4");

    asm volatile ("cmp r2,#2");
    asm volatile ("bgt .multiloop");

    asm volatile (".singleloop:");

    asm volatile ("mov r14,#0xfa000000");       // R14 = -100000000
    asm volatile ("orr r14,#0xa1000");  // R14 = -100000000
    asm volatile ("orr r14,#0xf00");    // R14 = -100000000

    asm volatile ("cmp r2,#1");
    asm volatile ("blt .nomorewords");
    asm volatile ("beq .onesingleword");

    asm volatile (".twowords:");

    /*    if(nwords==2) {

       hi=(n1start[0] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION
       hi2=(n1start[1] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION

       lo=n1start[0]*K2-hi*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
       lo2=n1start[1]*K2-hi2*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW

       result[0]+=lo;
       result[1]+=hi+lo2;
       result[2]+=hi2;
     */

    asm volatile ("ldmia r1!,{r6,r7}");
    asm volatile ("umull r9,r10,r6,r4");
    asm volatile ("umull r11,r12,r7,r4");
    asm volatile ("mul r6,r5,r6");
    asm volatile ("mul r7,r5,r7");
    asm volatile ("lsr r9,r9,#24");
    asm volatile ("lsr r11,r11,#24");
    asm volatile ("orr r9,r9,r10,lsl #8");      // R9 = hi
    asm volatile ("orr r11,r11,r12,lsl #8");    // R11 = hi2
    asm volatile ("mla r6,r9,r14,r6");
    asm volatile ("mla r7,r11,r14,r7");
//        lo=n1start[0]*K2-hi*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
//        lo2=n1start[1]*K2-hi2*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW

    // HERE WE HAVE R6,R7 =lo,lo2
    // R9, R11 = hi,hi2

    // R10,R12,R14 = FREE FOR USE

//        result[0]+=lo;
//        result[1]+=hi+lo2;
//        result[2]+=hi2;

    asm volatile ("add r7,r7,r9");

    // NOW R9 AND R11 ARE FREE

    asm volatile ("ldmia r0,{r9,r10,r12}");
    asm volatile ("add r6,r6,r9");
    asm volatile ("add r7,r7,r10");
    asm volatile ("add r11,r11,r12");
    asm volatile ("stmia r0!,{r6,r7,r11}");

    asm volatile ("pop {r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");
    asm volatile ("bx lr");

//       return;

    asm volatile (".onesingleword:");

    asm volatile ("ldr r6,[r1]");
    asm volatile ("umull r9,r10,r6,r4");
    asm volatile ("mul r6,r5,r6");
    asm volatile ("lsr r9,r9,#24");
    asm volatile ("orr r9,r9,r10,lsl #8");      // R9 = hi
    asm volatile ("mla r6,r9,r14,r6");  // R6 = lo

    //  hi=(n1start[0] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION

    //  lo=n1start[0]*K2-hi*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW

    asm volatile ("ldmia r0,{r7,r8}");
    asm volatile ("add r7,r7,r6");
    asm volatile ("add r8,r8,r9");
    asm volatile ("stmia r0!,{r7,r8}");

//        result[0]+=lo;
//        result[1]+=hi;

    asm volatile (".nomorewords:");
    asm volatile ("pop {r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");
    asm volatile ("bx lr");

}

void sub_long_mul_shift_arm(BINT * result, BINT * n1start, BINT nwords,
        BINT shift_mul) __attribute__((naked));
__ARM_MODE__ void sub_long_mul_shift_arm(BINT * result, BINT * n1start,
        BINT nwords, BINT shift_mul)
{

    //  r0 = result
    //  r1 = n1start
    //  r2 = nwords
    //  r3 = (shift<<16) | mul

    asm volatile ("push {r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");

    /*
       BINT K1,K2;
       BINT hi,lo,hi2,lo2,hi3,lo3;

       K1=shiftmul_K1[((mul-1)<<3)+shift];
       K2=shiftmul_K2[((mul-1)<<3)+shift];
     */
    asm volatile ("mov r4,r3,lsr #16");
    asm volatile ("sub r3,r3,r4,lsl #16");
    asm volatile ("sub r3,r3,#1");
    asm volatile ("add r6,r4,r3,lsl #3");

    // THE NEXT 2 LINES ARE VOODOO TO MAKE GCC PUT THE ADDRESS IN R3
    register char *shiftptr asm("r3");
    asm volatile ("":"=r" (shiftptr):"0"(shiftmul_K1));

    // R4 = K1
    asm volatile ("ldr r4,[r3,r6,lsl #2]");

    // R5 = K2
    asm volatile ("":"=r" (shiftptr):"0"(shiftmul_K2));
    asm volatile ("ldr r5,[r3,r6,lsl #2]");

    // while(nwords>=3) {

    asm volatile ("cmp r2,#2");
    asm volatile ("ble .ssingleloop");

    asm volatile (".smultiloop:");

    //hi=(n1start[0] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION
    //hi2=(n1start[1] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION
    //hi3=(n1start[2] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION

    asm volatile ("ldmia r1!,{r6,r7,r8}");
    asm volatile ("umull r9,r10,r6,r4");
    asm volatile ("umull r11,r12,r7,r4");
    asm volatile ("umull r3,r14,r8,r4");
    asm volatile ("mul r6,r5,r6");
    asm volatile ("mul r7,r5,r7");
    asm volatile ("mul r8,r5,r8");
    asm volatile ("lsr r9,r9,#24");
    asm volatile ("lsr r11,r11,#24");
    asm volatile ("lsr r3,r3,#24");
    asm volatile ("orr r9,r9,r10,lsl #8");      // R9 = hi
    asm volatile ("orr r11,r11,r12,lsl #8");    // R11 = hi2
    asm volatile ("orr r3,r3,r14,lsl #8");      // R3 = hi3
    asm volatile ("mov r14,#0xfa000000");       // R14 = -100000000
    asm volatile ("orr r14,#0xa1000");  // R14 = -100000000
    asm volatile ("orr r14,#0xf00");    // R14 = -100000000
    asm volatile ("mla r6,r9,r14,r6");
    asm volatile ("mla r7,r11,r14,r7");
    asm volatile ("mla r8,r3,r14,r8");
//        lo=n1start[0]*K2-hi*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
//        lo2=n1start[1]*K2-hi2*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
//        lo3=n1start[2]*K2-hi3*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW

    // HERE WE HAVE R6,R7,R8 =lo,lo2,lo3
    // R9, R11, R3 = hi,hi2,hi3

    // R10,R12,R14 = FREE FOR USE

//        result[0]+=lo;
//        result[1]+=hi+lo2;
//        result[2]+=hi2+lo3;
//        result[3]+=hi3;

    asm volatile ("add r7,r7,r9");
    asm volatile ("add r8,r8,r11");

    // NOW R9 AND R11 ARE FREE

    asm volatile ("ldmia r0,{r9,r10,r11,r12}");
    asm volatile ("rsb r6,r6,r9");
    asm volatile ("rsb r7,r7,r10");
    asm volatile ("rsb r8,r8,r11");
    asm volatile ("rsb r9,r3,r12");
    asm volatile ("stmia r0!,{r6,r7,r8,r9}");

    asm volatile ("sub r2,r2,#3");
    asm volatile ("sub r0,r0,#4");

    asm volatile ("cmp r2,#2");
    asm volatile ("bgt .smultiloop");

    asm volatile (".ssingleloop:");

    asm volatile ("mov r14,#0xfa000000");       // R14 = -100000000
    asm volatile ("orr r14,#0xa1000");  // R14 = -100000000
    asm volatile ("orr r14,#0xf00");    // R14 = -100000000

    asm volatile ("cmp r2,#1");
    asm volatile ("blt .snomorewords");
    asm volatile ("beq .sonesingleword");

    asm volatile (".stwowords:");

    /*    if(nwords==2) {

       hi=(n1start[0] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION
       hi2=(n1start[1] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION

       lo=n1start[0]*K2-hi*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
       lo2=n1start[1]*K2-hi2*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW

       result[0]+=lo;
       result[1]+=hi+lo2;
       result[2]+=hi2;
     */

    asm volatile ("ldmia r1!,{r6,r7}");
    asm volatile ("umull r9,r10,r6,r4");
    asm volatile ("umull r11,r12,r7,r4");
    asm volatile ("mul r6,r5,r6");
    asm volatile ("mul r7,r5,r7");
    asm volatile ("lsr r9,r9,#24");
    asm volatile ("lsr r11,r11,#24");
    asm volatile ("orr r9,r9,r10,lsl #8");      // R9 = hi
    asm volatile ("orr r11,r11,r12,lsl #8");    // R11 = hi2
    asm volatile ("mla r6,r9,r14,r6");
    asm volatile ("mla r7,r11,r14,r7");
//        lo=n1start[0]*K2-hi*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
//        lo2=n1start[1]*K2-hi2*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW

    // HERE WE HAVE R6,R7 =lo,lo2
    // R9, R11 = hi,hi2

    // R10,R12,R14 = FREE FOR USE

//        result[0]+=lo;
//        result[1]+=hi+lo2;
//        result[2]+=hi2;

    asm volatile ("add r7,r7,r9");

    // NOW R9 AND R11 ARE FREE

    asm volatile ("ldmia r0,{r9,r10,r12}");
    asm volatile ("rsb r6,r6,r9");
    asm volatile ("rsb r7,r7,r10");
    asm volatile ("rsb r11,r11,r12");
    asm volatile ("stmia r0!,{r6,r7,r11}");

    asm volatile ("pop {r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");
    asm volatile ("bx lr");

//       return;

    asm volatile (".sonesingleword:");

    asm volatile ("ldr r6,[r1]");
    asm volatile ("umull r9,r10,r6,r4");
    asm volatile ("mul r6,r5,r6");
    asm volatile ("lsr r9,r9,#24");
    asm volatile ("orr r9,r9,r10,lsl #8");      // R9 = hi
    asm volatile ("mla r6,r9,r14,r6");  // R6 = lo

    //  hi=(n1start[0] *(BINT64)K1)>>24;    // 64-bit MULTIPLICATION

    //  lo=n1start[0]*K2-hi*100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW

    asm volatile ("ldmia r0,{r7,r8}");
    asm volatile ("sub r7,r7,r6");
    asm volatile ("sub r8,r8,r9");
    asm volatile ("stmia r0!,{r7,r8}");

//        result[0]+=lo;
//        result[1]+=hi;

    asm volatile (".snomorewords:");
    asm volatile ("pop {r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}");
    asm volatile ("bx lr");

}
