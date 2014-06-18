/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include <stdio.h>
#include <time.h>

extern "C" void rplShowRuntimeState(void);


/*
BYTEPTR testprogram=(BYTEPTR)"<< 1 'A' LAMSTO A 'A' LAMSTO >> " // UNIT TEST: ALREADY DEFINED LAMS COMPILED AS GETLAM/PUTLAM
                             "<< 1 'A' LAMSTO << A 'A' LAMSTO >> A 'A' LAMSTO >> " // UNIT TEST: LAMS ACROSS SECONDARY BOUNDARIES ARE SEARCHED BY NAME, NOT GETLAM/PUTLAM
                             "<< 1 'A' LAMSTO 1 A FOR i A i + 'A' LAMSTO NEXT A 'A' LAMSTO 'i' LAMSTO i >> " // UNIT TEST: LAMS ACROSS FOR LOOPS ARE OK AS GETLAM/PUTLAM, LOOP VARS DISSAPPEAR AFTER NEXT/STEP
                             "<< 1 'A' LAMSTO << 1 'B' LAMSTO A B C >> A B C >> " // UNIT TEST: LAMS AFTER SECONDARIES ARE CLEANED UP PROPERLY
                             "<< 1 'A' LAMSTO :: 1 'B' LAMSTO A B C ; A B C >> " // UNIT TEST: LAMS ACROSS DOCOL ARE COMPILED AS GETLAM/PUTLAM, BUT NEW ENVIRONMENTS NEED TO BE CREATED/CLEANED UP AS NEEDED
                            ;
*/
/*
BYTEPTR testprogram=(BYTEPTR)"<< 1 'A' LAMSTO DO A A 1 + 'A' LAMSTO UNTIL A 10 == END \"END!\" >> EVAL " // UNIT TEST: DO/UNTIL TEST WITH LAMS
                            ;
*/
/*
BYTEPTR testprogram=(BYTEPTR)//"<< 1 'A' LAMSTO WHILE A 10 <= REPEAT A A 1 + 'A' LAMSTO END \"END!\" >> " // UNIT TEST: WHILE/REPEAT TEST WITH LAMS
                             "<< 1 'A' LAMSTO WHILE A 'B' LAMSTO A 10 <= REPEAT B A A 1 + 'A' LAMSTO END B \"END!\" >> EVAL " // UNIT TEST: WHILE/REPEAT TEST WITH LAMS
                            ;
*/
/*
BYTEPTR testprogram=(BYTEPTR) "<< 1 'A' STO 'DIR1' CRDIR DIR1 10 'B' STO 'DIR1' RCL 'DIR2' STO DIR2 1000 'C' STO HOME 'DIR1' RCL 'NEWDIR' STO >> EVAL";
*/
/*
BYTEPTR testprogram=(BYTEPTR) "DISPDEBUG 10 2 3 + SWAP DUP + SWAP DROP DISPDEBUG GARBAGE DISPDEBUG 1 SWAP FOR i i i * NEXT DISPDEBUG GARBAGE DISPDEBUG 1 10 FOR i DROP NEXT GARBAGE DISPDEBUG";
*/
/*
BYTEPTR testprogram=(BYTEPTR) "1 DISPDEBUG 1 100000 FOR i i 1 - DUP * + GARBAGE NEXT DISPDEBUG GARBAGE DISPDEBUG";
*/
/*
 BYTEPTR testprogram=(BYTEPTR) "{ 1 2 3 4 5 6 7 8 9 } 'A' LAMSTO 'A' 3 16 PUT 'A' 3 GET A";
*/

