/*
 * Copyright (c) 2016, Claudio Lapilli and the newRPL Team
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
#define LIBRARY_NUMBER  48

//@TITLE=Operations with Angles

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(TAGDEG,"→∡°",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TAGRAD,"→∡r",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TAGGRAD,"→∡g",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TAGDMS,"→∡d",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ANGTODEG,"A→∡°",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ANGTORAD,"A→∡r",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ANGTOGRAD,"A→∡g",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ANGTODMS,"A→∡d",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TORECT,"→RECT",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOPOLAR,"→POLAR",MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOSPHER,"→SPHER",MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2))




#define ERROR_LIST \
        ERR(REALORANGLEEXPECTED,0)



// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER,LIBRARY_NUMBER+1,LIBRARY_NUMBER+2,LIBRARY_NUMBER+3


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib48_menu);

// OTHER ROMOBJECTS
ROMOBJECT angle_0[]={
    MKPROLOG(DOANGLE | ANGLEDEG,1),
    MAKESINT(0)
};


ROMOBJECT angle_90[]={
    MKPROLOG(DOANGLE | ANGLEDEG,1),
    MAKESINT(90)
};

ROMOBJECT angle_180[]={
    MKPROLOG(DOANGLE | ANGLEDEG,1),
    MAKESINT(180)
};

ROMOBJECT angle_270[]={
    MKPROLOG(DOANGLE | ANGLEDEG,1),
    MAKESINT(-90)
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
     (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib48_menu,
     (WORDPTR)angle_180,
    (WORDPTR)angle_0,
    (WORDPTR)angle_90,
    (WORDPTR)angle_270,

    0
};





// HIGHER LEVEL API USING ANGLE OBJECTS DIRECTLY
void rplConvertAngleObj(WORDPTR angleobj,BINT newmode)
{
    BINT oldmode;
    REAL oldang;

    if(ISANGLE(*angleobj)) { oldmode=ANGLEMODE(*angleobj); ++angleobj; }
    else oldmode=ANGLERAD;
    rplReadNumberAsReal(angleobj,&oldang);

    trig_convertangle(&oldang,oldmode,newmode);
}



WORDPTR rplNewAngleFromNumber(WORDPTR numobj,BINT newmode)
{
    if(!ISNUMBER(*numobj)) {
        rplError(ERR_REALEXPECTED);
        return 0;
    }
    ScratchPointer1=numobj;
    BINT size=rplObjSize(numobj);

    WORDPTR newobj=rplAllocTempOb(size);

    if(!newobj) return 0;

    *newobj=MKPROLOG(DOANGLE + (newmode&3), size);
    memcpyw(newobj+1,ScratchPointer1,size);

    return newobj;

}


WORDPTR rplNewAngleFromReal(REAL *number,BINT newmode)
{
    BINT size=number->len+2;

    WORDPTR newobj=rplAllocTempOb(size);

    if(!newobj) return 0;

    WORDPTR endofobject=rplNewRealInPlace(number,newobj+1);

    *newobj=MKPROLOG(DOANGLE + (newmode&3), endofobject-newobj-1);

    rplTruncateLastObject(endofobject);

    return newobj;

}









void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // NORMAL BEHAVIOR FOR A REAL IS TO PUSH THE OBJECT ON THE STACK:
        rplPushData(IPtr);
        return;
    }


    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE)
    {
        // THESE ARE OVERLOADABLE COMMANDS DISPATCHED FROM THE
        // OVERLOADABLE OPERATORS LIBRARY.

        // PROVIDE BEHAVIOR FOR OVERLOADABLE OPERATORS HERE
        // USE GC-SAFE POINTERS, NEVER LOCAL COPIES OF POINTERS INTO TEMPOB
#define arg1 ScratchPointer1
#define arg2 ScratchPointer2

        int nargs=OVR_GETNARGS(CurOpcode);

        if(rplDepthData()<nargs) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(nargs==1) {

            if(!ISPROLOG(*rplPeekData(1))) {
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
            case OVR_INV:
            {
                // ARGUMENT CHECKS SHOULD NOT BE NECESSARY

                // CONVERT TO CURRENT SYSTEM AND REMOVE THE TAG
                BINT curmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

                rplConvertAngleObj(rplPeekData(1),curmode);
                // NEW ANGLE IS IN RReg[0]
                WORDPTR number=rplNewReal(&RReg[0]);
                if(Exceptions) return;

                // THEN CALL THE ORIGINAL OPERATOR
                rplOverwriteData(1,number);
                rplCallOvrOperator(CurOpcode);
                return;

            }

            case OVR_ABS:
            case OVR_NEG:
            {
                // OPERATE, KEEP THE TAG
                rplPushData(rplPeekData(1)+1);  // EXTRACT THE NUMBER
                rplCallOvrOperator(CurOpcode);  // DO THE THING ON THE NUMBER

                WORDPTR newangle=rplNewAngleFromNumber(rplPeekData(1),ANGLEMODE(*rplPeekData(2)));
                rplDropData(1);
                if(Exceptions) return;

                rplOverwriteData(1,newangle);
                return;
            }
            case OVR_EVAL1:
            case OVR_EVAL:
            case OVR_XEQ:
            case OVR_FUNCEVAL:
                // DO NOTHING!
                return;
            case OVR_NUM:
            {
                // ARGUMENT CHECKS SHOULD NOT BE NECESSARY

                // CONVERT TO CURRENT SYSTEM AND REMOVE THE TAG
                BINT curmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

                rplConvertAngleObj(rplPeekData(1),curmode);
                // NEW ANGLE IS IN RReg[0]
                WORDPTR number=rplNewReal(&RReg[0]);
                if(Exceptions) return;

                rplOverwriteData(1,number);
                return;

            }
                // CHANGE SIGN, KEEP THE TAG

            case OVR_ISTRUE:
            case OVR_NOT:
                // REMOVE THE TAG, CONVERT TO 1 OR 0
            {
                // ARGUMENT CHECKS SHOULD NOT BE NECESSARY
                // REMOVE THE TAG, NO NEED TO CONVERT
                rplOverwriteData(1,rplPeekData(1)+1);

                // THEN CALL THE ORIGINAL OPERATOR
                rplCallOvrOperator(CurOpcode);
                return;

            }

            }

        }
        else {
            // BINARY OPERATORS

            arg1=rplPeekData(2);
            arg2=rplPeekData(1);

            if(!(ISANGLE(*arg1)||ISNUMBER(*arg1)) || !(ISANGLE(*arg2)||ISNUMBER(*arg2))) {
                // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
                    if((OPCODE(CurOpcode)==OVR_SAME) && (!ISPROLOG(*arg1)|| !ISPROLOG(*arg2))) {
                        if(*rplPeekData(2)==*rplPeekData(1)) {
                            rplDropData(2);
                            rplPushTrue();
                        } else {
                            rplDropData(2);
                            rplPushFalse();
                        }
                        return;

                    }

                    // AT LEAST ONE OBJECT IS NOT A NUMBER
                    switch(OPCODE(CurOpcode))
                    {
                    case OVR_SAME:
                    case OVR_EQ:
                        rplDropData(2);
                        rplPushFalse();
                        break;
                    case OVR_NOTEQ:
                        rplDropData(2);
                        rplPushTrue();
                        break;
                    default:
                        rplError(ERR_REALORANGLEEXPECTED);
                    }
                    return;

            }

            if(ISANGLE(*arg1) && ISANGLE(*arg2)) {
                // ONLY ALLOW ADDITION/SUBTRACTION OF ANGLES

                if(OPCODE(CurOpcode)==OVR_ADD) {

                    // CONVERT 2ND ARGUMENT TO THE SYSTEM OF THE FIRST

                    BINT angmode=ANGLEMODE(*arg1);
                    if(angmode!=ANGLEDMS) {
                        REAL arg1num;

                        rplReadNumberAsReal(arg1+1,&arg1num);

                        rplConvertAngleObj(arg2,angmode);
                        addReal(&RReg[1],&arg1num,&RReg[0]);
                        WORDPTR newang=rplNewAngleFromReal(&RReg[1],angmode);

                        if(Exceptions) return;
                        rplOverwriteData(2,newang);
                        rplDropData(1);
                        return;

                    }
                        // ADDING NUMBERS IN DMS FORMAT DIRECTLY TAKES MORE WORK

                        rplConvertAngleObj(arg2,ANGLEDEG);
                        copyReal(&RReg[7],&RReg[0]);
                        rplConvertAngleObj(arg1,ANGLEDEG);

                        addReal(&RReg[6],&RReg[0],&RReg[7]);
                        trig_convertangle(&RReg[6],ANGLEDEG,ANGLEDMS);

                        WORDPTR newang=rplNewAngleFromReal(&RReg[0],angmode);

                        if(Exceptions) return;
                        rplOverwriteData(2,newang);
                        rplDropData(1);
                        return;

                }

                if(OPCODE(CurOpcode)==OVR_SUB) {

                    // CONVERT 2ND ARGUMENT TO THE SYSTEM OF THE FIRST

                    BINT angmode=ANGLEMODE(*arg1);


                    if(angmode!=ANGLEDMS) {
                        REAL arg1num;
                        rplReadNumberAsReal(arg1+1,&arg1num);
                        rplConvertAngleObj(arg2,angmode);
                        subReal(&RReg[1],&arg1num,&RReg[0]);
                        WORDPTR newang=rplNewAngleFromReal(&RReg[1],angmode);

                        if(Exceptions) return;
                        rplOverwriteData(2,newang);
                        rplDropData(1);
                        return;

                    }
                        // ADDING NUMBERS IN DMS FORMAT DIRECTLY TAKES MORE WORK

                        rplConvertAngleObj(arg2,ANGLEDEG);
                        copyReal(&RReg[7],&RReg[0]);
                        rplConvertAngleObj(arg1,ANGLEDEG);

                        subReal(&RReg[6],&RReg[0],&RReg[7]);
                        trig_convertangle(&RReg[6],ANGLEDEG,ANGLEDMS);

                        WORDPTR newang=rplNewAngleFromReal(&RReg[0],angmode);

                        if(Exceptions) return;
                        rplOverwriteData(2,newang);
                        rplDropData(1);
                        return;

                }

            }
                // ALL OTHER OPERATORS SHOULD CONVERT TO CURRENT ANGLE SYSTEM AND REMOVE TAGS
                // THEN PROCESS THE OPCODE NORMALLY.

                BINT curmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

                if(ISANGLE(*arg1)) {
                    // CONVERT TO CURRENT SYSTEM AND REMOVE THE TAG

                    rplConvertAngleObj(arg1,curmode);
                    // NEW ANGLE IS IN RReg[0]
                    WORDPTR number=rplNewReal(&RReg[0]);
                    if(Exceptions) return;

                    rplOverwriteData(2,number);
                }

                if(ISANGLE(*arg2)) {
                    // CONVERT TO CURRENT SYSTEM AND REMOVE THE TAG

                    rplConvertAngleObj(arg2,curmode);
                    // NEW ANGLE IS IN RReg[0]
                    WORDPTR number=rplNewReal(&RReg[0]);
                    if(Exceptions) return;

                    rplOverwriteData(1,number);
                }

                rplCallOvrOperator(CurOpcode);
                return;




        }

#undef arg1
#undef arg2
    }   // END OF OVERLOADABLE OPERATORS


    switch(OPCODE(CurOpcode))
    {

    case TAGDEG:
    {
        //@SHORT_DESC=Mark a number as an angle in degrees
        //@NEW
    // TAG A NUMBER
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {


            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }





        if(ISNUMBER(*rplPeekData(1))) {
            WORDPTR newang=rplNewAngleFromNumber(rplPeekData(1),ANGLEDEG);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        rplError(ERR_REALEXPECTED);
        return;

    }
    case TAGRAD:
    {
        //@SHORT_DESC=Mark a number as an angle in radians
        //@NEW
    // TAG A NUMBER
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {


            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }



        if(ISNUMBER(*rplPeekData(1))) {
            WORDPTR newang=rplNewAngleFromNumber(rplPeekData(1),ANGLERAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        rplError(ERR_REALEXPECTED);
        return;

    }

    case TAGGRAD:
    {
        //@SHORT_DESC=Mark a number as an angle in grads (gons)
        //@NEW
    // TAG A NUMBER
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;


            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }



        if(ISNUMBER(*rplPeekData(1))) {
            WORDPTR newang=rplNewAngleFromNumber(rplPeekData(1),ANGLEGRAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        rplError(ERR_REALEXPECTED);
        return;

    }

    case TAGDMS:
    {
        //@SHORT_DESC=Mark a number as an angle in DMS (DD.MMSS)
        //@NEW
    // TAG A NUMBER
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }



        if(ISNUMBER(*rplPeekData(1))) {
            WORDPTR newang=rplNewAngleFromNumber(rplPeekData(1),ANGLEDMS);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        rplError(ERR_REALEXPECTED);
        return;

    }

    case ANGTODEG:
    {
        //@SHORT_DESC=Convert an angle to degrees
        //@NEW
        // CONVERT ANGLE TO DEGREES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }






        if(ISNUMBER(*rplPeekData(1))) {
            REAL num;
            rplReadNumberAsReal(rplPeekData(1),&num);
            BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);
            trig_convertangle(&num,angmode,ANGLEDEG);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEDEG);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        if(ISANGLE(*rplPeekData(1))) {
            rplConvertAngleObj(rplPeekData(1),ANGLEDEG);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEDEG);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }


        if(ISCOMPLEX(*rplPeekData(1))) {
            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode==ANGLENONE) return;   // NOTHING TO DO
            if(angmode==ANGLEDEG) return; // ALREADY IN THE RIGHT SYSTEM

            REAL rp,ip;
            rplRealPart(rplPeekData(1),&rp);
            rplImaginaryPart(rplPeekData(1),&ip);

            trig_convertangle(&ip,angmode,ANGLEDEG);

            rplDropData(1);
            rplNewComplexPush(&rp,&RReg[0],ANGLEDEG);

            return;

       }

        if(ISMATRIX(*rplPeekData(1))) {

            // CONVERT ALL ANGLES WITHIN A VECTOR

            WORDPTR matrix=rplPeekData(1);
            BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            // EXPLODE ALL NUMBERS IN THE STACK

            WORDPTR *first=rplMatrixExplode();
            BINT f;

            for(f=0;f<cols;++f)
            {
                // CONVERT ANY ANGLES TO THE PROPER SYSTEM
                if(ISANGLE(*first[f])) {
                    rplConvertAngleObj(first[f],ANGLEDEG);

                    WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEDEG);
                    if(!newang) { DSTop=first; return; }
                    first[f]=newang;    // REPLACE IT WITH THE NEW ANGLE
                    continue;
                }
                if(ISCOMPLEX(*first[f])) {
                    BINT angmode=rplPolarComplexMode(first[f]);

                    if(angmode==ANGLENONE) continue;

                    if(angmode==ANGLEDEG) continue; // ALREADY IN THE RIGHT SYSTEM

                    REAL rp,ip;
                    rplRealPart(first[f],&rp);
                    rplImaginaryPart(first[f],&ip);

                    trig_convertangle(&ip,angmode,ANGLEDEG);

                    WORDPTR newobj=rplNewComplex(&rp,&RReg[0],ANGLEDEG);

                    if(!newobj) { DSTop=first; return; }
                    first[f]=newobj;    // REPLACE IT WITH THE NEW COMPLEX
                    continue;

                }
            }

            // NOW RECREATE THE MATRIX

            WORDPTR newmat=rplMatrixCompose(rows,cols);
            if(!newmat) { DSTop=first; return; }

            DSTop=first;    // CLEAN UP THE STACK

            rplOverwriteData(1,newmat);

            return;

        }



        return;


    }

    case ANGTORAD:
    {
        //@SHORT_DESC=Convert an angle to radians
        //@NEW
        // CONVERT ANGLE TO DEGREES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }




        if(ISNUMBER(*rplPeekData(1))) {
            REAL num;
            rplReadNumberAsReal(rplPeekData(1),&num);
            BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);
            trig_convertangle(&num,angmode,ANGLERAD);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLERAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        if(ISANGLE(*rplPeekData(1))) {
            rplConvertAngleObj(rplPeekData(1),ANGLERAD);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLERAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }


        if(ISCOMPLEX(*rplPeekData(1))) {
            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode==ANGLENONE) return;   // NOTHING TO DO
            if(angmode==ANGLERAD) return; // ALREADY IN THE RIGHT SYSTEM

            REAL rp,ip;
            rplRealPart(rplPeekData(1),&rp);
            rplImaginaryPart(rplPeekData(1),&ip);

            trig_convertangle(&ip,angmode,ANGLERAD);

            rplDropData(1);
            rplNewComplexPush(&rp,&RReg[0],ANGLERAD);

            return;

       }


        if(ISMATRIX(*rplPeekData(1))) {

            // CONVERT ALL ANGLES WITHIN A VECTOR

            WORDPTR matrix=rplPeekData(1);
            BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            // EXPLODE ALL NUMBERS IN THE STACK

            WORDPTR *first=rplMatrixExplode();
            BINT f;

            for(f=0;f<cols;++f)
            {
                // CONVERT ANY ANGLES TO THE PROPER SYSTEM
                if(ISANGLE(*first[f])) {
                    rplConvertAngleObj(first[f],ANGLERAD);

                    WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLERAD);
                    if(!newang) { DSTop=first; return; }
                    first[f]=newang;    // REPLACE IT WITH THE NEW ANGLE
                    continue;
                }
                if(ISCOMPLEX(*first[f])) {
                    BINT angmode=rplPolarComplexMode(first[f]);

                    if(angmode==ANGLENONE) continue;

                    if(angmode==ANGLERAD) continue; // ALREADY IN THE RIGHT SYSTEM

                    REAL rp,ip;
                    rplRealPart(first[f],&rp);
                    rplImaginaryPart(first[f],&ip);

                    trig_convertangle(&ip,angmode,ANGLERAD);

                    WORDPTR newobj=rplNewComplex(&rp,&RReg[0],ANGLERAD);

                    if(!newobj) { DSTop=first; return; }
                    first[f]=newobj;    // REPLACE IT WITH THE NEW COMPLEX
                    continue;

                }
            }

            // NOW RECREATE THE MATRIX

            WORDPTR newmat=rplMatrixCompose(rows,cols);
            if(!newmat) { DSTop=first; return; }

            DSTop=first;    // CLEAN UP THE STACK

            rplOverwriteData(1,newmat);

            return;

        }



        return;


    }

    case ANGTOGRAD:
    {
        //@SHORT_DESC=Convert an angle to grads (gons)
        //@NEW
        // CONVERT ANGLE TO DEGREES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;


            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }




        if(ISNUMBER(*rplPeekData(1))) {
            REAL num;
            rplReadNumberAsReal(rplPeekData(1),&num);
            BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);
            trig_convertangle(&num,angmode,ANGLEGRAD);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEGRAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        if(ISANGLE(*rplPeekData(1))) {
            rplConvertAngleObj(rplPeekData(1),ANGLEGRAD);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEGRAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }


        if(ISCOMPLEX(*rplPeekData(1))) {
            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode==ANGLENONE) return;   // NOTHING TO DO
            if(angmode==ANGLEGRAD) return; // ALREADY IN THE RIGHT SYSTEM

            REAL rp,ip;
            rplRealPart(rplPeekData(1),&rp);
            rplImaginaryPart(rplPeekData(1),&ip);

            trig_convertangle(&ip,angmode,ANGLEGRAD);

            rplDropData(1);
            rplNewComplexPush(&rp,&RReg[0],ANGLEGRAD);

            return;

       }

        if(ISMATRIX(*rplPeekData(1))) {

            // CONVERT ALL ANGLES WITHIN A VECTOR

            WORDPTR matrix=rplPeekData(1);
            BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            // EXPLODE ALL NUMBERS IN THE STACK

            WORDPTR *first=rplMatrixExplode();
            BINT f;

            for(f=0;f<cols;++f)
            {
                // CONVERT ANY ANGLES TO THE PROPER SYSTEM
                if(ISANGLE(*first[f])) {
                    rplConvertAngleObj(first[f],ANGLEGRAD);

                    WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEGRAD);
                    if(!newang) { DSTop=first; return; }
                    first[f]=newang;    // REPLACE IT WITH THE NEW ANGLE
                    continue;
                }
                if(ISCOMPLEX(*first[f])) {
                    BINT angmode=rplPolarComplexMode(first[f]);

                    if(angmode==ANGLENONE) continue;

                    if(angmode==ANGLEGRAD) continue; // ALREADY IN THE RIGHT SYSTEM

                    REAL rp,ip;
                    rplRealPart(first[f],&rp);
                    rplImaginaryPart(first[f],&ip);

                    trig_convertangle(&ip,angmode,ANGLEGRAD);

                    WORDPTR newobj=rplNewComplex(&rp,&RReg[0],ANGLEGRAD);

                    if(!newobj) { DSTop=first; return; }
                    first[f]=newobj;    // REPLACE IT WITH THE NEW COMPLEX
                    continue;

                }
            }

            // NOW RECREATE THE MATRIX

            WORDPTR newmat=rplMatrixCompose(rows,cols);
            if(!newmat) { DSTop=first; return; }

            DSTop=first;    // CLEAN UP THE STACK

            rplOverwriteData(1,newmat);

            return;

        }



        return;


    }

    case ANGTODMS:
    {
        //@SHORT_DESC=Convert an angle to DMS (DD.MMSS)
        //@NEW
        // CONVERT ANGLE TO DEGREES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }




        if(ISNUMBER(*rplPeekData(1))) {
            REAL num;
            rplReadNumberAsReal(rplPeekData(1),&num);
            BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);
            trig_convertangle(&num,angmode,ANGLEDMS);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEDMS);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        if(ISANGLE(*rplPeekData(1))) {
            rplConvertAngleObj(rplPeekData(1),ANGLEDMS);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEDMS);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }


        if(ISCOMPLEX(*rplPeekData(1))) {
            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode==ANGLENONE) return;   // NOTHING TO DO
            if(angmode==ANGLEDMS) return; // ALREADY IN THE RIGHT SYSTEM

            REAL rp,ip;
            rplRealPart(rplPeekData(1),&rp);
            rplImaginaryPart(rplPeekData(1),&ip);

            trig_convertangle(&ip,angmode,ANGLEDMS);

            rplDropData(1);
            rplNewComplexPush(&rp,&RReg[0],ANGLEDMS);

            return;

       }

        if(ISMATRIX(*rplPeekData(1))) {

            // CONVERT ALL ANGLES WITHIN A VECTOR

            WORDPTR matrix=rplPeekData(1);
            BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            // EXPLODE ALL NUMBERS IN THE STACK

            WORDPTR *first=rplMatrixExplode();
            BINT f;

            for(f=0;f<cols;++f)
            {
                // CONVERT ANY ANGLES TO THE PROPER SYSTEM
                if(ISANGLE(*first[f])) {
                    rplConvertAngleObj(first[f],ANGLEDMS);

                    WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEDMS);
                    if(!newang) { DSTop=first; return; }
                    first[f]=newang;    // REPLACE IT WITH THE NEW ANGLE
                    continue;
                }
                if(ISCOMPLEX(*first[f])) {
                    BINT angmode=rplPolarComplexMode(first[f]);

                    if(angmode==ANGLENONE) continue;

                    if(angmode==ANGLEDMS) continue; // ALREADY IN THE RIGHT SYSTEM

                    REAL rp,ip;
                    rplRealPart(first[f],&rp);
                    rplImaginaryPart(first[f],&ip);

                    trig_convertangle(&ip,angmode,ANGLEDMS);

                    WORDPTR newobj=rplNewComplex(&rp,&RReg[0],ANGLEDMS);

                    if(!newobj) { DSTop=first; return; }
                    first[f]=newobj;    // REPLACE IT WITH THE NEW COMPLEX
                    continue;

                }
            }

            // NOW RECREATE THE MATRIX

            WORDPTR newmat=rplMatrixCompose(rows,cols);
            if(!newmat) { DSTop=first; return; }

            DSTop=first;    // CLEAN UP THE STACK

            rplOverwriteData(1,newmat);

            return;

        }



        return;


    }

    case TORECT:
        // CONVERT EITHER A COMPLEX OR A VECTOR TO CARTESIAN COORDINATES
    {
        //@SHORT_DESC=Convert vector or complex to cartesian coordinates
        //@NEW
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISCOMPLEX(*rplPeekData(1))) {
            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode==ANGLENONE) return;   // NOTHING TO DO

            REAL rp,ip;
            rplReadCNumber(rplPeekData(1),&rp,&ip,&angmode);
            rplPolar2Rect(&rp,&ip,angmode);
            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[1]);

            if(Exceptions) return;

            rplDropData(1);
            rplNewComplexPush(&RReg[0],&RReg[1],ANGLENONE);
            return;
       }

        if(ISMATRIX(*rplPeekData(1))) {

           // CONVERT TO RECTANGULAR

            WORDPTR matrix=rplPeekData(1);
            BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            if(!rplMatrixIsPolar(matrix)) return; // NOTHING TO DO


            // EXPLODE ALL NUMBERS IN THE STACK

            WORDPTR *first=rplMatrixExplode();

            rplMatrixPolarToRectEx(first-1,1,cols);
            if(Exceptions) { DSTop=first; return; }

            // NOW RECREATE THE MATRIX

            WORDPTR newmat=rplMatrixCompose(rows,cols);
            if(!newmat) { DSTop=first; return; }

            DSTop=first;    // CLEAN UP THE STACK

            rplOverwriteData(1,newmat);

            return;

        }


        rplError(ERR_BADARGTYPE);
        return;


    }

    case TOPOLAR:

        // CONVERT EITHER A COMPLEX OR A VECTOR TO POLAR COORDINATES
    {
        //@SHORT_DESC=Convert vector or complex to polar coordinates
        //@NEW
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISCOMPLEX(*rplPeekData(1))) {
            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode!=ANGLENONE) return;   // NOTHING TO DO

            REAL rp,ip;
            rplReadCNumber(rplPeekData(1),&rp,&ip,&angmode);

            angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

            rplRect2Polar(&rp,&ip,angmode);
            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[1]);

            if(Exceptions) return;

            rplDropData(1);
            rplNewComplexPush(&RReg[0],&RReg[1],angmode);
            return;
       }

        if(ISMATRIX(*rplPeekData(1))) {

           // CONVERT TO POLAR (CYLINDRICAL)

            WORDPTR matrix=rplPeekData(1);
            BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            if(rplMatrixIsPolar(matrix)) return; // NOTHING TO DO

            BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

            // EXPLODE ALL NUMBERS IN THE STACK

            WORDPTR *first=rplMatrixExplode();

            rplMatrixRectToPolarEx(first-1,1,cols,1,angmode);
            if(Exceptions) { DSTop=first; return; }

            // NOW RECREATE THE MATRIX

            WORDPTR newmat=rplMatrixCompose(rows,cols);
            if(!newmat) { DSTop=first; return; }

            DSTop=first;    // CLEAN UP THE STACK

            rplOverwriteData(1,newmat);

            return;

        }


        rplError(ERR_BADARGTYPE);
        return;


    }



    case TOSPHER:

        // CONVERT EITHER A COMPLEX OR A VECTOR TO POLAR COORDINATES
    {
        //@SHORT_DESC=Convert vector or complex to spherical coordinates
        //@NEW
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISMATRIX(*rplPeekData(1))) {

           // CONVERT TO SPHERICAL COORDINATES

            WORDPTR matrix=rplPeekData(1);
            BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            if(rplMatrixIsPolar(matrix)) return; // NOTHING TO DO

            BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

            // EXPLODE ALL NUMBERS IN THE STACK

            WORDPTR *first=rplMatrixExplode();

            rplMatrixRectToPolarEx(first-1,1,cols,0xffffffff,angmode);
            if(Exceptions) { DSTop=first; return; }

            // NOW RECREATE THE MATRIX

            WORDPTR newmat=rplMatrixCompose(rows,cols);
            if(!newmat) { DSTop=first; return; }

            DSTop=first;    // CLEAN UP THE STACK

            rplOverwriteData(1,newmat);

            return;

        }


        rplError(ERR_BADARGTYPE);
        return;


    }


    return;





    // STANDARIZED OPCODES:
    // --------------------
    // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES



    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlankLen = blanks length

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors



        // COMPILE A NUMBER TO A REAL
    {
        if(LIBNUM(CurOpcode)!=DOANGLE) {
            // DO NOT COMPILE ANYTHING WHEN CALLED WITH THE UPPER LIBRARY NUMBERS
            RetNum=ERR_NOTMINE;
            return;
        }

        if(!utf8ncmp2((char *)TokenStart,(char *)BlankStart,"∡",1)) {


        // POINT TO THE LAST CHARACTER
        BINT tlen=TokenLen-1;
        BYTEPTR ptr=(BYTEPTR)utf8nskip((char *)TokenStart,(char *)BlankStart,tlen);
        BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);
        BYTEPTR strptr=(BYTEPTR)utf8skip((char *)TokenStart,(char *)BlankStart);
        BINT isapprox=0;


            if(!utf8ncmp2((char *)ptr,(char *)BlankStart,"°",1)) { angmode=ANGLEDEG; --tlen; }
            if((*ptr=='d')||(*ptr=='D')) { angmode=ANGLEDMS; --tlen; }
            if((*ptr=='g')||(*ptr=='G')) { angmode=ANGLEGRAD; --tlen; }
            if((*ptr=='r')||(*ptr=='R')) { angmode=ANGLERAD; --tlen; }


            UBINT64 locale=rplGetSystemLocale();


            newRealFromText(&RReg[0],(char *)strptr,utf8nskip((char *)strptr,(char *)BlankStart,tlen),locale);


            if(RReg[0].flags&F_ERROR) {
                // THERE WAS SOME ERROR DURING THE CONVERSION, PROBABLY A SYNTAX ERROR
                RetNum=ERR_NOTMINE;
                return;
            }

                if(RReg[0].flags&F_OVERFLOW) {
                    rplError(ERR_MATHOVERFLOW);
                    RetNum=ERR_INVALID;
                    return;
                }
                if(RReg[0].flags&(F_NEGUNDERFLOW|F_POSUNDERFLOW)) {
                    rplError(ERR_MATHUNDERFLOW);
                    RetNum=ERR_INVALID;
                    return;
                }


                if(RReg[0].flags&F_APPROX) isapprox=APPROX_BIT;

                ScratchPointer1=CompileEnd;

                // WRITE THE PROLOG

                rplCompileAppend(MKPROLOG(LIBRARY_NUMBER|angmode,0));

                if(isintegerReal(&RReg[0]) && inBINT64Range(&RReg[0])) {
                        BINT64 num=getBINT64Real(&RReg[0]);
                        rplCompileBINT(num,DECBINT|isapprox);
                 }

                 else {
                rplCompileAppend(MKPROLOG(DOREAL|isapprox,1+RReg[0].len));
                // PACK THE INFORMATION
                REAL_HEADER real;
                real.flags=RReg[0].flags&0xf;
                real.len=RReg[0].len;
                real.exp=RReg[0].exp;

                rplCompileAppend(real.word);      // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
                BINT count;
                for(count=0;count<RReg[0].len;++count) {
                    rplCompileAppend(RReg[0].data[count]);      // STORE ALL THE MANTISSA WORDS
                }
                }

                *ScratchPointer1|=CompileEnd-ScratchPointer1-1; // FIX THE SIZE


                RetNum=OK_CONTINUE;
            return;

    }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;
    }

    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Byte Ptr to end of current string. Write here with rplDecompAppendString(); rplDecompAppendChar();

        if(ISPROLOG(*DecompileObject)) {
        rplDecompAppendString((BYTEPTR)"∡");


        // THIS IS A BASE-10 NUMBER
        // CONVERT TO STRING

        REAL realnum;

        NUMFORMAT fmt;

        BINT Format,sign;

        rplGetSystemNumberFormat(&fmt);


        rplReadNumberAsReal(DecompileObject+1,&realnum);

        sign=realnum.flags&F_NEGATIVE;

        realnum.flags^=sign;

        if(iszeroReal(&realnum)) Format=fmt.MiddleFmt;
        else if(ltReal(&realnum,&(fmt.SmallLimit))) Format=fmt.SmallFmt;
        else if(gtReal(&realnum,&(fmt.BigLimit))) Format=fmt.BigFmt;
        else Format=fmt.MiddleFmt;

        realnum.flags^=sign;

        if(CurOpcode==OPCODE_DECOMPEDIT) Format|=FMT_CODE;

        // ESTIMATE THE MAXIMUM STRING LENGTH AND RESERVE THE MEMORY

        BYTEPTR string;

        BINT len=formatlengthReal(&realnum,Format,fmt.Locale);

        // realnum DATA MIGHT MOVE DUE TO GC, NEEDS TO BE PROTECTED
        ScratchPointer1=(WORDPTR)realnum.data;
        ScratchPointer2=(WORDPTR)fmt.SmallLimit.data;
        ScratchPointer3=(WORDPTR)fmt.BigLimit.data;


        // RESERVE THE MEMORY FIRST
        rplDecompAppendString2(0,len);

        realnum.data=(BINT *)ScratchPointer1;
        fmt.SmallLimit.data=(BINT *)ScratchPointer2;
        fmt.BigLimit.data=(BINT *)ScratchPointer3;


        // NOW USE IT
        string=(BYTEPTR)DecompStringEnd;
        string-=len;

        if(Exceptions) {
            RetNum=ERR_INVALID;
            return;
        }
        DecompStringEnd=(WORDPTR) formatReal(&realnum,(char *)string,Format,fmt.Locale);



        switch(ANGLEMODE(*DecompileObject))
        {
        case ANGLERAD:
            rplDecompAppendChar('r');
            break;
        case ANGLEGRAD:
            rplDecompAppendChar('g');
            break;
        case ANGLEDEG:
            rplDecompAppendString((BYTEPTR)"°");
            break;
        case ANGLEDMS:
            rplDecompAppendChar('d');
            break;
        }

        RetNum=OK_CONTINUE;
        return;
        }


        // STANDARD COMMAND HANDLING
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

    {
        RetNum=ERR_INVALID;
        return;
    }

    case OPCODE_PROBETOKEN:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // ArgPtr2 = token blanks afterwards
        // ArgNum2 = blanks length

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...), or ERR_NOTMINE IF NO TOKEN IS FOUND
    {

        if(LIBNUM(CurOpcode)!=LIBRARY_NUMBER) {
            // DO NOT COMPILE ANYTHING WHEN CALLED WITH THE UPPER (APPROX) LIBRARY NUMBER
            RetNum=ERR_NOTMINE;
            return;
        }
        enum {
            MODE_IP=0,
            MODE_FP,
            MODE_EXPLETTER,
            MODE_EXPSIGN,
            MODE_EXP
        };
        NUMFORMAT nformat;
        rplGetSystemNumberFormat(&nformat);
        BINT mode=MODE_IP;
        WORD num;
        int f,exitfor=0;
        BYTEPTR ptr=(BYTEPTR)TokenStart;

        for(f=0;f<(int)TokenLen;++f,ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)ptr+4)) {
            num=utf82cp((char *)ptr,(char *)ptr+4);
            switch(mode)
            {
            case MODE_IP:
                if(f==0) {
                    if(num==0x2221) break;
                    exitfor=1;
                    break;
                }
                if(num==DECIMAL_DOT(nformat.Locale)) { mode=MODE_FP; break; }
                if(num==THOUSAND_SEP(nformat.Locale)) { break; }
                if((f!=0) && (num=='e' || num=='E' || num==EXP_LETTER(nformat.MiddleFmt))) { mode=MODE_EXPSIGN; break; }
                if(num<'0' || num>'9') { exitfor=1; break; }
                break;
            case MODE_FP:
                if(num==FRAC_SEP(nformat.Locale)) { break; }
                if(num=='.') { mode=MODE_EXPLETTER; break; }
                if(num=='e' || num=='E' || num==EXP_LETTER(nformat.MiddleFmt)) { mode=MODE_EXPSIGN; break; }
                if(num<'0' || num>'9') { exitfor=1; break; }
                break;
            case MODE_EXPLETTER:
                if(num=='e' || num=='E' || num==EXP_LETTER(nformat.MiddleFmt)) { mode=MODE_EXPSIGN; break; }
                exitfor=1;
                break;
            case MODE_EXPSIGN:
                if(num=='+' || num=='-') { mode=MODE_EXP; break; }
                if(num<'0' || num>'9') { exitfor=1; break; }
                mode=MODE_EXP;
                break;
            case MODE_EXP:
                if(num<'0' || num>'9') { exitfor=1; break; }
                break;
            }
            if(exitfor) break;
        }



        if(mode==MODE_EXPSIGN) --f;
        if(f==0) RetNum=ERR_NOTMINE;

        else {
            if(f<(int)TokenLen) {
                if(num=='.') { ++f; ++ptr; num=*ptr; }
                if(f<(int)TokenLen) {
                    if(!utf8ncmp2((char *)ptr,(char *)BlankStart,"°",1)) ++f;
                    else
                        if((num=='d')||(num=='D')||(num=='g')||(num=='G')||(num=='r')||(num=='R')) ++f;
                }
            }

            RetNum=OK_TOKENINFO | MKTOKENINFO(f,TITYPE_REAL,0,1);
        }

        return;
    }

    case OPCODE_GETINFO:

        // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
        // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
        // IN RetNum: RETURN THE MKTOKENINFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
        // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
        // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
        //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
        //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
        //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
        // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL BINT, .42 = HEX INTEGER

        if(ISPROLOG(*ObjectPTR)) {
        TypeInfo=LIBRARY_NUMBER*100;
        DecompHints=0;
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_REAL,0,1);
        }
        else {
            TypeInfo=0;     // ALL COMMANDS ARE TYPE 0
            DecompHints=0;
            libGetInfo2(*ObjectPTR,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        }
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID, ObjectIDHash=hash
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID,ObjectIDHash);
        return;


    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID

        if(ISPROLOG(*ObjectPTR)) {
            if(OBJSIZE(*ObjectPTR)<2) { RetNum=ERR_INVALID; return; }
            if(rplSkipOb(ObjectPTR)!=rplSkipOb(ObjectPTR+1)) { RetNum=ERR_INVALID; return; }
            if(!ISNUMBER(ObjectPTR[1])) { RetNum=ERR_INVALID; return; }
            if(ISREAL(ObjectPTR[1])) {
            REAL r;
            rplReadReal(ObjectPTR+1,&r);
            // CHECK PROPER LENGTH
            if((WORD)(r.len+1)!=OBJSIZE(*ObjectPTR)) { RetNum=ERR_INVALID; return; }
            // CHECK FOR CORRUPTED DATA
            BINT k;
            for(k=0;k<r.len;++k) {
                // IF THE NUMBER IS NOT NORMALIZED, ASSUME IT WAS CORRUPTED
                if( (r.data[k]<0) || (r.data[k]>=100000000) ) { RetNum=ERR_INVALID; return; }
            }
            }
        }
        RetNum=OK_CONTINUE;
        return;


    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        //RetNum=ERR_NOTMINE;
        return;

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(MENUNUMBER(MenuCodeArg)>0) {
            RetNum=ERR_NOTMINE;
            return;
        }
        ObjectPTR=(WORDPTR)lib48_menu;
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
