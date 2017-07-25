/*
 * Copyright (c) 2017, Claudio Lapilli and the newRPL Team
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
#define LIBRARY_NUMBER  96


#define ERROR_LIST \
    ERR(COMPOSITEEXPECTED,0), \
    ERR(INVALIDPOSITION,1)


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(PUT,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(PUTI,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(GET,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(GETI,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(HEAD,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(TAIL,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(REPL,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(OBJDECOMP,"OBJ→",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(POS,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(SUB,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2))




// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);

INCLUDE_ROMOBJECT(lib96menu_main);



// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib96menu_main,
    0
};



void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if( (OPCODE(CurOpcode)==OVR_EVAL)||
                (OPCODE(CurOpcode)==OVR_EVAL1)||
                (OPCODE(CurOpcode)==OVR_XEQ) )
        {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORD saveOpcode=CurOpcode;
            CurOpcode=*rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode=saveOpcode;
            return;
        }
    }

    switch(OPCODE(CurOpcode))
    {

    case PUT:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

            WORDPTR comp=rplPeekData(3);
            WORDPTR *var=0;
            if(ISIDENT(*comp)) {
                var=rplFindLAM(comp,1);
                if(!var) {
                    var=rplFindGlobal(comp,1);
                    if(!var) {
                        rplError(ERR_COMPOSITEEXPECTED);
                        return;
                    }

                }
                comp=*(var+1);
            }


            // CHECK AND DISPATCH




            if(ISLIST(*comp)) {

            WORDPTR *stksave=DSTop;
            WORDPTR posobj,listelem;
            BINT ndims,k,position;
            if(ISLIST(*rplPeekData(2))) {
                ndims=rplListLength(rplPeekData(2));
            }
            else ndims=1;

            // EXTRACT ELEMENTS IN ALL DIMS BUT THE LAST
            listelem=comp;

            for(k=1;k<ndims;++k)
            {
                posobj=rplGetListElement(rplPeekData(2),k);
                if(!posobj) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }
                position=rplReadNumberAsBINT(posobj);

                if(Exceptions) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }
                if(!ISLIST(*listelem)) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }
                listelem=rplGetListElement(listelem,position);
                if(!listelem) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }
            }

            // HERE k==ndims= LAST DIMENSION
            // listelem = LAST LIST

            WORDPTR newobj=rplPeekData(1); // OBJECT TO REPLACE
            ScratchPointer3=comp;

            for(;k>=1;--k) {

                posobj=rplGetListElement(rplPeekData(2),k);
                if(!posobj) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }
                position=rplReadNumberAsBINT(posobj);

                if(Exceptions) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }

                if(!ISLIST(*listelem)) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }
                newobj=rplListReplace(listelem,position,newobj);
                if(!newobj) {
                    if(!Exceptions) rplError(ERR_INVALIDPOSITION);
                    return;
                }

                // NOW GET THE PARENT LIST
                    int j;
                    listelem=ScratchPointer3;
                    for(j=1;j<k-1;++j) {

                    posobj=rplGetListElement(rplPeekData(2),j);
                    if(!posobj) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    position=rplReadNumberAsBINT(posobj);

                    if(Exceptions) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }

                    listelem=rplGetListElement(listelem,position);
                    if(!listelem) {
                        DSTop=stksave;
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    }

            }

            // HERE newobj = NEW LIST WITH OBEJCT REPLACED


            rplOverwriteData(3,newobj);

            rplDropData(2);

            if(var) {
                *(var+1)=rplPopData();
            }

            return;
            }

            if(ISMATRIX(*comp)) {
                // DO IT FOR VECTORS AND MATRICES
                WORDPTR *stksave=DSTop;
                WORDPTR posobj;
                BINT rows,cols,ndims;
                BINT posrow,poscol;
                rows=rplMatrixRows(comp);
                cols=rplMatrixCols(comp);

                if(!rows) {
                    // THIS IS A VECTOR
                    ndims=1;
                    rows=1;
                } else ndims=2; // IT'S A 2D MATRIX


                // CHECK IF WE HAVE THE RIGHT POSITION
                if(ISLIST(*rplPeekData(2))) {
                    BINT nelem=rplListLengthFlat(rplPeekData(2));

                    if( (nelem!=ndims) && !( (ndims==2)&&(nelem==1))) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    posobj=rplGetListElementFlat(rplPeekData(2),1);
                    if(!posobj) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    posrow=rplReadNumberAsBINT(posobj);
                    if(Exceptions) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }

                    if(nelem==2) {
                      // READ THE SECOND COORDINATE (COLUMN)
                      posobj=rplGetListElementFlat(rplPeekData(2),2);
                      if(!posobj) {
                          rplError(ERR_INVALIDPOSITION);
                          return;
                      }
                      poscol=rplReadNumberAsBINT(posobj);
                      if(Exceptions) {
                          rplError(ERR_INVALIDPOSITION);
                          return;
                      }

                    } else {
                        if(ndims==2) {
                            // BREAK THE SINGLE POSITION IN TERMS OF ROW AND COLUMN
                            poscol=((posrow-1)%cols)+1;
                            posrow=((posrow-1)/cols)+1;
                        } else { poscol=posrow; posrow=1; }
                    }
                }
                else {
                    posobj=rplPeekData(2);
                    posrow=rplReadNumberAsBINT(posobj);
                    if(Exceptions) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }

                    if(ndims==2) {
                        // BREAK THE SINGLE POSITION IN TERMS OF ROW AND COLUMN
                        poscol=((posrow-1)%cols)+1;
                        posrow=((posrow-1)/cols)+1;
                    } else { poscol=posrow; posrow=1; }
                }

                // CHECK IF THE POSITION IS WITHIN THE MATRIX

                if( (posrow<1) || (posrow>rows) || (poscol<1) || (poscol>cols)) {
                    rplError(ERR_INDEXOUTOFBOUNDS);
                    return;
                }

                // CHECK IF THE OBJECT TO PUT IS VALID INSIDE A MATRIX

                if(!rplMatrixIsAllowed(rplPeekData(1))) {
                    rplError(ERR_NOTALLOWEDINMATRIX);
                    return;
                }

                // HERE POSITION POINTS TO THE FIRST ELEMENT
                rplPushDataNoGrow(comp);

                WORDPTR *first=rplMatrixExplode();
                if(Exceptions) { DSTop=stksave; return; }

                WORDPTR *elem=rplMatrixFastGetEx(first,cols,posrow,poscol);

                *elem=*(first-2);   // REPLACE THE ELEMENT

                WORDPTR newmatrix=rplMatrixCompose((ndims==2)? rows:0,cols);

                if( (!newmatrix) || Exceptions) { DSTop=stksave; return; }

                DSTop=stksave-2;    // ALREADY DROPPED LAST 2 ARGUMENTS HERE
                rplOverwriteData(1,newmatrix);

                if(var) {
                    *(var+1)=rplPopData();
                }

            return;
            }

            rplError(ERR_COMPOSITEEXPECTED);

            return;
    }
    case PUTI:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

            WORDPTR comp=rplPeekData(3);
            WORDPTR *var=0;
            if(ISIDENT(*comp)) {
                var=rplFindLAM(comp,1);
                if(!var) {
                    var=rplFindGlobal(comp,1);
                    if(!var) {
                        rplError(ERR_COMPOSITEEXPECTED);
                        return;
                    }

                }
                comp=*(var+1);
            }


            // CHECK AND DISPATCH




            if(ISLIST(*comp)) {
            BINT islist=0;
            WORDPTR *stksave=DSTop;
            WORDPTR posobj;
            if(ISLIST(*rplPeekData(2))) {
                islist=1;
                if(rplListLengthFlat(rplPeekData(2))!=1) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }
                posobj=rplGetListElementFlat(rplPeekData(2),1);
                if(!posobj) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }
            }
            else posobj=rplPeekData(2);

            BINT position=rplReadNumberAsBINT(posobj);

            if(Exceptions) {
                rplError(ERR_INVALIDPOSITION);
                return;
            }

            BINT nitems=rplExplodeList(comp);
            if(Exceptions) { DSTop=stksave; return; }

            if(position<1 || position>nitems) {
                DSTop=stksave;
                rplError(ERR_INDEXOUTOFBOUNDS);
                return;
            }
            rplOverwriteData(nitems+2-position,rplPeekData(nitems+2));

            rplCreateList();
            if(Exceptions) {
                DSTop=stksave;
                return;
            }

            if(position==nitems) {
                // INDEX MUST WRAP
                position=0;
                rplSetSystemFlag(FL_INDEXWRAP);
            }

            rplNewBINTPush(position+1,DECBINT);
            if(islist) rplCreateListN(1,1,1);
            if(Exceptions) {
                DSTop=stksave;
                return;
            }


            rplOverwriteData(5,rplPeekData(2));
            rplOverwriteData(4,rplPeekData(1));
            rplDropData(3);

            if(var) {
                *(var+1)=rplPeekData(2);
                rplOverwriteData(2,rplPeekData(1));
                rplDropData(1);
            }




            return;
            }

            if(ISMATRIX(*comp)) {
                // DO IT FOR VECTORS AND MATRICES
                WORDPTR *stksave=DSTop;
                WORDPTR posobj;
                BINT rows,cols,ndims,nelem=0;
                BINT posrow,poscol;
                rows=rplMatrixRows(comp);
                cols=rplMatrixCols(comp);

                if(!rows) {
                    // THIS IS A VECTOR
                    ndims=1;
                    rows=1;
                } else ndims=2; // IT'S A 2D MATRIX


                // CHECK IF WE HAVE THE RIGHT POSITION
                if(ISLIST(*rplPeekData(2))) {
                    nelem=rplListLengthFlat(rplPeekData(2));

                    if( (nelem!=ndims) && !( (ndims==2)&&(nelem==1))) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    posobj=rplGetListElementFlat(rplPeekData(2),1);
                    if(!posobj) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    posrow=rplReadNumberAsBINT(posobj);
                    if(Exceptions) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }

                    if(nelem==2) {
                      // READ THE SECOND COORDINATE (COLUMN)
                      posobj=rplGetListElementFlat(rplPeekData(2),2);
                      if(!posobj) {
                          rplError(ERR_INVALIDPOSITION);
                          return;
                      }
                      poscol=rplReadNumberAsBINT(posobj);
                      if(Exceptions) {
                          rplError(ERR_INVALIDPOSITION);
                          return;
                      }

                    } else {
                        if(ndims==2) {
                            // BREAK THE SINGLE POSITION IN TERMS OF ROW AND COLUMN
                            poscol=((posrow-1)%cols)+1;
                            posrow=((posrow-1)/cols)+1;
                        } else { poscol=posrow; posrow=1; }
                    }
                }
                else {
                    posobj=rplPeekData(2);
                    posrow=rplReadNumberAsBINT(posobj);
                    if(Exceptions) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }

                    if(ndims==2) {
                        // BREAK THE SINGLE POSITION IN TERMS OF ROW AND COLUMN
                        poscol=((posrow-1)%cols)+1;
                        posrow=((posrow-1)/cols)+1;
                    } else { poscol=posrow; posrow=1; }
                }

                // CHECK IF THE POSITION IS WITHIN THE MATRIX

                if( (posrow<1) || (posrow>rows) || (poscol<1) || (poscol>cols)) {
                    rplError(ERR_INDEXOUTOFBOUNDS);
                    return;
                }

                // CHECK IF THE OBJECT TO PUT IS VALID INSIDE A MATRIX

                if(!rplMatrixIsAllowed(rplPeekData(1))) {
                    rplError(ERR_NOTALLOWEDINMATRIX);
                    return;
                }

                // HERE POSITION POINTS TO THE FIRST ELEMENT
                rplPushDataNoGrow(comp);

                WORDPTR *first=rplMatrixExplode();
                if(Exceptions) { DSTop=stksave; return; }

                WORDPTR *elem=rplMatrixFastGetEx(first,cols,posrow,poscol);

                *elem=*(first-2);   // REPLACE THE ELEMENT

                WORDPTR newmatrix=rplMatrixCompose((ndims==2)? rows:0,cols);

                if( (!newmatrix) || Exceptions) { DSTop=stksave; return; }


                rplDropData(rows*cols+1);
                rplPushData(newmatrix);

                ++posrow;
                if(posrow>rows) {
                    posrow=1;
                    ++poscol;
                    if(poscol>cols) { poscol=posrow=1; rplSetSystemFlag(FL_INDEXWRAP); }
                }

                if(nelem!=2) rplNewBINTPush((posrow-1)*cols+poscol,DECBINT);
                else {
                    rplNewBINTPush(posrow,DECBINT);
                    rplNewBINTPush(poscol,DECBINT);
                }
                if(Exceptions) {
                    DSTop=stksave;
                    return;
                }

                if(nelem) rplCreateListN(nelem,1,1);
                if(Exceptions) {
                    DSTop=stksave;
                    return;
                }


                rplOverwriteData(5,rplPeekData(2));
                rplOverwriteData(4,rplPeekData(1));
                rplDropData(3);

                if(var) {
                    *(var+1)=rplPeekData(2);
                    rplOverwriteData(2,rplPeekData(1));
                    rplDropData(1);
                }

            return;
            }

            rplError(ERR_COMPOSITEEXPECTED);

            return;
    }

    case GET:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR list=rplPeekData(2);
        WORDPTR *var=0;
        if(ISIDENT(*list)) {
            var=rplFindLAM(list,1);
            if(!var) {
                var=rplFindGlobal(list,1);
                if(!var) {
                    rplError(ERR_LISTEXPECTED);

                    return;
                }

            }
            list=*(var+1);
        }
        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        BINT nitems=rplExplodeList(list);
        BINT position=rplReadNumberAsBINT(rplPeekData(nitems+2));
        if(Exceptions) return;
        if(position<1 || position>nitems) {
            rplDropData(nitems+1);
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }
        rplOverwriteData(nitems+3,rplPeekData(nitems+2-position));
        rplDropData(nitems+2);

    }
        return;



    case GETI:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR list=rplPeekData(2);
        WORDPTR *var=0;
        if(ISIDENT(*list)) {
            var=rplFindLAM(list,1);
            if(!var) {
                var=rplFindGlobal(list,1);
                if(!var) {
                    rplError(ERR_LISTEXPECTED);

                    return;
                }

            }
            list=*(var+1);
        }
        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);

            return;
        }

        BINT nitems=rplExplodeList(list);
        BINT position=rplReadNumberAsBINT(rplPeekData(nitems+2));
        if(Exceptions) return;
        if(position<1 || position>nitems) {
            rplDropData(nitems+1);
            rplError(ERR_INDEXOUTOFBOUNDS);

            return;
        }

        // HERE THE STACK IS: LIST POSITION OBJ1 ... OBJN N

        rplOverwriteData(nitems+1,rplPeekData(nitems+2-position));
        rplDropData(nitems);

        if(position==nitems) {
            // INDEX MUST WRAP
            position=0;
            rplSetSystemFlag(FL_INDEXWRAP);
        }

        rplNewBINTPush(position+1,DECBINT);
        rplOverwriteData(2,rplPopData());
    }
        return;


    case HEAD:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR list=rplPeekData(1);

        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        BINT nitems=rplExplodeList(list);
        if(Exceptions) return;
        if(nitems>0) {
            rplOverwriteData(nitems+2,rplPeekData(nitems+1));
            rplDropData(nitems+1);
        }
        else {
            rplDropData(1);
            rplError(ERR_EMPTYLIST);
            return;
        }


        return;
    }

    case TAIL:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR list=rplPeekData(1);

        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        BINT nitems=rplExplodeList(list);
        if(Exceptions) return;

        rplDropData(1);
        rplNewBINTPush(nitems-1,DECBINT);

        rplCreateList();
        if(Exceptions) return;
        // HERE THE STACK HAS: LIST OBJ1 NEWLIST
        rplOverwriteData(3,rplPeekData(1));
        rplDropData(2);

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
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors


            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

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
        libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);

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
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_NOTALLOWED,0,1);
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
        if(ISPROLOG(*ObjectPTR)) { RetNum=ERR_INVALID; return; }

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
        if(MENUNUMBER(MenuCodeArg)>0) { RetNum=ERR_NOTMINE; return; }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR=ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)+2];
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