/*
// N-QUEENS WITH ALL CONSTANT NUMBERS AS REALS
BYTEPTR testprogram=(BYTEPTR) "<< 8. 0. 0. 0. { } -> R S X Y A "
                "  << "
               " 1. R START 0. NEXT R ->LIST 'A' STO "
               " DO "
               "  'A' 'X' INCR R PUT "
               "  DO "
               "    'S' INCR DROP "
               "   X 'Y' STO "
               "   WHILE Y 1 > REPEAT "
               "     A X GET A 'Y' DECR  GET - "
               "     IF DUP 0. == SWAP ABS X Y - == OR THEN "
               "       0. 'Y' STO "
               "      'A' X A X GET 1. - PUT "
               "       WHILE A X GET 0. == REPEAT "
               "         'A' 'X' DECR A X GET 1. - PUT "
               "       END "
               "     END "
               "   END "
               "  UNTIL Y 1. == END "
               " UNTIL X R == END "
               " "
               " S A "
            " >> "
          " >> "
          " 'PRO' STO "
          "     1 10 START PRO DROP DROP NEXT"
          ;
*/
/*
// N-QUEENS WITH ALL CONSTANTS AS INTEGERS (SINT)
BYTEPTR testprogram=(BYTEPTR) "<< 8 0 0 0 { } -> R S X Y A "
                "  << "
               " 1 R START 0 NEXT R ->LIST 'A' STO "
               " DO "
               "  'A' 'X' INCR R PUT "
               "  DO "
               "    'S' INCR DROP "
               "   X 'Y' STO "
               "   WHILE Y 1 > REPEAT "
               "     A X GET A 'Y' DECR  GET - "
               "     IF DUP 0 == SWAP ABS X Y - == OR THEN "
               "       0 'Y' STO "
               "      'A' X A X GET 1 - PUT "
               "       WHILE A X GET 0 == REPEAT "
               "         'A' 'X' DECR A X GET 1 - PUT "
               "       END "
               "     END "
               "   END "
               "  UNTIL Y 1 == END "
               " UNTIL X R == END "
               " "
               " S A "
            " >> "
          " >> "
          " 'PRO' STO "
          "     1 1000 START PRO DROP DROP NEXT "
           ;
*/

/*
BYTEPTR testprogram=(BYTEPTR) "1.0 'val' LAMSTO 1 1000000 FOR J 150. 1. DUP ROT FOR I I * NEXT 'val' LAMSTO NEXT val";
*/





// GENERATE THE TRANSCENDENTALS TABLE ATAN(X) FOR X=1*10^-N
// USES 2016 DIGITS PRECISION (2025 TEMPORARY TO GUARANTEE ROUNDING)
/*
BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "
                                " << 0.0 'RESULT' LAMSTO 1 'SIGN' LAMSTO "
                                "DUP UNROT NEG 10 SWAP ^ * 3000 ROT / IP 2 * 0  FOR k DUP 2 k * 1 + DUP UNROT ^ SWAP / SIGN * "
                                "RESULT + 'RESULT' LAMSTO SIGN NEG 'SIGN' LAMSTO -1 STEP DROP RESULT >>"
                                " 'ATAN' STO "
                                " << 1 'SIGN' LAMSTO 0.0 1200 0 FOR k "
                                " -32  4 k * 1 + / "
                                "      4 k * 3 + INV - "
                                " 256 10 k * 1 + / + "
                                " 64  10 k * 3 + / - "
                                " 4   10 k * 5 + / - "
                                " 4   10 k * 7 + / - "
                                 "    10 k * 9 + INV + "
                                " SIGN 2 10 k * ^ * INV * + "
                                " SIGN NEG 'SIGN' LAMSTO -1 STEP 256 / >> "
                                " 'QUARTERPI' STO "
                                " QUARTERPI "
                              " TRANSCENTABLE WRITETABLE 1 1008 FOR I 1 I ATAN TRANSCENTABLE WRITETABLE NEXT "
                                ;
*/

// GENERATE THE TRANSCENDENTALS TABLE ATAN(X) FOR X=2*10^-N
// USES 2016 DIGITS PRECISION w/2025 INTERNAL PRECISION
/*
BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "
                                " << 0.0 'RESULT' LAMSTO 1 'SIGN' LAMSTO "
                                "DUP UNROT NEG 10 SWAP ^ * 3000 ROT / IP 2 * 0  FOR k DUP 2 k * 1 + DUP UNROT ^ SWAP / SIGN * "
                                "RESULT + 'RESULT' LAMSTO SIGN NEG 'SIGN' LAMSTO -1 STEP DROP RESULT >>"
                                " 'ATAN' STO "
                                "1 1008 FOR I 2 I ATAN TRANSCENTABLE WRITETABLE NEXT "


                                ;
*/

