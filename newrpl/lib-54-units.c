/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************



// REPLACE THE NUMBER
#define LIBRARY_NUMBER  54


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(UDEFINE,MKTOKENINFO(6,TITYPE_NOTALLOWED,2,2)), \
    CMD(UPURGE,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(UVAL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(UBASE,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(CONVERT,MKTOKENINFO(7,TITYPE_NOTALLOWED,2,2)), \
    CMD(UFACT,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    ECMD(TOUNIT,"→UNIT",MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    ECMD(SYMBTOUNIT,"_[",MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,2))

// ADD MORE OPCODES HERE


// LIST OF ERROR CODES DEFINED BY THIS LIBRARY

#define ERROR_LIST \
        ERR(UNITEXPECTED,0), \
        ERR(INCONSISTENTUNITS,1), \
        ERR(INVALIDUNITDEFINITION,2), \
        ERR(EXPECTEDREALEXPONENT,3), \
        ERR(INVALIDUNITNAME,4), \
        ERR(UNDEFINEDUNIT,5)


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS


// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

ROMOBJECT unitdir_ident[]={
    MKPROLOG(DOIDENT,2),
    TEXT2WORD('U','N','I','T'),
    TEXT2WORD('S',0,0,0)
};

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(unitmenu_0_main);
INCLUDE_ROMOBJECT(unitmenu_1_tools);
INCLUDE_ROMOBJECT(unitmenu_2_length);
INCLUDE_ROMOBJECT(unitmenu_3_area);
INCLUDE_ROMOBJECT(unitmenu_4_volume);
INCLUDE_ROMOBJECT(unitmenu_5_time);
INCLUDE_ROMOBJECT(unitmenu_6_speed);
INCLUDE_ROMOBJECT(unitmenu_7_mass);
INCLUDE_ROMOBJECT(unitmenu_8_force);
INCLUDE_ROMOBJECT(unitmenu_9_energy);
INCLUDE_ROMOBJECT(unitmenu_10_power);
INCLUDE_ROMOBJECT(unitmenu_11_pressure);
INCLUDE_ROMOBJECT(unitmenu_12_temperature);
INCLUDE_ROMOBJECT(unitmenu_13_electrical);
INCLUDE_ROMOBJECT(unitmenu_14_angles);
INCLUDE_ROMOBJECT(unitmenu_15_light);
INCLUDE_ROMOBJECT(unitmenu_16_radiation);
INCLUDE_ROMOBJECT(unitmenu_17_viscosity);
INCLUDE_ROMOBJECT(unitmenu_18_acceleration);





// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
     (WORDPTR)unitdir_ident,
     (WORDPTR)LIB_MSGTABLE,
     (WORDPTR)LIB_HELPTABLE,

    (WORDPTR)unitmenu_0_main,
    (WORDPTR)unitmenu_1_tools,
    (WORDPTR)unitmenu_2_length,
    (WORDPTR)unitmenu_3_area,
    (WORDPTR)unitmenu_4_volume,
    (WORDPTR)unitmenu_5_time,
    (WORDPTR)unitmenu_6_speed,
    (WORDPTR)unitmenu_7_mass,
    (WORDPTR)unitmenu_8_force,
    (WORDPTR)unitmenu_9_energy,
    (WORDPTR)unitmenu_10_power,
    (WORDPTR)unitmenu_11_pressure,
    (WORDPTR)unitmenu_12_temperature,
    (WORDPTR)unitmenu_13_electrical,
    (WORDPTR)unitmenu_14_angles,
    (WORDPTR)unitmenu_15_light,
    (WORDPTR)unitmenu_16_radiation,
    (WORDPTR)unitmenu_17_viscosity,
    (WORDPTR)unitmenu_18_acceleration,
     0
};


// RETURN A POINTER TO THE END OF THE NEXT UNIT IDENTIFIER
// SEPARATOR SYMBOLS ALLOWED ARE ( ) * / ^

BYTEPTR rplNextUnitToken(BYTEPTR start,BYTEPTR end)
{
    while(start<end) {
        if((*start=='*')||(*start=='/')||(*start=='^')||(*start=='(')||(*start==')')) break;
        start=(BYTEPTR)utf8skip((char *)start,(char *)end);
    }
    return start;
}





//   THIS IS TEMPORARY AND WILL NEVER BE SEEN BY THE USER OR STORED ANYWHERE
const WORD const temp_ident[]=
{
    MKPROLOG(DOIDENT,1),
    TEXT2WORD('?',0,0,0)
};





void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplPushData(IPtr);
        return;
    }

    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE)
    {
        // THESE ARE OVERLOADABLE COMMANDS DISPATCHED FROM THE
        // OVERLOADABLE OPERATORS LIBRARY.

        int nargs=OVR_GETNARGS(CurOpcode);

        if(rplDepthData()<nargs) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if( (nargs==1) && !ISPROLOG(*rplPeekData(1))) {
            // COMMAND AS ARGUMENT
            if( (OPCODE(CurOpcode)==OVR_EVAL)||
                    (OPCODE(CurOpcode)==OVR_EVAL1)||
                    (OPCODE(CurOpcode)==OVR_XEQ) )
            {

                WORD saveOpcode=CurOpcode;
                CurOpcode=*rplPopData();
                // RECURSIVE CALL
                LIB_HANDLER();
                CurOpcode=saveOpcode;
                return;
            }
            else {
                rplError(ERR_INVALIDOPCODE);
                return;
            }
        }

        switch(OPCODE(CurOpcode))
        {
        case OVR_EVAL:
        case OVR_EVAL1:
        case OVR_XEQ:
        case OVR_NUM:
            // JUST LEAVE IT ON THE STACK
            return;
        case OVR_ABS:
        case OVR_ISTRUE:
        case OVR_NOT:
        case OVR_NEG:
        {
            if(!ISUNIT(*rplPeekData(1))) {
                rplError(ERR_UNITEXPECTED);
                return;
            }
            // JUST APPLY THE OPERATOR TO THE VALUE
            WORDPTR *stkclean=DSTop;
            rplPushData(rplPeekData(1)+1);
            rplCallOvrOperator(CurOpcode);
            if(Exceptions) { DSTop=stkclean; return; }
            // AND PUT BACK THE SAME UNIT
            BINT nlevels=rplUnitExplode(rplPeekData(2));
            rplUnitPopItem(nlevels);
            if(Exceptions) { DSTop=stkclean; return; }
            WORDPTR newunit=rplUnitAssemble(nlevels);
            if(!newunit) { DSTop=stkclean; return; }

            // FINAL CLEANUP
            rplDropData(nlevels);
            rplOverwriteData(1,newunit);

        return;
        }
        case OVR_MUL:
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                        LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                        (*symblib)();
                        return;
            }

            BINT nlevels1,nlevels2;
            WORDPTR *stkclean=DSTop;

            BINT isspec1,isspec2;
            BINT bothunit= (ISUNIT(*rplPeekData(1)) && ISUNIT(*rplPeekData(2)));

            isspec2=rplUnitIsSpecial(rplPeekData(1));
            isspec1=rplUnitIsSpecial(rplPeekData(2));

            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec1 && bothunit) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec2 && bothunit) {
                rplUnitReplaceSpecial(nlevels2);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels1=rplUnitSimplify(nlevels1+nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            WORDPTR newunit=rplUnitAssemble(nlevels1);
            if(!newunit) { DSTop=stkclean; return; }

            // FINAL CLEANUP
            rplDropData(nlevels1+1);
            rplOverwriteData(1,newunit);
            return;
        }

        case OVR_INV:
            // JUST PUT A NUMBER ONE IN THE STACK AND FALL THROUGH A NORMAL DIVISION
            rplPushData(rplPeekData(1));
            rplOverwriteData(2,(WORDPTR)one_bint);
            // DELIBERATE FALL THROUGH

        case OVR_DIV:
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                        LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                        (*symblib)();
                        return;
            }

            BINT nlevels1,nlevels2;
            WORDPTR *stkclean=DSTop;

            BINT isspec1,isspec2,bothunit;

            bothunit= (ISUNIT(*rplPeekData(1)) && ISUNIT(*rplPeekData(2)));
            isspec2=rplUnitIsSpecial(rplPeekData(1));
            isspec1=rplUnitIsSpecial(rplPeekData(2));

            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec1 && bothunit) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec2) {
                rplUnitReplaceSpecial(nlevels2);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels1=rplUnitDivide(nlevels1+nlevels2,nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            WORDPTR newunit=rplUnitAssemble(nlevels1);
            if(!newunit) { DSTop=stkclean; return; }

            // FINAL CLEANUP
            rplDropData(nlevels1+1);
            rplOverwriteData(1,newunit);
            return;
        }
        case OVR_ADD:
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                        LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                        (*symblib)();
                        return;
            }

            BINT nlevels1,nlevels2;
            WORDPTR *stkclean=DSTop;

            BINT isspec1,isspec2;
            isspec2=rplUnitIsSpecial(rplPeekData(1));
            isspec1=rplUnitIsSpecial(rplPeekData(2));


            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }

            if(isspec1) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
                ScratchPointer3=rplPeekData(nlevels1);      // SAVE THE CONVERTED VALUE FOR LATER, rplUnitExplode USES ScratchPointers 1 AND 2
            }

            rplOverwriteData(nlevels1,(WORDPTR)one_bint);        // MAKE IT ONE TO PRODUCE A CONVERSION FACTOR
            nlevels1=rplUnitToBase(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels1=rplUnitSimplify(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }

            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));
            if(Exceptions) { DSTop=stkclean; return; }

            if(isspec2) {
                if(isspec1) {
                // WE SHOULD NEG AND SUBTRACT SPECIALS: A+B = A-(-B)
                rplPushData(rplPeekData(nlevels2));  // GET THE VALUE

                rplCallOvrOperator((CMD_OVR_NEG));
                if(Exceptions) { DSTop=stkclean; return; }

                rplOverwriteData(nlevels2,rplPopData());
                }

                rplUnitReplaceSpecial(nlevels2);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels2=rplUnitToBase(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels2=rplUnitSimplify(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }

            // BOTH UNITS WERE REDUCED TO THE BASE
            BINT result=rplUnitIsConsistent(nlevels1+nlevels2,nlevels2);
            if(!result) {
                rplError(ERR_INCONSISTENTUNITS);
                DSTop=stkclean;
                return;
            }

            // THE UNITS ARE CONSISTENT
            WORDPTR unitval;

            if(isspec1) rplPushData(ScratchPointer3);
            else {
                unitval=rplPeekData(2+nlevels1+nlevels2);
                if(ISUNIT(*unitval)) ++unitval; // IF IT'S A UNIT, POINT TO THE VALUE
                rplPushData(unitval);
            }
            rplPushData(rplPeekData(nlevels2+1));
            rplPushData(rplPeekData(nlevels1+nlevels2+2));
            rplCallOvrOperator((CMD_OVR_DIV));
            if(Exceptions) { DSTop=stkclean; return; }

            if(isspec2 && isspec1) rplCallOvrOperator((CMD_OVR_SUB));
                else rplCallOvrOperator((CMD_OVR_ADD));
            if(Exceptions) { DSTop=stkclean; return; }

            unitval=rplPopData();   // GET THE NEW VALUE

            rplDropData(nlevels2+nlevels1); // CLEANUP THE STACK, EXCEPT THE ORIGINAL ARGUMENTS
            rplPushData(unitval);               // PUSH THE NEW VALUE


            if(isspec2 && (!isspec1)) {
                nlevels1=rplUnitExplode(rplPeekData(2));    // EXPLODE THE OLD UNIT
                rplUnitReplaceSpecial(nlevels1);
            }
            else {
                nlevels1=rplUnitExplode(rplPeekData(3));    // EXPLODE THE OLD UNIT
                if(isspec1 && (!isspec2)) rplUnitReplaceSpecial(nlevels1);
            }
            if(Exceptions) { DSTop=stkclean; return; }
            rplUnitPopItem(nlevels1);           // AND REMOVE THE OLD VALUE, LEAVING THE NEW VALUE AND THE UNIT
            if(isspec1 && isspec2) rplUnitSpecialToDelta(nlevels1);
            else {
                if((isspec1+isspec2>0) && (isspec1+isspec2<7)) rplUnitReverseReplaceSpecial(nlevels1);
            }

            WORDPTR newunit=rplUnitAssemble(nlevels1);
            if(!newunit) { DSTop=stkclean; return; }

            // FINAL CLEANUP
            rplDropData(nlevels1+1);
            rplOverwriteData(1,newunit);

            return;
        }
        case OVR_SUB:
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                        LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                        (*symblib)();
                        return;
            }

            BINT nlevels1,nlevels2;
            WORDPTR *stkclean=DSTop;

            BINT isspec1,isspec2;
            isspec2=rplUnitIsSpecial(rplPeekData(1));
            isspec1=rplUnitIsSpecial(rplPeekData(2));


            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }

            if(isspec1) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
                ScratchPointer3=rplPeekData(nlevels1);      // SAVE THE CONVERTED VALUE FOR LATER, rplUnitExplode USES ScratchPointers 1 AND 2
            }

            rplOverwriteData(nlevels1,(WORDPTR)one_bint);        // MAKE IT ONE TO PRODUCE A CONVERSION FACTOR
            nlevels1=rplUnitToBase(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels1=rplUnitSimplify(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }

            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));
            if(Exceptions) { DSTop=stkclean; return; }

            if(isspec2) {
                if(!isspec1) {
                // WE SHOULD NEG AND ADD SPECIALS: A-B = A+(-B)
                rplPushData(rplPeekData(nlevels2));  // GET THE VALUE

                rplCallOvrOperator((CMD_OVR_NEG));
                if(Exceptions) { DSTop=stkclean; return; }

                rplOverwriteData(nlevels2,rplPopData());
                }

                rplUnitReplaceSpecial(nlevels2);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels2=rplUnitToBase(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels2=rplUnitSimplify(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }

            // BOTH UNITS WERE REDUCED TO THE BASE
            BINT result=rplUnitIsConsistent(nlevels1+nlevels2,nlevels2);
            if(!result) {
                rplError(ERR_INCONSISTENTUNITS);
                DSTop=stkclean;
                return;
            }

            // THE UNITS ARE CONSISTENT
            WORDPTR unitval;

            if(isspec1) rplPushData(ScratchPointer3);
            else {
                unitval=rplPeekData(2+nlevels1+nlevels2);
                if(ISUNIT(*unitval)) ++unitval; // IF IT'S A UNIT, POINT TO THE VALUE
                rplPushData(unitval);
            }
            rplPushData(rplPeekData(nlevels2+1));
            rplPushData(rplPeekData(nlevels1+nlevels2+2));
            rplCallOvrOperator((CMD_OVR_DIV));
            if(Exceptions) { DSTop=stkclean; return; }

            if(isspec2 && (!isspec1)) rplCallOvrOperator((CMD_OVR_ADD));
                else rplCallOvrOperator((CMD_OVR_SUB));
            if(Exceptions) { DSTop=stkclean; return; }

            unitval=rplPopData();   // GET THE NEW VALUE

            rplDropData(nlevels2+nlevels1); // CLEANUP THE STACK, EXCEPT THE ORIGINAL ARGUMENTS
            rplPushData(unitval);               // PUSH THE NEW VALUE


            if(isspec2 && (!isspec1)) {
                nlevels1=rplUnitExplode(rplPeekData(2));    // EXPLODE THE OLD UNIT
                rplUnitReplaceSpecial(nlevels1);
            }
            else {
                nlevels1=rplUnitExplode(rplPeekData(3));    // EXPLODE THE OLD UNIT
                if(isspec1 && (!isspec2)) rplUnitReplaceSpecial(nlevels1);
            }
            if(Exceptions) { DSTop=stkclean; return; }
            rplUnitPopItem(nlevels1);           // AND REMOVE THE OLD VALUE, LEAVING THE NEW VALUE AND THE UNIT
            if(isspec1 && isspec2) rplUnitSpecialToDelta(nlevels1);
            else {
                if((isspec1+isspec2>0) && (isspec1+isspec2<7)) rplUnitReverseReplaceSpecial(nlevels1);
            }

            WORDPTR newunit=rplUnitAssemble(nlevels1);
            if(!newunit) { DSTop=stkclean; return; }

            // FINAL CLEANUP
            rplDropData(nlevels1+1);
            rplOverwriteData(1,newunit);

            return;
        }

        case OVR_POW:
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                        LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                        (*symblib)();
                        return;
            }

            BINT nlevels1;
            WORDPTR *stkclean=DSTop;

            BINT isspec1;

            if(!ISNUMBER(*rplPeekData(1))) {
                rplError(ERR_REALEXPECTED);
                return;
            }
            isspec1=rplUnitIsSpecial(rplPeekData(2));

            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec1) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels1=rplUnitPow(nlevels1+1,nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
            WORDPTR newunit=rplUnitAssemble(nlevels1);
            if(!newunit) { DSTop=stkclean; return; }

            // FINAL CLEANUP
            rplDropData(nlevels1+1);
            rplOverwriteData(1,newunit);
            return;
        }

        case OVR_XROOT:
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                        LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                        (*symblib)();
                        return;
            }

            BINT nlevels1;
            WORDPTR *stkclean=DSTop;

            BINT isspec1;

            if(!ISNUMBER(*rplPeekData(1))) {
                rplError(ERR_REALEXPECTED);
                return;
            }

            rplPushData((WORDPTR)temp_ident);
            rplPushData((WORDPTR)one_bint);
            rplPushData(rplPeekData(3));


            isspec1=rplUnitIsSpecial(rplPeekData(5));

            nlevels1=rplUnitExplode(rplPeekData(5));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec1) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels1=rplUnitPow(nlevels1+3,nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
            WORDPTR newunit=rplUnitAssemble(nlevels1);
            if(!newunit) { DSTop=stkclean; return; }

            // FINAL CLEANUP
            rplDropData(nlevels1+3);
            rplOverwriteData(1,newunit);
            return;
        }

        case OVR_CMP:
            // COMPARISON OPERATIONS REQUIRE UNIT CONSISTENCY
            // FIRST, DO A SUBTRACTION
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                (*symblib)();
                return;
            }

            BINT nlevels1,nlevels2,swap=0;
            WORDPTR *stkclean=DSTop;

            BINT isspec1,isspec2;

            if(ISREAL(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                REAL p;
                rplReadReal(rplPeekData(2),&p);
                if(iszeroReal(&p)) {
                    swap=1;
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }
            if(ISBINT(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                BINT64 num=rplReadBINT(rplPeekData(2));

                if(num==0) {
                    swap=1;
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }



            isspec2=rplUnitIsSpecial(rplPeekData(1));
            isspec1=rplUnitIsSpecial(rplPeekData(2));


            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec1) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
                ScratchPointer3=rplPeekData(nlevels1);      // SAVE THE CONVERTED VALUE FOR LATER, rplUnitExplode USES ScratchPointers 1 AND 2

            }
            rplOverwriteData(nlevels1,(WORDPTR)one_bint);        // MAKE IT ONE TO PRODUCE A CONVERSION FACTOR
            nlevels1=rplUnitToBase(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels1=rplUnitSimplify(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }

            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec2) {
                rplUnitReplaceSpecial(nlevels2);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels2=rplUnitToBase(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels2=rplUnitSimplify(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }

            // BOTH UNITS WERE REDUCED TO THE BASE
            BINT result=rplUnitIsConsistent(nlevels1+nlevels2,nlevels2);
            if(!result) {
                rplError(ERR_INCONSISTENTUNITS);
                DSTop=stkclean;
                return;
            }

            // THE UNITS ARE CONSISTENT
            WORDPTR unitval;


            if(isspec1) rplPushData(ScratchPointer3);
            else {
                unitval=rplPeekData(2+nlevels1+nlevels2);
                if(ISUNIT(*unitval)) ++unitval; // IF IT'S A UNIT, POINT TO THE VALUE
                rplPushData(unitval);
            }
            rplPushData(rplPeekData(nlevels2+1));
            rplPushData(rplPeekData(nlevels1+nlevels2+2));
            rplCallOvrOperator((CMD_OVR_DIV));
            if(Exceptions) { DSTop=stkclean; return; }

            // INVERT THE RESULT IF THE OPERANDS WERE SWAPPED
            rplCallOvrOperator((CMD_OVR_CMP));
            if(swap)  rplCallOvrOperator((CMD_OVR_NEG));


            if(Exceptions) { DSTop=stkclean; return; }

            unitval=rplPopData();   // GET THE  RESULT
            rplDropData(nlevels2+nlevels1+1); // CLEANUP THE STACK
            rplOverwriteData(1,unitval);

            return;
        }


        case OVR_LT:

            // COMPARISON OPERATIONS REQUIRE UNIT CONSISTENCY
            // FIRST, DO A SUBTRACTION
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                (*symblib)();
                return;
            }

            BINT nlevels1,nlevels2,swap=0;
            WORDPTR *stkclean=DSTop;

            BINT isspec1,isspec2;

            if(ISREAL(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                REAL p;
                rplReadReal(rplPeekData(2),&p);
                if(iszeroReal(&p)) {
                    swap=1;
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }
            if(ISBINT(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                BINT64 num=rplReadBINT(rplPeekData(2));

                if(num==0) {
                    swap=1;
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }



            isspec2=rplUnitIsSpecial(rplPeekData(1));
            isspec1=rplUnitIsSpecial(rplPeekData(2));


            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec1) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
                ScratchPointer3=rplPeekData(nlevels1);      // SAVE THE CONVERTED VALUE FOR LATER, rplUnitExplode USES ScratchPointers 1 AND 2

            }
            rplOverwriteData(nlevels1,(WORDPTR)one_bint);        // MAKE IT ONE TO PRODUCE A CONVERSION FACTOR
            nlevels1=rplUnitToBase(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels1=rplUnitSimplify(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }

            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec2) {
                rplUnitReplaceSpecial(nlevels2);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels2=rplUnitToBase(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels2=rplUnitSimplify(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }

            // BOTH UNITS WERE REDUCED TO THE BASE
            BINT result=rplUnitIsConsistent(nlevels1+nlevels2,nlevels2);
            if(!result) {
                rplError(ERR_INCONSISTENTUNITS);
                DSTop=stkclean;
                return;
            }

            // THE UNITS ARE CONSISTENT
            WORDPTR unitval;


            if(isspec1) rplPushData(ScratchPointer3);
            else {
                unitval=rplPeekData(2+nlevels1+nlevels2);
                if(ISUNIT(*unitval)) ++unitval; // IF IT'S A UNIT, POINT TO THE VALUE
                rplPushData(unitval);
            }
            rplPushData(rplPeekData(nlevels2+1));
            rplPushData(rplPeekData(nlevels1+nlevels2+2));
            rplCallOvrOperator((CMD_OVR_DIV));
            if(Exceptions) { DSTop=stkclean; return; }

            // INVERT THE RESULT IF THE OPERANDS WERE SWAPPED
            if(swap) rplCallOvrOperator((CMD_OVR_GT));
            else rplCallOvrOperator((CMD_OVR_LT));

            if(Exceptions) { DSTop=stkclean; return; }

            unitval=rplPopData();   // GET THE  RESULT
            rplDropData(nlevels2+nlevels1+1); // CLEANUP THE STACK
            rplOverwriteData(1,unitval);

            return;
        }

        case OVR_GT:

            // COMPARISON OPERATIONS REQUIRE UNIT CONSISTENCY
            // FIRST, DO A SUBTRACTION
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                (*symblib)();
                return;
            }

            BINT nlevels1,nlevels2,swap=0;
            WORDPTR *stkclean=DSTop;

            BINT isspec1,isspec2;

            if(ISREAL(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                REAL p;
                rplReadReal(rplPeekData(2),&p);
                if(iszeroReal(&p)) {
                    swap=1;
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }
            if(ISBINT(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                BINT64 num=rplReadBINT(rplPeekData(2));

                if(num==0) {
                    swap=1;
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }



            isspec2=rplUnitIsSpecial(rplPeekData(1));
            isspec1=rplUnitIsSpecial(rplPeekData(2));


            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec1) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
                ScratchPointer3=rplPeekData(nlevels1);      // SAVE THE CONVERTED VALUE FOR LATER, rplUnitExplode USES ScratchPointers 1 AND 2

            }
            rplOverwriteData(nlevels1,(WORDPTR)one_bint);        // MAKE IT ONE TO PRODUCE A CONVERSION FACTOR
            nlevels1=rplUnitToBase(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels1=rplUnitSimplify(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }

            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec2) {
                rplUnitReplaceSpecial(nlevels2);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels2=rplUnitToBase(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels2=rplUnitSimplify(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }

            // BOTH UNITS WERE REDUCED TO THE BASE
            BINT result=rplUnitIsConsistent(nlevels1+nlevels2,nlevels2);
            if(!result) {
                rplError(ERR_INCONSISTENTUNITS);
                DSTop=stkclean;
                return;
            }

            // THE UNITS ARE CONSISTENT
            WORDPTR unitval;


            if(isspec1) rplPushData(ScratchPointer3);
            else {
                unitval=rplPeekData(2+nlevels1+nlevels2);
                if(ISUNIT(*unitval)) ++unitval; // IF IT'S A UNIT, POINT TO THE VALUE
                rplPushData(unitval);
            }
            rplPushData(rplPeekData(nlevels2+1));
            rplPushData(rplPeekData(nlevels1+nlevels2+2));
            rplCallOvrOperator((CMD_OVR_DIV));
            if(Exceptions) { DSTop=stkclean; return; }

            // INVERT THE RESULT IF THE OPERANDS WERE SWAPPED
            if(swap) rplCallOvrOperator((CMD_OVR_LT));
            else rplCallOvrOperator((CMD_OVR_GT));

            if(Exceptions) { DSTop=stkclean; return; }

            unitval=rplPopData();   // GET THE  RESULT
            rplDropData(nlevels2+nlevels1+1); // CLEANUP THE STACK
            rplOverwriteData(1,unitval);

            return;
        }

        case OVR_LTE:

            // COMPARISON OPERATIONS REQUIRE UNIT CONSISTENCY
            // FIRST, DO A SUBTRACTION
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                (*symblib)();
                return;
            }

            BINT nlevels1,nlevels2,swap=0;
            WORDPTR *stkclean=DSTop;

            BINT isspec1,isspec2;

            if(ISREAL(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                REAL p;
                rplReadReal(rplPeekData(2),&p);
                if(iszeroReal(&p)) {
                    swap=1;
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }
            if(ISBINT(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                BINT64 num=rplReadBINT(rplPeekData(2));

                if(num==0) {
                    swap=1;
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }



            isspec2=rplUnitIsSpecial(rplPeekData(1));
            isspec1=rplUnitIsSpecial(rplPeekData(2));


            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec1) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
                ScratchPointer3=rplPeekData(nlevels1);      // SAVE THE CONVERTED VALUE FOR LATER, rplUnitExplode USES ScratchPointers 1 AND 2

            }
            rplOverwriteData(nlevels1,(WORDPTR)one_bint);        // MAKE IT ONE TO PRODUCE A CONVERSION FACTOR
            nlevels1=rplUnitToBase(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels1=rplUnitSimplify(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }

            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec2) {
                rplUnitReplaceSpecial(nlevels2);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels2=rplUnitToBase(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels2=rplUnitSimplify(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }

            // BOTH UNITS WERE REDUCED TO THE BASE
            BINT result=rplUnitIsConsistent(nlevels1+nlevels2,nlevels2);
            if(!result) {
                rplError(ERR_INCONSISTENTUNITS);
                DSTop=stkclean;
                return;
            }

            // THE UNITS ARE CONSISTENT
            WORDPTR unitval;


            if(isspec1) rplPushData(ScratchPointer3);
            else {
                unitval=rplPeekData(2+nlevels1+nlevels2);
                if(ISUNIT(*unitval)) ++unitval; // IF IT'S A UNIT, POINT TO THE VALUE
                rplPushData(unitval);
            }
            rplPushData(rplPeekData(nlevels2+1));
            rplPushData(rplPeekData(nlevels1+nlevels2+2));
            rplCallOvrOperator((CMD_OVR_DIV));
            if(Exceptions) { DSTop=stkclean; return; }

            // INVERT THE RESULT IF THE OPERANDS WERE SWAPPED
            if(swap) rplCallOvrOperator((CMD_OVR_GTE));
            else rplCallOvrOperator((CMD_OVR_LTE));

            if(Exceptions) { DSTop=stkclean; return; }

            unitval=rplPopData();   // GET THE  RESULT
            rplDropData(nlevels2+nlevels1+1); // CLEANUP THE STACK
            rplOverwriteData(1,unitval);

            return;
        }
        case OVR_GTE:

            // COMPARISON OPERATIONS REQUIRE UNIT CONSISTENCY
            // FIRST, DO A SUBTRACTION
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                (*symblib)();
                return;
            }

            BINT nlevels1,nlevels2,swap=0;
            WORDPTR *stkclean=DSTop;

            BINT isspec1,isspec2;

            if(ISREAL(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                REAL p;
                rplReadReal(rplPeekData(2),&p);
                if(iszeroReal(&p)) {
                    swap=1;
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }
            if(ISBINT(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                BINT64 num=rplReadBINT(rplPeekData(2));

                if(num==0) {
                    swap=1;
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }



            isspec2=rplUnitIsSpecial(rplPeekData(1));
            isspec1=rplUnitIsSpecial(rplPeekData(2));


            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec1) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
                ScratchPointer3=rplPeekData(nlevels1);      // SAVE THE CONVERTED VALUE FOR LATER, rplUnitExplode USES ScratchPointers 1 AND 2

            }
            rplOverwriteData(nlevels1,(WORDPTR)one_bint);        // MAKE IT ONE TO PRODUCE A CONVERSION FACTOR
            nlevels1=rplUnitToBase(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels1=rplUnitSimplify(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }

            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec2) {
                rplUnitReplaceSpecial(nlevels2);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels2=rplUnitToBase(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels2=rplUnitSimplify(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }

            // BOTH UNITS WERE REDUCED TO THE BASE
            BINT result=rplUnitIsConsistent(nlevels1+nlevels2,nlevels2);
            if(!result) {
                rplError(ERR_INCONSISTENTUNITS);
                DSTop=stkclean;
                return;
            }

            // THE UNITS ARE CONSISTENT
            WORDPTR unitval;


            if(isspec1) rplPushData(ScratchPointer3);
            else {
                unitval=rplPeekData(2+nlevels1+nlevels2);
                if(ISUNIT(*unitval)) ++unitval; // IF IT'S A UNIT, POINT TO THE VALUE
                rplPushData(unitval);
            }
            rplPushData(rplPeekData(nlevels2+1));
            rplPushData(rplPeekData(nlevels1+nlevels2+2));
            rplCallOvrOperator((CMD_OVR_DIV));
            if(Exceptions) { DSTop=stkclean; return; }

            // INVERT THE RESULT IF THE OPERANDS WERE SWAPPED
            if(swap) rplCallOvrOperator((CMD_OVR_LTE));
            else rplCallOvrOperator((CMD_OVR_GTE));

            if(Exceptions) { DSTop=stkclean; return; }

            unitval=rplPopData();   // GET THE  RESULT
            rplDropData(nlevels2+nlevels1+1); // CLEANUP THE STACK
            rplOverwriteData(1,unitval);

            return;
        }

        case OVR_SAME:
        case OVR_EQ:
            // COMPARISON OPERATIONS REQUIRE UNIT CONSISTENCY
            // FIRST, DO A SUBTRACTION
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                (*symblib)();
                return;
            }

            BINT nlevels1,nlevels2;
            WORDPTR *stkclean=DSTop;

            BINT isspec1,isspec2;

            if(ISREAL(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                REAL p;
                rplReadReal(rplPeekData(2),&p);
                if(iszeroReal(&p)) {
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }
            if(ISBINT(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                BINT64 num=rplReadBINT(rplPeekData(2));

                if(num==0) {
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }



            isspec2=rplUnitIsSpecial(rplPeekData(1));
            isspec1=rplUnitIsSpecial(rplPeekData(2));


            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec1) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
                ScratchPointer3=rplPeekData(nlevels1);      // SAVE THE CONVERTED VALUE FOR LATER, rplUnitExplode USES ScratchPointers 1 AND 2

            }
            rplOverwriteData(nlevels1,(WORDPTR)one_bint);        // MAKE IT ONE TO PRODUCE A CONVERSION FACTOR
            nlevels1=rplUnitToBase(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels1=rplUnitSimplify(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }

            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec2) {
                rplUnitReplaceSpecial(nlevels2);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels2=rplUnitToBase(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels2=rplUnitSimplify(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }

            // BOTH UNITS WERE REDUCED TO THE BASE
            BINT result=rplUnitIsConsistent(nlevels1+nlevels2,nlevels2);
            if(!result) {
                rplError(ERR_INCONSISTENTUNITS);
                DSTop=stkclean;
                return;
            }

            // THE UNITS ARE CONSISTENT
            WORDPTR unitval;


            if(isspec1) rplPushData(ScratchPointer3);
            else {
                unitval=rplPeekData(2+nlevels1+nlevels2);
                if(ISUNIT(*unitval)) ++unitval; // IF IT'S A UNIT, POINT TO THE VALUE
                rplPushData(unitval);
            }
            rplPushData(rplPeekData(nlevels2+1));
            rplPushData(rplPeekData(nlevels1+nlevels2+2));
            rplCallOvrOperator((CMD_OVR_DIV));
            if(Exceptions) { DSTop=stkclean; return; }

            rplCallOvrOperator((CMD_OVR_EQ));

            if(Exceptions) { DSTop=stkclean; return; }

            unitval=rplPopData();   // GET THE  RESULT
            rplDropData(nlevels2+nlevels1+1); // CLEANUP THE STACK
            rplOverwriteData(1,unitval);

            return;
        }
        case OVR_NOTEQ:
            // COMPARISON OPERATIONS REQUIRE UNIT CONSISTENCY
            // FIRST, DO A SUBTRACTION
        {
            if(ISIDENT(*rplPeekData(1))||ISIDENT(*rplPeekData(2))) {
                // TREAT ANY IDENTS AS SYMBOLICS
                LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                (*symblib)();
                return;
            }

            BINT nlevels1,nlevels2;
            WORDPTR *stkclean=DSTop;

            BINT isspec1,isspec2;

            if(ISREAL(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                REAL p;
                rplReadReal(rplPeekData(2),&p);
                if(iszeroReal(&p)) {
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }
            if(ISBINT(*rplPeekData(2))) {
                // ONLY THE SPECIAL CASE OF ZERO NEEDS TO BE CONSIDERED
                // SINCE ZERO DOESN'T MAKE A GOOD CONVERSION FACTOR
                BINT64 num=rplReadBINT(rplPeekData(2));

                if(num==0) {
                    WORDPTR tmp=rplPeekData(1);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,tmp);
                }

            }



            isspec2=rplUnitIsSpecial(rplPeekData(1));
            isspec1=rplUnitIsSpecial(rplPeekData(2));


            nlevels1=rplUnitExplode(rplPeekData(2));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec1) {
                rplUnitReplaceSpecial(nlevels1);
                if(Exceptions) { DSTop=stkclean; return; }
                ScratchPointer3=rplPeekData(nlevels1);      // SAVE THE CONVERTED VALUE FOR LATER, rplUnitExplode USES ScratchPointers 1 AND 2

            }
            rplOverwriteData(nlevels1,(WORDPTR)one_bint);        // MAKE IT ONE TO PRODUCE A CONVERSION FACTOR
            nlevels1=rplUnitToBase(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels1=rplUnitSimplify(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }

            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));
            if(Exceptions) { DSTop=stkclean; return; }
            if(isspec2) {
                rplUnitReplaceSpecial(nlevels2);
                if(Exceptions) { DSTop=stkclean; return; }
            }

            nlevels2=rplUnitToBase(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            nlevels2=rplUnitSimplify(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }

            // BOTH UNITS WERE REDUCED TO THE BASE
            BINT result=rplUnitIsConsistent(nlevels1+nlevels2,nlevels2);
            if(!result) {
                rplError(ERR_INCONSISTENTUNITS);
                DSTop=stkclean;
                return;
            }

            // THE UNITS ARE CONSISTENT
            WORDPTR unitval;


            if(isspec1) rplPushData(ScratchPointer3);
            else {
                unitval=rplPeekData(2+nlevels1+nlevels2);
                if(ISUNIT(*unitval)) ++unitval; // IF IT'S A UNIT, POINT TO THE VALUE
                rplPushData(unitval);
            }
            rplPushData(rplPeekData(nlevels2+1));
            rplPushData(rplPeekData(nlevels1+nlevels2+2));
            rplCallOvrOperator((CMD_OVR_DIV));
            if(Exceptions) { DSTop=stkclean; return; }

            rplCallOvrOperator((CMD_OVR_NOTEQ));

            if(Exceptions) { DSTop=stkclean; return; }

            unitval=rplPopData();   // GET THE  RESULT
            rplDropData(nlevels2+nlevels1+1); // CLEANUP THE STACK
            rplOverwriteData(1,unitval);

            return;
        }


        }



    }


    switch(OPCODE(CurOpcode))
    {
    case UDEFINE:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR unit,name;
        unit=rplPeekData(2);
        name=rplPeekData(1);

        if( (!ISNUMBER(*unit))&&(!ISUNIT(*unit))) {
            rplError(ERR_UNITEXPECTED);
            return;
        }
        if(!ISIDENT(*name)) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        if( (name[1]&0xff)=='?') {
            // GIVEN NAME STARTS QUESTION MARK, SO
            // THIS UNIT WILL ACCEPT SI PREFIX

            // NEED TO REPLACE THE IDENT WITH A NEW
            // ONE THAT DOESN'T HAVE THE RIGHT ARROW

            BINT newlen=rplGetIdentLength(name);

            if(newlen<=1) {
                rplError(ERR_INVALIDUNITNAME);
                return;
            }

            if(!rplIsValidIdent((BYTEPTR)(name+1)+1,(BYTEPTR)(name+1)+newlen)) {
                rplError(ERR_INVALIDUNITNAME);
                return;
            }

            name=rplCreateIDENT(DOIDENTSIPREFIX,(BYTEPTR)(name+1)+1,(BYTEPTR)(name+1)+newlen);

            if(!name) return;
            rplOverwriteData(1,name);   // LEAVE IT ON THE STACK FOR GC PROTECTION
            unit=rplPeekData(2);    // RELOAD POINTER IN CASE IT MOVED

        }

        // HERE WE HAVE A PROPER NAME AND UNIT OBJECT

        // CHECK IF THE UNIT EXISTS OR CONFLICTS WITH ANOTHER UNIT
        BINT siindex=0;
        WORDPTR *found=rplUnitFindCustom(name,&siindex);

        if(found) {
            // HERE, WE HAVE A UNIT THAT WAS EXISTING
            if(siindex!=0) {
                rplError(ERR_INVALIDUNITNAME);
                return;
            }

            // JUST OVERWRITE THE UNIT
            found[0]=name;
            found[1]=unit;

            rplDropData(2);
            return;
        }


        // UNIT DOESN'T EXIST, WE NEED TO CREATE IT
        // GET THE UNITS DIRECTORY

        WORDPTR unitdir_obj=rplGetSettings((WORDPTR)unitdir_ident);

        if(!unitdir_obj) {
            WORDPTR *settings=rplFindDirbyHandle(SettingsDir);
            if(!settings) return;

            // NEED TO CREATE A NEW DIRECTORY
            unitdir_obj=rplCreateNewDir((WORDPTR)unitdir_ident,settings);
            if(!unitdir_obj) return;  // EXCEPTIONS SHOULD'VE BEEN RAISED ALREADY
            // RELOAD THE POINTERS FROM THE STACK, IN CASE THERE WAS A GC
            unit=rplPeekData(2);
            name=rplPeekData(1);
        }

        WORDPTR *unitdir=rplFindDirbyHandle(unitdir_obj);
        if(!unitdir) return;  // EXCEPTIONS SHOULD'VE BEEN RAISED ALREADY
        rplCreateGlobalInDir(name,unit,unitdir);
        rplDropData(2);
        return;

    }
    case UPURGE:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR name=rplPeekData(1);

        if(!ISIDENT(*name)) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        //  GET THE UNITS DIRECTORY

        WORDPTR unitdir_obj=rplGetSettings((WORDPTR)unitdir_ident);

        if(!unitdir_obj) {
        rplError(ERR_UNDEFINEDUNIT);
        return;
        }
        BINT siindex=0;
        WORDPTR *found=rplUnitFindCustom(name,&siindex);

        if( (!found) || (siindex!=0)) {
            rplError(ERR_UNDEFINEDUNIT);
            return;
        }

        rplPurgeForced(found);

        rplDropData(1);
        return;

    }
    case UVAL:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISUNIT(*rplPeekData(1))) {
            rplError(ERR_UNITEXPECTED);
            return;
        }
        WORDPTR *stkclean=DSTop;
        BINT nlevels=rplUnitExplode(rplPeekData(1));
        if(Exceptions) { DSTop=stkclean; return; }
        rplOverwriteData(nlevels+1,rplPeekData(nlevels));

        rplDropData(nlevels);         // POP EVERYTHING EXCEPT THE VALUE
        return;
    }
    case CONVERT:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }


        BINT nlevels1,nlevels2;
        WORDPTR *stkclean=DSTop;
        BINT isspec1,isspec2;
        isspec1=rplUnitIsSpecial(rplPeekData(1));
        isspec2=rplUnitIsSpecial(rplPeekData(2));



        nlevels1=rplUnitExplode(rplPeekData(1));
        if(Exceptions) { DSTop=stkclean; return; }
        if(isspec1) {
            rplUnitReplaceSpecial(nlevels1);
            if(Exceptions) { DSTop=stkclean; return; }
        }

        rplOverwriteData(nlevels1,(WORDPTR)one_bint);        // MAKE IT ONE TO PRODUCE A CONVERSION FACTOR
        nlevels1=rplUnitToBase(nlevels1);
        if(Exceptions) { DSTop=stkclean; return; }
        nlevels1=rplUnitSimplify(nlevels1);
        if(Exceptions) { DSTop=stkclean; return; }

        nlevels2=rplUnitExplode(rplPeekData(2+nlevels1));
        if(Exceptions) { DSTop=stkclean; return; }
        if(isspec2) {
            rplUnitReplaceSpecial(nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
        }

        nlevels2=rplUnitToBase(nlevels2);
        if(Exceptions) { DSTop=stkclean; return; }
        nlevels2=rplUnitSimplify(nlevels2);
        if(Exceptions) { DSTop=stkclean; return; }

        // BOTH UNITS WERE REDUCED TO THE BASE
        BINT result=rplUnitIsConsistent(nlevels1+nlevels2,nlevels2);
        if(!result) {
            rplError(ERR_INCONSISTENTUNITS);
            DSTop=stkclean;
            return;
        }

        // THE UNITS ARE CONSISTENT
        rplPushData(rplPeekData(nlevels2));
        rplPushData(rplPeekData(nlevels1+nlevels2+1));
        rplCallOvrOperator((CMD_OVR_DIV));
        if(Exceptions) { DSTop=stkclean; return; }


        if(isspec1) {
            rplUnitReverseReplaceSpecial2(isspec1);
            if(Exceptions) { DSTop=stkclean; return; }
        }




        WORDPTR unitval=rplPopData();   // GET THE NEW VALUE
        rplDropData(nlevels2+nlevels1); // CLEANUP THE STACK, EXCEPT THE ORIGINAL ARGUMENTS
        rplPushData(unitval);               // PUSH THE NEW VALUE
        nlevels1=rplUnitExplode(rplPeekData(2));    // EXPLODE THE NEW UNIT
        if(Exceptions) { DSTop=stkclean; return; }
        rplUnitPopItem(nlevels1);           // AND REMOVE THE OLD VALUE, LEAVING THE NEW VALUE AND THE UNIT


        WORDPTR newunit=rplUnitAssemble(nlevels1);
        if(!newunit) { DSTop=stkclean; return; }

        // FINAL CLEANUP
        rplDropData(nlevels1+1);
        rplOverwriteData(1,newunit);

        return;
    }

    case UFACT:
    {
        // THIS IS EASILY ACHIEVED WITH << SWAP OVER / UBASE * >>
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // DO SWAP OVER
        WORDPTR *savestk=DSTop;
        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));

        rplCallOvrOperator((CMD_OVR_DIV));
        if(Exceptions) { DSTop=savestk; return; }

        rplCallOperator(MKOPCODE(LIBRARY_NUMBER,UBASE));
        if(Exceptions) { DSTop=savestk; return; }

        rplPushData(rplPeekData(2));

        rplCallOvrOperator((CMD_OVR_MUL));
        if(Exceptions) { DSTop=savestk; return; }

        rplOverwriteData(3,rplPeekData(1));

        rplDropData(2);

    return;
    }
    case SYMBTOUNIT:
        // DELIBERATE FALL THROUGH
    case TOUNIT:
        // THIS IS IDENTICAL TO OVR_MUL
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISUNIT(*rplPeekData(1))) {
            rplError(ERR_UNITEXPECTED);
            return;
        }

        rplCallOvrOperator((CMD_OVR_MUL));
        return;
    }

    case UBASE:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISUNIT(*rplPeekData(1))) {
            rplError(ERR_UNITEXPECTED);
            return;
        }
        WORDPTR *stkclean=DSTop;
        BINT nlevels=rplUnitExplode(rplPeekData(1));
        if(Exceptions) { DSTop=stkclean; return; }

        nlevels=rplUnitToBase(nlevels);
        if(Exceptions) { DSTop=stkclean; return; }

        nlevels=rplUnitSimplify(nlevels);
        if(Exceptions) { DSTop=stkclean; return; }

        WORDPTR newunit=rplUnitAssemble(nlevels);
        if(!newunit) { DSTop=stkclean; return; }

        // FINAL CLEANUP
        rplDropData(nlevels);
        rplOverwriteData(1,newunit);

        return;
    }

        // STANDARIZED OPCODES:
        // --------------------
        // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES

    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors
    {

        BYTEPTR ptr=(BYTEPTR)TokenStart;

        if(*ptr=='_') {
        // STARTS WITH THE UNIT, CHECK IF WE ARE IN A UNIT CONSTRUCT


            if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,0)) {
            // IF  WE ARE NOT IN A UNIT CONSTRUCT,  ADD A NUMBER ONE AND START THE OBJECT
            ScratchPointer2=CompileEnd; // rplCompileIDENT uses ScratchPointer1
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,0));
            rplCompileAppend(MAKESINT(1));
        }
            // THE NUMBER WAS COMPILED PROPERLY, NOW ADD THE UNIT ITSELF

            // START COMPILING THE UNIT EXPRESSION

            // ONLY ALLOWS IDENTIFIERS, * AND / OPERATIONS BETWEEN THEM
            // ALSO ALLOWS ^ BUT ONLY WITH REAL EXPONENTS
            // PARENTHESIS ARE SUPPORTED BUT REMOVED AT COMPILE TIME s^2/(Kg*m) --> s^2*Kg^-1*m^-1
            // MAXIMUM 8 LEVELS SUPPORTED

            BYTEPTR nextptr,endptr;
            BINT count=0;
            BINT exponent=1,negexp=0,needident=0,needexp=0;
            BINT groupoff[8];
            BINT groupexp[8];
            BINT groupidx=0;
            WORD Locale=rplGetSystemLocale();
            BINT toklen;
            BINT tokstart=1;

            nextptr=ptr+1;
            endptr=(BYTEPTR)BlankStart;
            toklen=endptr-nextptr;

            if(*nextptr=='[') {
                // THERE'S A UNIT WRAPPED IN BRACKETS
                ++nextptr;
                ++tokstart;
                // FIND MATCHING BRACKET
                endptr=nextptr;

                while( (endptr<(BYTEPTR)BlankStart) && (*endptr!=']')) {
                    endptr=(BYTEPTR)utf8skip((char *)endptr,(char *)BlankStart);
                }

                // IT'S OK TO REACH END OF TOKEN WITHOUT A BRACKET
                // SINCE IT MAY HAVE BEEN SPLIT BY THE MATRIX LIB
                /*if(endptr>=(BYTEPTR)BlankStart) {
                 // CLOSING BRACKET WAS NEVER FOUND
                    RetNum=ERR_SYNTAX;
                    return;
                }*/
                toklen=endptr-nextptr;

            }

            // HERE WE HAVE A NEW TOTAL LENGTH OF THE TOKEN IN toklen, AND endptr PROPERLY SET


            needident=1;

            while(nextptr<endptr) {
                if(needident) {

                // HANDLE THE SPECIAL CASE OF A PARENTHESIS

                if(*nextptr=='(') {
                    // OPEN A NEW GROUP
                    groupoff[groupidx]=CompileEnd-*(ValidateTop-1); // STORE THE OFFSET OF THE CURRENT OBJECT
                    groupexp[groupidx]=exponent;
                    ++groupidx;
                    if(groupidx>8) {
                        // NO MORE THAN 8 NESTED LEVELS ALLOWED
                        rplError(ERR_INVALIDUNITDEFINITION);
                        RetNum=ERR_SYNTAX;
                        return;
                    }
                    // SET THE EXPONENT FOR ALL IDENTS
                    if(negexp) {
                        exponent=-exponent;
                        negexp=0;
                    }


                    ++nextptr;
                    ++count;
                    continue;
                }

                // HANDLE SPECIAL CASE OF '1/X'

                if(*nextptr=='1') {
                    ++nextptr;
                    ++count;
                    needident=0;
                    continue;
                }


                // GET THE NEXT IDENT
                BYTEPTR nameend=rplNextUnitToken(nextptr,endptr);

                if(nameend<=nextptr) {
                    rplError(ERR_INVALIDUNITDEFINITION);
                    RetNum=ERR_SYNTAX;
                    return;
                }

                if(!rplIsValidIdent(nextptr,nameend)) {
                    rplError(ERR_INVALIDUNITDEFINITION);
                    RetNum=ERR_SYNTAX;
                    return;
                }

                BINT nletters=utf8nlen((char *)nextptr,(char *)nameend);
                // COMPILE THE IDENT

                rplCompileIDENT(DOIDENT,nextptr,nameend);
                if(Exceptions) {
                RetNum=ERR_INVALID;
                return;
                }


                // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+tokstart,(char *)BlankStart,count)+(nameend-nextptr);
                endptr=((BYTEPTR)TokenStart)+tokstart+toklen;

                count+=nletters;
                needexp=1;
                needident=0;

                }
                else {
                    // NOT LOOKING FOR AN IDENTIFIER
                if(*nextptr==')') {

                    // END OF A GROUP
                    if(!groupidx) {
                        rplError(ERR_INVALIDUNITDEFINITION);
                        RetNum=ERR_SYNTAX;
                        return;
                    }
                    if(needexp) {
                        BINT finalexp=(negexp)? -exponent:exponent;
                        rplCompileAppend(MAKESINT(finalexp));
                        rplCompileAppend(MAKESINT(1));
                        // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                        nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+tokstart,(char *)BlankStart,count);
                        endptr=((BYTEPTR)TokenStart)+tokstart+toklen;

                        needexp=0;
                    }

                    if(*(nextptr+1)=='^') {
                        // TODO: HANDLE SPECIAL CASE OF A GROUP TO AN EXPONENT

                        nextptr++;
                        count++;

                        // DO THE EXACT SAME THING TO READ THE EXPONENT

                        if(*(nextptr+1)=='(') {
                            // THE EXPONENT IS A FRACTION OR NUMERIC EXPRESSION


                         nextptr+=2;
                         count+=2;

                             BYTEPTR numend=rplNextUnitToken(nextptr,endptr);

                             if(numend<=nextptr) {
                                 rplError(ERR_INVALIDUNITDEFINITION);
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             // GET THE NUMERATOR INTO RReg[0]
                             newRealFromText(&RReg[0],(char *)nextptr,(char *)numend,Locale);

                             if(RReg[0].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                                 // BAD EXPONENT!
                                 rplError(ERR_EXPECTEDREALEXPONENT);
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             BINT nletters=utf8nlen((char *)nextptr,(char *)numend);

                             count+=nletters;
                             nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+tokstart,(char *)BlankStart,count);

                             // ONLY OPERATOR ALLOWED HERE IS DIVISION

                             if(*nextptr==')') {
                                 // JUST A NUMBER WITHIN PARENTHESIS, SET DENOMINATOR TO 1
                                 rplOneToRReg(1);
                                 nletters=1;
                             }
                             else {

                             if(*nextptr!='/') {
                                 rplError(ERR_EXPECTEDREALEXPONENT);
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             ++nextptr;
                             ++count;

                             BYTEPTR numend=rplNextUnitToken(nextptr,endptr);

                             if(numend<=nextptr) {
                                 rplError(ERR_INVALIDUNITDEFINITION);
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             // GET THE DENOMINATOR INTO RReg[1]
                             newRealFromText(&RReg[1],(char *)nextptr,(char *)numend,Locale);

                             if(RReg[1].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                                 // BAD EXPONENT!
                                 rplError(ERR_EXPECTEDREALEXPONENT);
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             nletters=utf8nlen((char *)nextptr,(char *)numend);

                             nextptr+=nletters;

                             if(*nextptr!=')') {
                                 rplError(ERR_INVALIDUNITDEFINITION);
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             ++nletters;

                             }



                             // RESTORE POINTERS AND CONTINUE

                             count+=nletters;
                        }
                        else {
                        // ONLY A REAL NUMBER SUPPORTED AS EXPONENT
                        // GET THE NEXT TOKEN
                        nextptr++;
                        BYTEPTR numend=rplNextUnitToken(nextptr,endptr);

                        if(numend<=nextptr) {
                            rplError(ERR_INVALIDUNITDEFINITION);
                            RetNum=ERR_SYNTAX;
                            return;
                        }

                        newRealFromText(&RReg[0],(char *)nextptr,(char *)numend,Locale);

                        if(RReg[0].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                            // BAD EXPONENT!
                            rplError(ERR_EXPECTEDREALEXPONENT);
                            RetNum=ERR_SYNTAX;
                            return;
                        }


                        rplOneToRReg(1);

                        BINT nletters=utf8nlen((char *)nextptr,(char *)numend);


                        count+=1+nletters;


                        }

                        // HERE WE HAVE NUMERATOR AND DENOMINATOR

                        // KEEP ONLY THE NUMERATOR SIGN
                        RReg[0].flags^=RReg[1].flags&F_NEGATIVE;
                        RReg[1].flags&=~F_NEGATIVE;


                        // CYCLE THROUGH ALL IDENTIFIERS SINCE THE GROUP STARTED
                        // MULTIPLY THEIR EXPONENTS BY THIS ONE

                        WORDPTR groupptr,unitptr,numptr,denptr;
                        BINT groupsize,offset=0;
                        REAL orgnum,orgden;

                        groupptr=*(ValidateTop-1)+groupoff[groupidx-1];

                        groupsize=CompileEnd-groupptr;

                        while(offset<groupsize) {
                            // FIRST THING IS TO RESTORE POSSIBLY MOVED POINTERS
                            groupptr=*(ValidateTop-1)+groupoff[groupidx-1];
                            unitptr=groupptr+offset;
                            numptr=rplSkipOb(unitptr);
                            denptr=rplSkipOb(numptr);


                            // NOW GET THE EXPONENTS OF THE NEXT UNIT
                            rplReadNumberAsReal(numptr,&orgnum);
                            rplReadNumberAsReal(denptr,&orgden);

                            mulReal(&RReg[2],&RReg[0],&orgnum);
                            mulReal(&RReg[3],&RReg[1],&orgden);

                            // AND COMPILE THEM AS NEW

                            BINT unitlen=rplObjSize(unitptr);

                            rplCompileAppendWords(unitlen);     // MAKE A COPY OF THE IDENT
                            if(Exceptions) {
                                RetNum=ERR_INVALID;
                                return;
                            }

                            groupptr=*(ValidateTop-1)+groupoff[groupidx-1];
                            unitptr=groupptr+offset;
                            numptr=rplSkipOb(unitptr);
                            denptr=rplSkipOb(numptr);

                            // MAKE A COPY OF THE IDENTIFIER
                            memmovew(CompileEnd-unitlen,unitptr,unitlen);

                            offset=rplSkipOb(denptr)-groupptr;

                            if(isintegerReal(&RReg[2]) && inBINT64Range(&RReg[2])) {
                                // EXPONENT IS AN INTEGER
                                BINT64 finalexp=getBINT64Real(&RReg[2]);
                                // COMPILE AS A BINT OR A SINT
                                rplCompileBINT(finalexp,DECBINT);
                                if(Exceptions) {
                                RetNum=ERR_INVALID;
                                return;
                                }


                            }
                            else {
                                // EXPONENT WILL HAVE TO BE A REAL
                                rplCompileReal(&RReg[2]);
                                if(Exceptions) {
                                RetNum=ERR_INVALID;
                                return;
                                }

                            }


                            if(isintegerReal(&RReg[3]) && inBINT64Range(&RReg[3])) {
                                // EXPONENT IS AN INTEGER
                                BINT64 finalexp=getBINT64Real(&RReg[3]);

                                // COMPILE AS A BINT OR A SINT
                                rplCompileBINT(finalexp,DECBINT);
                                if(Exceptions) {
                                RetNum=ERR_INVALID;
                                return;
                                }


                            }
                            else {
                                // EXPONENT WILL HAVE TO BE A REAL
                                rplCompileReal(&RReg[3]);
                                if(Exceptions) {
                                RetNum=ERR_INVALID;
                                return;
                                }

                            }



                        } // AND REPEAT FOR ALL IDENTIFIERS

                        // HERE WE HAVE THE ENTIRE GROUP DUPLICATED, WE NEED TO MOVE THE MEMORY

                        groupptr=*(ValidateTop-1)+groupoff[groupidx-1];

                        memmovew(groupptr,groupptr+groupsize,CompileEnd-(groupptr+groupsize));
                        CompileEnd-=groupsize;



                    }


                    needexp=0;
                    --groupidx;
                    exponent=groupexp[groupidx];    // RESTORE EXPONENT FROM UPPER GROUP
                    if(exponent<0) exponent=-exponent;
                    negexp=0;
                    ++nextptr;
                    ++count;
                    continue;
                }

                if(*nextptr=='*') {
                    if(needexp) {
                        BINT finalexp=(negexp)? -exponent:exponent;
                        rplCompileAppend(MAKESINT(finalexp));
                        rplCompileAppend(MAKESINT(1));
                        // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                        nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+tokstart,(char *)BlankStart,count);
                        endptr=((BYTEPTR)TokenStart)+tokstart+toklen;

                        needexp=0;
                        negexp=0;
                    }


                    // NOTHING TO DO ON MULTIPLICATION
                   needident=1;
                   ++nextptr;
                    ++count;
                    continue;
                }
                if(*nextptr=='/') {
                    if(needexp) {
                        BINT finalexp=(negexp)? -exponent:exponent;
                        rplCompileAppend(MAKESINT(finalexp));
                        rplCompileAppend(MAKESINT(1));
                        // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                        nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+tokstart,(char *)BlankStart,count);
                        endptr=((BYTEPTR)TokenStart)+tokstart+toklen;

                        needexp=0;
                    }

                    // NEGATE THE EXPONENT FOR THE NEXT IDENT
                    negexp=1;
                    needident=1;
                    ++nextptr;
                    ++count;
                    continue;
                }

                if(*nextptr=='^') {

                    if(!needexp) {
                        rplError(ERR_INVALIDUNITDEFINITION);
                        RetNum=ERR_SYNTAX;
                        return;
                    }


                    if(*(nextptr+1)=='(') {
                        // THE EXPONENT IS A FRACTION OR NUMERIC EXPRESSION


                     nextptr+=2;
                     count+=2;

                         BYTEPTR numend=rplNextUnitToken(nextptr,endptr);

                         if(numend<=nextptr) {
                             rplError(ERR_INVALIDUNITDEFINITION);
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         // GET THE NUMERATOR INTO RReg[0]
                         newRealFromText(&RReg[0],(char *)nextptr,(char *)numend,Locale);

                         if(RReg[0].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                             // BAD EXPONENT!
                             rplError(ERR_EXPECTEDREALEXPONENT);
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         BINT nletters=utf8nlen((char *)nextptr,(char *)numend);

                         count+=nletters;
                         nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+tokstart,(char *)BlankStart,count);

                         // ONLY OPERATOR ALLOWED HERE IS DIVISION

                         if(*nextptr==')') {
                             // JUST A NUMBER WITHIN PARENTHESIS, SET DENOMINATOR TO 1
                             rplOneToRReg(1);
                             nletters=1;
                         }
                         else {

                         if(*nextptr!='/') {
                             rplError(ERR_INVALIDUNITDEFINITION);
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         ++nextptr;
                         ++count;

                         BYTEPTR numend=rplNextUnitToken(nextptr,endptr);

                         if(numend<=nextptr) {
                             rplError(ERR_INVALIDUNITDEFINITION);
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         // GET THE DENOMINATOR INTO RReg[1]
                         newRealFromText(&RReg[1],(char *)nextptr,(char *)numend,Locale);

                         if(RReg[1].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                             // BAD EXPONENT!
                             rplError(ERR_EXPECTEDREALEXPONENT);
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         nletters=utf8nlen((char *)nextptr,(char *)numend);

                         nextptr+=nletters;

                         if(*nextptr!=')') {
                             rplError(ERR_INVALIDUNITDEFINITION);
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         ++nletters;

                         }


                         // HERE WE HAVE NUMERATOR AND DENOMINATOR

                         // KEEP ONLY THE NUMERATOR SIGN
                         RReg[0].flags^=RReg[1].flags&F_NEGATIVE;
                         RReg[1].flags&=~F_NEGATIVE;

                         if(isintegerReal(&RReg[0]) && inBINT64Range(&RReg[0])) {
                             // EXPONENT IS AN INTEGER
                             BINT64 finalexp=getBINT64Real(&RReg[0]);
                             finalexp*=exponent;
                             if(negexp) finalexp=-finalexp;

                             // COMPILE AS A BINT OR A SINT
                             rplCompileBINT(finalexp,DECBINT);
                             if(Exceptions) {
                             RetNum=ERR_INVALID;
                             return;
                             }


                         }
                         else {
                             // EXPONENT WILL HAVE TO BE A REAL
                             BINT sign=(negexp)? -exponent:exponent;

                             if(sign<0) RReg[0].flags^=F_NEGATIVE;

                             rplCompileReal(&RReg[0]);
                             if(Exceptions) {
                             RetNum=ERR_INVALID;
                             return;
                             }

                         }


                         if(isintegerReal(&RReg[1]) && inBINT64Range(&RReg[1])) {
                             // EXPONENT IS AN INTEGER
                             BINT64 finalexp=getBINT64Real(&RReg[1]);

                             // COMPILE AS A BINT OR A SINT
                             rplCompileBINT(finalexp,DECBINT);
                             if(Exceptions) {
                             RetNum=ERR_INVALID;
                             return;
                             }


                         }
                         else {
                             // EXPONENT WILL HAVE TO BE A REAL
                             rplCompileReal(&RReg[1]);
                             if(Exceptions) {
                             RetNum=ERR_INVALID;
                             return;
                             }

                         }


                         // RESTORE POINTERS AND CONTINUE

                         count+=nletters;
                         // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                         nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+tokstart,(char *)BlankStart,count);
                         endptr=((BYTEPTR)TokenStart)+tokstart+toklen;

                         needexp=0;
                         continue;



                    }
                    else {
                    // ONLY A REAL NUMBER SUPPORTED AS EXPONENT
                    // GET THE NEXT TOKEN
                    nextptr++;
                    BYTEPTR numend=rplNextUnitToken(nextptr,endptr);

                    if(numend<=nextptr) {
                        rplError(ERR_INVALIDUNITDEFINITION);
                        RetNum=ERR_SYNTAX;
                        return;
                    }

                    newRealFromText(&RReg[0],(char *)nextptr,(char *)numend,Locale);

                    if(RReg[0].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                        // BAD EXPONENT!
                        rplError(ERR_EXPECTEDREALEXPONENT);
                        RetNum=ERR_SYNTAX;
                        return;
                    }


                    BINT nletters=utf8nlen((char *)nextptr,(char *)numend);


                    if(isintegerReal(&RReg[0]) && inBINT64Range(&RReg[0])) {
                        // EXPONENT IS AN INTEGER
                        BINT64 finalexp=getBINT64Real(&RReg[0]);
                        finalexp*=exponent;
                        if(negexp) finalexp=-finalexp;

                        // COMPILE AS A BINT OR A SINT
                        rplCompileBINT(finalexp,DECBINT);
                        if(Exceptions) {
                        RetNum=ERR_INVALID;
                        return;
                        }
                        rplCompileAppend(MAKESINT(1));


                    }
                    else {
                        // EXPONENT WILL HAVE TO BE A REAL
                        BINT sign=(negexp)? -exponent:exponent;

                        if(sign<0) RReg[0].flags^=F_NEGATIVE;

                        rplCompileReal(&RReg[0]);
                        if(Exceptions) {
                        RetNum=ERR_INVALID;
                        return;
                        }

                        rplCompileAppend(MAKESINT(1));

                    }

                    count+=1+nletters;
                    // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                    nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+tokstart,(char *)BlankStart,count);
                    endptr=((BYTEPTR)TokenStart)+tokstart+toklen;

                    needexp=0;
                    continue;
                    }

                }

                // AT THIS POINT ANYTHING ELSE IS A SYNTAX ERROR
                rplError(ERR_INVALIDUNITDEFINITION);
                RetNum=ERR_SYNTAX;
                return;

                }
            }   // END WHILE

            if(needexp) {
                BINT finalexp=(negexp)? -exponent:exponent;
                rplCompileAppend(MAKESINT(finalexp));
                rplCompileAppend(MAKESINT(1));

            }
            if(needident) {
                rplError(ERR_INVALIDUNITDEFINITION);
                RetNum=ERR_SYNTAX;
                return;
            }

            // HERE WE SHOULD HAVE A UNIT OBJECT PROPERLY COMPILED!

            endptr=((BYTEPTR)TokenStart)+tokstart+toklen;

            if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,0)) {
            // THERE'S NO CONSTRUCT, WE MUST COMPILE AN ATOMIC OBJECT
            if( (endptr==(BYTEPTR)BlankStart)&&(tokstart>1)) {
                    // THIS IMMEDIATE MODE REQUIRES THE CLOSING BRACKET
                    rplError(ERR_INVALIDUNITDEFINITION);
                    RetNum=ERR_SYNTAX;
                    return;
                }
            // FIX THE SIZE OF THE OBJECT
            BINT sizewords=CompileEnd-ScratchPointer2;
            *ScratchPointer2=MKPROLOG(LIBRARY_NUMBER,sizewords-1);
            // AND ADD THE UNIT APPLY OPERATOR

            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,SYMBTOUNIT));

            RetNum=OK_CONTINUE;
            return;

            }


            if( (endptr==(BYTEPTR)BlankStart)&&(tokstart>1)) {
                // THE NEXT TOKEN MUST BE A CLOSING BRACKET
                // REQUEST IT TO THE COMPILER
                RetNum=OK_NEEDMORE;
                return;
            }
            RetNum=OK_ENDCONSTRUCT;
            return;
        }

        // DOESN'T START WITH '_'
        // FIRST LOOK FOR THE PRESENCE OF THE '_' SEPARATOR INSIDE THE TOKEN

        // BUT IGNORE IT IF THE SYMBOLIC START IS PRESENT

        if(*ptr=='\'') {
            RetNum=ERR_NOTMINE;
            return;
        }

        int f;

        for(f=0;f<(int)TokenLen;++f)
        {
            if(*ptr=='_') break;
            ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart);
        }

        if(f==(int)TokenLen) {
            // NOT FOUND, THIS IS NOT A UNIT
            libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
            return;
        }

        // THERE IS A '_', NOW SPLIT THE TOKEN AND START A PROLOG OF A UNIT

        if( (*(ptr-1)==']') || (*(ptr-1)=='}') ) {
            // LET THE MATRIX OR LIST FINISH COMPILING, THEN DO
            // A UNIT-APPLY OPERATOR INSTEAD OF COMPILING A UNIT OBJECT

            BlankStart=NextTokenStart=(WORDPTR)utf8nskip((char * )TokenStart,(char *)BlankStart,f);
            RetNum=ERR_NOTMINE_SPLITTOKEN;
            return;

        }

        rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,0));

        BlankStart=NextTokenStart=(WORDPTR)utf8nskip((char * )TokenStart,(char *)BlankStart,f);
        RetNum=OK_STARTCONSTRUCT_SPLITTOKEN;
        return;
    }

    case OPCODE_COMPILECONT:
    {
        // THE OBJECT SHOULD'VE BEEN PROPERLY COMPILED
        // THE ONLY ACCEPTED TOKEN IS THE CLOSING BRACKET
        if((TokenLen==1)&& (*((BYTEPTR)TokenStart)==']')) {
            RetNum=OK_ENDCONSTRUCT;
            return;
        }
        // ANYTHING ELSE SHOULD BE A SYNTAX ERROR
        rplError(ERR_INVALIDUNITDEFINITION);
        RetNum=ERR_SYNTAX;
        return;

    }
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string
        // DecompMode = infix mode number, 0 = RPL mode

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors


        if(ISPROLOG(*DecompileObject)) {


            // THIS IS CHEATING, BUT LOOK AT THE TEXT STRING
            // TO SEE IF THE SYMBOLIC UNIT OPERATOR WAS ALREADY INCLUDED
            // AND SKIP THE NUMERIC PART OF THE UNIT IF IT WAS

            BYTEPTR decstring=(BYTEPTR)DecompStringEnd;
            BINT closebracket=1, addnumber=1;

            if( (decstring[-2]!='_') || (decstring[-1]!='[')) {

            // ALSO, IF NOT IN SYMBOLIC MODE, CHECK IF THE NEXT OBJECT IS THE SYMBTOUNIT COMMAND
            if(!DecompMode) {
                WORDPTR nextobj=rplSkipOb(DecompileObject);
                if(nextobj<EndOfObject) {
                    if(*nextobj==MKOPCODE(LIBRARY_NUMBER,SYMBTOUNIT)) {
                        // SPECIAL CASE, DON'T ADD THE NUMERIC PART OF THE UNIT AND DON'T
                        addnumber=0;
                    }
                }
            }

            // DO AN EMBEDDED DECOMPILATION OF THE VALUE OBJECT
            if(addnumber) {
                rplDecompile(DecompileObject+1,DECOMP_EMBEDDED | ((CurOpcode==OPCODE_DECOMPEDIT)? DECOMP_EDIT:0));    // RUN EMBEDDED
                if(Exceptions) { RetNum=ERR_INVALID; return; }
            }

            // NOW ADD THE UNIT
            rplDecompAppendChar('_');

            // NO NEED TO USE BRACKETS UNLESS IT'S A SYMBOLIC
            if(DecompMode) rplDecompAppendChar('[');
            else closebracket=0;


            }

            BINT offset=1;
            BINT totalsize=rplObjSize(DecompileObject);
            BINT needmult=0;
            BINT Format=4 | ((CurOpcode==OPCODE_DECOMPEDIT)? FMT_CODE:0);  // SIMPLE FORMAT FOR ALL EXPONENTS, ONLY 4 DECIMAL PLACES IS ENOUGH
            UBINT64 Locale=rplGetSystemLocale();

            offset+=rplObjSize(DecompileObject+1);  // SKIP THE MAIN VALUE

            while(offset<totalsize) {

                // TAKE A LOOK AT THE EXPONENT
                WORDPTR expnum,expden;
                REAL rnum,rden;
                expnum=rplSkipOb(DecompileObject+offset);
                expden=rplSkipOb(expnum);
                rplReadNumberAsReal(expnum,&rnum);
                rplReadNumberAsReal(expden,&rden);

                if(!needmult) {
                    if(rnum.flags&F_NEGATIVE) { rplDecompAppendChar('1'); needmult=1; }
                }

                if(needmult) {
                    // CHECK FOR THE SIGN OF THE EXPONENT, ADD A '*' IF POSITIVE, '/' IF NEGATIVE
                if(rnum.flags&F_NEGATIVE) { rplDecompAppendChar('/'); rnum.flags^=F_NEGATIVE; }
                else rplDecompAppendChar('*');
                }

                // DECOMPILE THE IDENTIFIER

                BYTEPTR ptr=(BYTEPTR)(DecompileObject+offset+OBJSIZE(*(DecompileObject+offset)));
                if(ptr[3]==0)
                    // WE HAVE A NULL-TERMINATED STRING, SO WE CAN USE THE STANDARD FUNCTION
                    rplDecompAppendString((BYTEPTR) (DecompileObject+offset+1));
                else
                    rplDecompAppendString2((BYTEPTR)(DecompileObject+offset+1),OBJSIZE(*(DecompileObject+offset))<<2);

                if(Exceptions) { RetNum=ERR_INVALID; return; }

                // ONLY ADD AN EXPONENT IF IT'S NOT ONE

                if(!((rden.len==1) && (rden.data[0]==1) && (rnum.len==1) && (rnum.data[0]==1))) {
                    rplDecompAppendChar('^');
                    if(!((rden.len==1) && (rden.data[0]==1))) {
                        // THIS IS A FRACTION
                        rplDecompAppendChar('(');

                        // ESTIMATE THE MAXIMUM STRING LENGTH AND RESERVE THE MEMORY

                        BYTEPTR string;

                        BINT len=formatlengthReal(&rnum,Format,Locale);

                        // RESERVE THE MEMORY FIRST
                        rplDecompAppendString2(0,len);

                        // NOW USE IT
                        string=(BYTEPTR)DecompStringEnd;
                        string-=len;

                        if(Exceptions) {
                            RetNum=ERR_INVALID;
                            return;
                        }
                        DecompStringEnd=(WORDPTR) formatReal(&rnum,(char *)string,Format,Locale);


                        rplDecompAppendChar('/');


                        len=formatlengthReal(&rden,Format,Locale);

                        // RESERVE THE MEMORY FIRST
                        rplDecompAppendString2(0,len);

                        // NOW USE IT
                        string=(BYTEPTR)DecompStringEnd;
                        string-=len;

                        if(Exceptions) {
                            RetNum=ERR_INVALID;
                            return;
                        }
                        DecompStringEnd=(WORDPTR) formatReal(&rden,(char *)string,Format,Locale);

                        rplDecompAppendChar(')');

                    }
                    else {
                        // JUST A NUMBER
                        // ESTIMATE THE MAXIMUM STRING LENGTH AND RESERVE THE MEMORY

                        BYTEPTR string;

                        BINT len=formatlengthReal(&rnum,Format,Locale);

                        // RESERVE THE MEMORY FIRST
                        rplDecompAppendString2(0,len);

                        // NOW USE IT
                        string=(BYTEPTR)DecompStringEnd;
                        string-=len;

                        if(Exceptions) {
                            RetNum=ERR_INVALID;
                            return;
                        }
                        DecompStringEnd=(WORDPTR) formatReal(&rnum,(char *)string,Format,Locale);




                    }
                }

                needmult=1;

                // SKIP THE THREE OBJECTS
                offset+=rplObjSize(DecompileObject+offset);
                offset+=rplObjSize(DecompileObject+offset);
                offset+=rplObjSize(DecompileObject+offset);


            }


            if(closebracket) rplDecompAppendChar(']');
            // DONE
            RetNum=OK_CONTINUE;
            return;

        }

        // MANUALLY DECOMPILE THE OPERATOR _ WHEN NOT IN SYMBOLIC MODE

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,SYMBTOUNIT)) {
            if(!DecompMode) {
               // THE UNIT ITSELF ELIMINATED THE NUMBER, SO DON'T INCLUDE ANY OUTPUT
               //rplDecompAppendString("→UNIT");
               // NEED TO REMOVE THE LAST SPACE TO PREVENT A DOUBLE SPACE
               BYTEPTR lastspace=(BYTEPTR)DecompStringEnd;
               --lastspace;
               if(*lastspace==' ') DecompStringEnd=(WORDPTR)lastspace;
               RetNum=OK_CONTINUE;
               return;
            }
        }



        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // Library receives:
        // CurrentConstruct = SET TO THE CURRENT ACTIVE CONSTRUCT TYPE
        // LastCompiledObject = POINTER TO THE LAST OBJECT THAT WAS COMPILED, THAT NEEDS TO BE VERIFIED

        // VALIDATE RETURNS:
        // RetNum =  OK_CONTINUE IF THE OBJECT IS ACCEPTED, ERR_INVALID IF NOT.


        RetNum=OK_CONTINUE;
        return;

    case OPCODE_PROBETOKEN:
        // PROBETOKEN FINDS A VALID WORD AT THE BEGINNING OF THE GIVEN TOKEN AND RETURNS
        // INFORMATION ABOUT IT. THIS OPCODE IS MANDATORY

        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND

    {

        BYTEPTR ptr=(BYTEPTR)TokenStart;

        if( (TokenLen>=2) && (ptr[0]=='_') && (ptr[1]=='[')) {

            BYTEPTR endptr;
            BINT count=2;
            // FIND MATCHING BRACKET
            endptr=ptr+2;

            while( (endptr<(BYTEPTR)BlankStart) && (*endptr!=']')) {
                endptr=(BYTEPTR)utf8skip((char *)endptr,(char *)BlankStart);
                ++count;
            }

            if(endptr<(BYTEPTR)BlankStart) ++count;     // INCLUDE THE CLOSING BRACKET


            RetNum=OK_TOKENINFO | MKTOKENINFO(count,TITYPE_POSTFIXOP,2,1);
            return;
        }



            libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
            return;

    }

    case OPCODE_GETINFO:
            if(ISPROLOG(*DecompileObject)) {
                RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_NUMBER,0,1);
                return;
            }
            libGetInfo2(*DecompileObject,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        return;


    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) {
        // BASIC CHECKS
        WORDPTR ptr,objend;

        objend=rplSkipOb(ObjectPTR);
        ptr=ObjectPTR+1;

        ptr=rplSkipOb(ptr); // TODO: CHECK IF THE VALUE IS A VALID OBJECT

        // CAN'T BE THE LAST OBJECT IN THE UNIT
        if(ptr>=objend) { RetNum=ERR_INVALID; return; }

        while(ptr<objend) {

            if(!ISIDENT(*ptr)) { RetNum=ERR_INVALID; return; }
            ptr=rplSkipOb(ptr);
            if(ptr>=objend) { RetNum=ERR_INVALID; return; }
            if(!ISNUMBER(*ptr)) { RetNum=ERR_INVALID; return; }
            ptr=rplSkipOb(ptr);
            if(ptr>=objend) { RetNum=ERR_INVALID; return; }
            if(!ISNUMBER(*ptr)) { RetNum=ERR_INVALID; return; }
            ptr=rplSkipOb(ptr);
        }

        if(ptr!=objend) { RetNum=ERR_INVALID; return; }

        }

        RetNum=OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;


    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(MENUNUMBER(MenuCodeArg)>17) {
            RetNum=ERR_NOTMINE;
            return;
        }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR=ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)+3];
        RetNum=OK_CONTINUE;
        return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp,(WORDPTR)LIB_HELPTABLE);
       return;
    }
    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError,(WORDPTR)LIB_MSGTABLE);
       return;
    }


    case OPCODE_LIBINSTALL:
        LibraryList=(WORDPTR)libnumberlist;
        RetNum=OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;

    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode)>=MIN_RESERVED_OPCODE) {
        RetNum=ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    rplError(ERR_INVALIDOPCODE);

    return;


}



#endif