// GENERATE THE TRANSCENDENTALS TABLE ATAN(X) FOR X=5*10^-N
// USES 2016 DIGITS PRECISION w/2025 INTERNAL
/*
BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "
                                " << 0.0 'RESULT' LAMSTO 1 'SIGN' LAMSTO "
                                "DUP UNROT NEG 10 SWAP ^ * 3000 ROT / IP 2 * 0  FOR k DUP 2 k * 1 + DUP UNROT ^ SWAP / SIGN * "
                                "RESULT + 'RESULT' LAMSTO SIGN NEG 'SIGN' LAMSTO -1 STEP DROP RESULT >>"
                                " 'ATAN' STO "
                                "1 1008 FOR I 5 I ATAN TRANSCENTABLE WRITETABLE NEXT "


                                ;
*/
// GENERATE THE CONSTANT K = PRODUCT(COS(ALPHAi))=1/SQRT (PRODUCT( 1+k^2*10^-2n)) with k=5,2,2,1... AND n=0,... n DIGITS
// USES 2016 DIGITS PRECISION
/*
BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "
                                "2.0 "
                                "1 1008 FOR I 10 2 I * NEG ^ 1 * 1 + * NEXT "
                                "1 1008 FOR I 10 2 I * NEG ^ 4 * 1 + DUP * * NEXT "
                                "1 1008 FOR I 10 2 I * NEG ^ 25 * 1 + * NEXT "
                                " 0.5 ^ INV DUP TRANSCENTABLE DUP WRITETABLE "
                                ;

*/

// GENERATE THE TRANSCENDENTALS TABLE WITH CONSTANTS 2*PI, PI, PI/2, PI/4 AT MAX. SYSTEM PRECISION
// USES 2016 DIGITS PRECISION (2025 TEMPORARY TO GUARANTEE ROUNDING)
/*
BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "
                                " << 1 'SIGN' LAMSTO 0.0 1200 0 FOR k "
                                " -32  4 k * 1 + / "
                                "      4 k * 3 + INV - "
                                " 256 10 k * 1 + / + "
                                " 64  10 k * 3 + / - "
                                " 4   10 k * 5 + / - "
                                " 4   10 k * 7 + / - "
                                 "    10 k * 9 + INV + "
                                " SIGN 2 10 k * ^ * INV * + "
                                " SIGN NEG 'SIGN' LAMSTO -1 STEP 64 / >> "
                                " 'PI' STO "
                                " PI DUP DUP DUP 2 * TRANSCENTABLE WRITETABLE "
                                " TRANSCENTABLE WRITETABLE "
                                " 2 / TRANSCENTABLE WRITETABLE "
                                " 4 / TRANSCENTABLE WRITETABLE "
                                ;

*/


// HYPERBOLIC TRANSCENDENTAL TABLES FOR ATANH(1*10^-x)
/*
BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "
                                " << 'K' LAMSTO 10 K NEG ^ 'X' LAMSTO 0.0 1 2025 K / IP 2 + FOR I X I ^ I / + 2 STEP >> 'MYATANH' STO "
                                "1 1008 FOR I I MYATANH TRANSCENTABLE WRITETABLE NEXT "
                                ;
*/
// HYPERBOLIC TRANSCENDENTAL TABLES FOR ATANH(2*10^-x)
/*
 BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "
                                " << 'K' LAMSTO 10 K NEG ^ 2 * 'X' LAMSTO 0.0 4000 K 3 / - K / 2 / IP 2 * 1 + 1 FOR I X I ^ I / + -2 STEP >> 'MYATANH' STO "
                                "1 1008 FOR I I MYATANH TRANSCENTABLE WRITETABLE NEXT "
                                ;                              
*/
// HYPERBOLIC TRANSCENDENTAL TABLES FOR ATANH(5*10^-x)
/*
BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "
                                " << 'K' LAMSTO 10 K NEG ^ 5 * 'X' LAMSTO 0.0 8000 K - K / 2 / IP 2 * 1 + 1 FOR I X I ^ I / + -2 STEP >> 'MYATANH' STO "
                                "1 1008 FOR I I MYATANH TRANSCENTABLE WRITETABLE NEXT "
                                ;
*/

 // GENERATE THE CONSTANT K = PRODUCT(1/sqrt(1-alphai^2))=1/SQRT (PRODUCT( 1-k^2*10^-2n)) with k=5,2,2,1... AND n=1,... n DIGITS
 // USES 2016 DIGITS PRECISION
/*
 BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "
                                 "1.0 "
                                 "1 1008 FOR I 10 2 I * NEG ^ 1 * 1 - * NEXT "
                                 "1 1008 FOR I 10 2 I * NEG ^ 4 * 1 - DUP * * NEXT "
                                 "1 1008 FOR I 10 2 I * NEG ^ 25 * 1 - * NEXT "
                                 " 0.5 ^ INV DUP TRANSCENTABLE DUP WRITETABLE "
                                 ;

*/
/*
BYTEPTR testprogram=(BYTEPTR) "2016 SETPREC "
                              " 1.0 CEXP "
                            " 0.5 CEXP "
                            " 0.2 CEXP "
                            " 0.1 CEXP "
                            " 0.0 CEXP "
                            " -0.3 CEXP "
                            " -0.6 CEXP "
                            " -0.9 CEXP "
                               ;
*/

// GENERATE THE CONSTANT LN(10)
/*
BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "
                                " << DUP 1 - SWAP / 'X' LAMSTO -1 'SIGN' LAMSTO 0.0 50000 1 FOR I X I ^ I / + -1 STEP >> 'MYLN' STO "
                                " 10 MYLN DUP TRANSCENTABLE WRITETABLE "
                                ;
*/
/*
BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "
                                " << 1 CEXP DUP DUP * * / 'Z' LAMSTO -0.7 'X' LAMSTO DO X CEXP DUP Z - SWAP / X SWAP - X SWAP DUP 'X' STO UNTIL - ABS 1E-2016 < END X >> 'MYLN' STO "
                                " 10 MYLN 3 + 2 / DUP TRANSCENTABLE WRITETABLE "
                                ;
*/


// GENERATE THE CONSTANT Khyp = PRODUCT(1/sqrt(1-alphai^2))=1/SQRT (PRODUCT( 1-k^2*10^-2n)) with k=5,2,2,1... AND n=1,... n DIGITS
// USES 2016 DIGITS PRECISION
/*
BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "

        " 1.0 "
        " 1008 1 FOR I 10 2 I * ^ 1 SWAP / 1 - * "
                     " 10 2 I * ^ 4 SWAP / 1 - DUP * * "
                     " 10 2 I * ^ 25 SWAP / 1 - * "
        " DUP 0.5 ^ INV TRANSCENTABLE WRITETABLE -1 STEP "
                               ;
*/

// GENERATE THE CONSTANT K = PRODUCT(1/sqrt(1+alphai^2))=1/SQRT (PRODUCT( 1+k^2*10^-2n)) with k=5,2,2,1... AND n=1,... n DIGITS
// USES 2016 DIGITS PRECISION
/*
BYTEPTR testprogram=(BYTEPTR) "2025 SETPREC "

        " 1.0 "
        " 1008 1 FOR I 10 2 I * ^ 1 SWAP / 1 + * "
                     " 10 2 I * ^ 4 SWAP / 1 + DUP * * "
                     " 10 2 I * ^ 25 SWAP / 1 + * "
        " DUP 0.5 ^ INV TRANSCENTABLE WRITETABLE -1 STEP "
        " 2 * 0.5 ^ INV TRANSCENTABLE WRITETABLE "
;
*/


BYTEPTR testprogram=(BYTEPTR) "<< \"\" SWAP "
        "WHILE DUP 0 > REPEAT "
           "CASE "
              "DUP 1000 >= THEN 1000 \"M\"  END "
              "DUP  900 >= THEN  900 \"CM\" END "
              "DUP  500 >= THEN  500 \"D\"  END "
              "DUP  400 >= THEN  400 \"CD\" END "
              "DUP  100 >= THEN  100 \"C\"  END "
              "DUP   90 >= THEN   90 \"XC\" END "
              "DUP   50 >= THEN   50 \"L\"  END "
              "DUP   40 >= THEN   40 \"XL\" END "
              "DUP   10 >= THEN   10 \"X\"  END "
              "DUP    9 >= THEN    9 \"IX\" END "
              "DUP    5 >= THEN    5 \"V\"  END "
              "DUP    4 >= THEN    4 \"IV\" END "
              "DUP    1 >= THEN    1 \"I\"  END "
          "END "
          "ROT ROT - "
          "ROT ROT + "
          "SWAP "
        "END DROP "
        ">> 'ROMAN' STO "
        "<< "
          "<< ROT ROT - ROT ROT + SWAP >> -> A "
          "<< \"\" SWAP "
            "WHILE DUP 1000 >= REPEAT 1000 \"M\" A EVAL END "
            "WHILE DUP 900 >= REPEAT 900 \"CM\" A EVAL END "
            "WHILE DUP 500 >= REPEAT 500 \"D\" A EVAL END "
            "WHILE DUP 400 >= REPEAT 400 \"CD\" A EVAL END "
            "WHILE DUP 100 >= REPEAT 100 \"C\" A EVAL END "
            "WHILE DUP 90 >= REPEAT 90 \"XC\" A EVAL END "
            "WHILE DUP 50 >= REPEAT 50 \"L\" A EVAL END "
            "WHILE DUP 40 >= REPEAT 40 \"XL\" A EVAL END "
            "WHILE DUP 10 >= REPEAT 10 \"X\" A EVAL END "
            "WHILE DUP 9 >= REPEAT 9 \"IX\" A EVAL END "
            "WHILE DUP 5 >= REPEAT 5 \"V\" A EVAL END "
            "WHILE DUP 4 >= REPEAT 4 \"IV\" A EVAL END "
            "WHILE DUP 1 >= REPEAT 1 \"I\" A EVAL END "
            "DROP "
          ">> "
        ">> 'DBROMAN' STO "

        "<< "
          "\"\" SWAP "
          "<< -> k r "
            "<< "
              "WHILE k 2 PICK 2 PICK >= "
              "REPEAT - SWAP r + SWAP "
              "END DROP "
            ">> "
          ">> -> A "
          "<< "
            "1000 \"M\" A EVAL "
            "900 \"CM\" A EVAL "
            "500 \"D\" A EVAL "
            "400 \"CD\" A EVAL "
            "100 \"C\" A EVAL "
            "90 \"XC\" A EVAL "
            "50 \"L\" A EVAL "
            "40 \"XL\" A EVAL "
            "10 \"X\" A EVAL "
            "9 \"IX\" A EVAL "
            "5 \"V\" A EVAL "
            "4 \"IV\" A EVAL "
            "1 \"I\" A EVAL "
          ">> "
          "DROP "
        ">> 'TK1ROMAN' STO "

        "<< -> n "
          "<< "
            "{ "
              "{ \"\"      n } "
              "{ \"M\"  1000 } "
              "{ \"CM\"  900 } "
              "{ \"D\"   500 } "
              "{ \"CD\"  400 } "
              "{ \"C\"   100 } "
              "{ \"XC\"   90 } "
              "{ \"L\"    50 } "
              "{ \"XL\"   40 } "
              "{ \"X\"    10 } "
              "{ \"IX\"    9 } "
              "{ \"V\"     5 } "
              "{ \"IV\"    4 } "
              "{ \"I\"     1 } "
            "} "
            "<< "
              "ADD LIST-> DROP "
              "-> r k "
              "<< "
                "WHILE DUP k >= "
                "REPEAT "
                  "k - SWAP "
                  "r + SWAP "
                "END "
                "2 ->LIST "
              ">> "
            ">> "
            "STREAM HEAD "
          ">> "
        ">> 'TK2ROMAN' STO "




        ;


void PrintObj(WORDPTR obj)
{
    WORDPTR string;
    BINT nwords;
    BYTEPTR charptr;
    string=rplDecompile(obj);

    if(string) {
    // NOW PRINT THE STRING OBJECT
    nwords=OBJSIZE(*string);
    charptr=(BYTEPTR) (string+1);
    for(;nwords>1;--nwords,charptr+=4)
    {
        printf("%c%c%c%c",charptr[0],charptr[1],charptr[2],charptr[3]);
    }
    // LAST WORD MAY CONTAIN LESS THAN 4 CHARACTERS
    nwords=4-(LIBNUM(*string)&3);
    for(;nwords>0;--nwords,charptr++)
    {
        printf("%c",*charptr);
    }
    }

}


void PrintSeco(WORDPTR obj)
{
    WORDPTR string;
    BINT nwords;
    BYTEPTR charptr;

    WORDPTR endobj=rplSkipOb(obj);

    if(ISPROLOG(*obj) && ((LIBNUM(*obj)==DOCOL) || (LIBNUM(*obj)==SECO))) {

    printf("%08X: ",obj-TempOb);
    printf(" %s\n",(LIBNUM(*obj)==DOCOL)? "::":"<<");

    ++obj;

    while(obj<endobj) {

        if(ISPROLOG(*obj) && ((LIBNUM(*obj)==DOCOL) || (LIBNUM(*obj)==SECO))) {
            PrintSeco(obj);
            obj=rplSkipOb(obj);
            continue;
        }


        printf("%08X: ",obj-TempOb);

        string=rplDecompile(obj);

    if(string) {
    // NOW PRINT THE STRING OBJECT
    nwords=OBJSIZE(*string);
    charptr=(BYTEPTR) (string+1);
    for(;nwords>1;--nwords,charptr+=4)
    {
        printf("%c%c%c%c",charptr[0],charptr[1],charptr[2],charptr[3]);
    }
    // LAST WORD MAY CONTAIN LESS THAN 4 CHARACTERS
    nwords=4-(LIBNUM(*string)&3);
    for(;nwords>0;--nwords,charptr++)
    {
        printf("%c",*charptr);
    }
    }

    printf("\n");
    obj=rplSkipOb(obj);
    }

    }
    else {

        printf("%08X: ",obj-TempOb);
        PrintObj(obj);

    }

}



void DumpDirs()
{
    WORDPTR *scan=Directories;

    while(scan<DirsTop) {

        if(**scan==DIR_START_MARKER) {
            printf("*** START ");
            WORDPTR *parent=scan;
            WORDPTR name=rplGetDirName(scan);
            while(name) {
            PrintObj(name);
            printf(" ");
            parent=rplGetParentDir(parent);
            name=rplGetDirName(parent);
            }

            printf(" *** (%d ITEMS)\n",*(*(scan+1)+1));
        }
        else {
        if(**scan==DIR_PARENT_MARKER) {

        }
        else {
        if(**scan==DIR_END_MARKER) {
            printf("*** END!\n");
        }
        else {
            PrintObj(*scan);
            printf(" = ");
            PrintObj(*(scan+1));
            printf("\n");
        }
        }
        }

        scan+=2;
    }

}


void DumpLAMs()
{
    WORDPTR *scan=LAMTop-2;

    while(scan>=LAMs) {

        if(**scan==LAM_BASESECO) {
            printf("*** Parent environment *** \n");
        }
        else {
            PrintObj(*scan);
            printf(" = ");
            PrintObj(*(scan+1));
            printf("\n");
        }

        scan-=2;
    }

}



void DumpDStack()
{
    BINT count=0;
    BINT nwords;
    WORDPTR string;
    BYTEPTR charptr;
    BINT nlevels=5;

    while(nlevels>rplDepthData() && nlevels>0 ) {
        printf("%d:\n",nlevels);
        --nlevels;
    }

    while(count<(DSTop-DStk)) {
        printf("%d:\t",DSTop-DStk-count);
        string=rplDecompile((WORDPTR)DStk[count]);

        if(string) {
        // NOW PRINT THE STRING OBJECT
        nwords=OBJSIZE(*string);
        charptr=(BYTEPTR) (string+1);
        for(;nwords>1;--nwords,charptr+=4)
        {
            printf("%c%c%c%c",charptr[0],charptr[1],charptr[2],charptr[3]);
        }
        // LAST WORD MAY CONTAIN LESS THAN 4 CHARACTERS
        nwords=4-(LIBNUM(*string)&3);
        for(;nwords>0;--nwords,charptr++)
        {
            printf("%c",*charptr);
        }




        } else { printf("***ERROR DURING DECOMPILE!!!***"); }


        printf("\n");
        ++count;
    }
}


void DumpErrors()
{
    struct error_message {
        unsigned int num;
        const char *string;
    }
    error_table[]={
    { 0x00000001,"Bad opcode"},
    { 0x00000002,"BreakPoint"},
    { 0x00000004,"Out of memory"},
    { 0x00000008,"Pointer out of range"}, // WILL CHANGE IN THE FUTURE
    { 0x00000010,"Divide by zero"}, // WILL CHANGE IN THE FUTURE
    { 0x00000020,"Overflow"}, // WILL CHANGE IN THE FUTURE
    { 0x00000040,"Empty stack"},
    { 0x00000080,"Empty return rtack"},
    { 0x00000100,"Syntax error"},
    { 0x00000200,"Undefined"},
    { 0x00000400,"Bad argument count"},
    { 0x00000800,"Bad argument type"},
    { 0x00001000,"Bad argument value"},
    { 0x00002000,"Undefined variable"},
    { 0x00004000,"Directory not empty"},
    { 0x00008000,"Invalid Dimension"},
    // THESE ARE MPDECIMAL ERRORS
    { 0x00010000,"Clamped exponent"},
    { 0x00020000,"Conversion syntax"},
    { 0x00040000,"Division by zero"},
    { 0x00080000,"Division impossible"},
    { 0x00100000,"Division undefined"},
    { 0x00200000,"FPU Error"},
    { 0x00400000,"Inexact"},
    { 0x00800000,"Invalid context"},
    { 0x01000000,"Invalid operation"},
    { 0x02000000,"Internal out of memory"},
    { 0x04000000,"Not implemented"},
    { 0x08000000,"Overflow"},
    { 0x10000000,"Rounded"},
    { 0x20000000,"Subnormal"},
    { 0x40000000,"Underflow"},
    { 0x80000000,"Undefined error??"},
    };
    int errbit;
    if(!Exceptions) return;
    printf("Error status:\n");
    for(errbit=0;errbit<32;++errbit)
    {
    if(error_table[errbit].num&Exceptions) printf("- %s\n",error_table[errbit].string);
    }

}


void Refresh()
{
    DumpDStack();
    printf("~~> ");
}


int main()
{
    char buffer[65535];

    rplInit();

    Context.prec=36;


    Refresh();
/*
    if(testprogram) {

        WORDPTR ptr=rplCompile(testprogram,strlen((char *)testprogram),1);
        if(ptr)   { rplSetEntryPoint(ptr); rplRun(); }


    }
*/

    do {

    fgets(buffer,65535,stdin);
    if(buffer[0]=='\n' && buffer[1]==0) {
        printf("Do you want to exit? Y/n: ");
        fgets(buffer,65535,stdin);
        if(buffer[0]=='y' || buffer[0]=='Y') return 0;
        Refresh();
        continue;
    }

    WORDPTR ptr=rplCompile((BYTEPTR)buffer,strlen(buffer),1);

    if(!ptr) {
        printf("COMPILE ERROR\n");
        DumpErrors();
        Exceptions=0;
        Refresh();
        continue;
    }

    clock_t start,end;

    start=clock();
    rplSetEntryPoint(ptr);
    rplRun();

    end=clock();


    if(Exceptions) {
        printf("Runtime Error: %08X at %08X\n",Exceptions,ExceptionPointer-TempOb);
        DumpErrors();
        Exceptions=0;
        DumpLAMs();
        DumpDirs();
        Refresh();

        continue;
    }

    printf("Elapsed time: %.6lf seconds\n",((double)(start-end))/(double)CLOCKS_PER_SEC);
    rplShowRuntimeState();
    Exceptions=0;
    Refresh();
    }
    while(1);


    return 0;

}

