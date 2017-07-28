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
    ECMD(OBJDECOMP,"OBJ→",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(REPL,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
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
                    if(position!=1) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                    }
                } else listelem=rplGetListElement(listelem,position);
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

                if(!ISLIST(*rplPeekData(2))) posobj=rplPeekData(2);
                else posobj=rplGetListElement(rplPeekData(2),k);
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
                    if(position!=1) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                    }
                    // KEEP THE PREVIOUS OBJECT
                } else newobj=rplListReplace(listelem,position,newobj);
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

                    if(ISLIST(*listelem)) listelem=rplGetListElement(listelem,position);
                    else if(position!=1) {
                        DSTop=stksave;
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    if(!listelem) {
                        DSTop=stksave;
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    }


            }

            // HERE newobj = NEW LIST WITH OBJECT REPLACED

            if(var) { *(var+1)=newobj; rplDropData(3); }
            else { rplOverwriteData(3,newobj);  rplDropData(2); }

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
                    if(position!=1) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                    }
                } else listelem=rplGetListElement(listelem,position);
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

                if(!ISLIST(*rplPeekData(2))) posobj=rplPeekData(2);
                else posobj=rplGetListElement(rplPeekData(2),k);
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
                    if(position!=1) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                    }
                    // KEEP THE PREVIOUS OBJECT
                } else newobj=rplListReplace(listelem,position,newobj);
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

                    if(ISLIST(*listelem)) listelem=rplGetListElement(listelem,position);
                    else if(position!=1) {
                        DSTop=stksave;
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    if(!listelem) {
                        DSTop=stksave;
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    }


            }

            // HERE newobj = NEW LIST WITH OBJECT REPLACED

            if(var) *(var+1)=newobj;
            else rplOverwriteData(3,newobj);


            // UPDATE THE INDEX, NOW AT LEVEL 1

            if(ISLIST(*rplPeekData(2))) rplExplodeList2(rplPeekData(2));
            else rplPushData(rplPeekData(2));

            BINT carry=1,llen,tpos;
            for(k=1;(k<=ndims)&&carry;++k)
                {
                    position=rplReadNumberAsBINT(rplPeekData(k))+carry; // INCREASE POSITION

                // NOW GET THE PARENT LIST
                    int j;
                    listelem=ScratchPointer3;
                    for(j=1;j<ndims-(k-1);++j) {

                    tpos=rplReadNumberAsBINT(rplPeekData(ndims-(j-1)));
                    if(ISLIST(*listelem)) listelem=rplGetListElement(listelem,tpos);
                    }

                    if(ISLIST(*listelem)) llen=rplListLength(listelem);
                    else llen=1;
                    if(position>llen) { position=1; carry=1; }
                    else carry=0;

                    WORDPTR newnum=rplNewBINT(position,DECBINT);
                    if(!newnum) {
                        DSTop=stksave;
                        return;
                    }

                    rplOverwriteData(k,newnum);


            }

            if(carry) rplSetSystemFlag(FL_INDEXWRAP);
            else rplClrSystemFlag(FL_INDEXWRAP);

            if(ISLIST(*rplPeekData(2+ndims))) newobj=rplCreateListN(ndims,1,1);
            else newobj=rplPopData();

            // HERE newobj HAS THE UPDATED INDEX
            rplOverwriteData(2,newobj);

            rplDropData(1);

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

                ++poscol;
                if(poscol>cols) {
                    poscol=1;
                    ++posrow;
                    if(posrow>rows) { poscol=posrow=1; rplSetSystemFlag(FL_INDEXWRAP); }
                    else rplClrSystemFlag(FL_INDEXWRAP);
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

                WORDPTR newposlist;
                if(nelem) {
                    newposlist=rplCreateListN(nelem,1,1);
                    if(!newposlist) {
                        DSTop=stksave;
                        return;
                    }
                    rplPushDataNoGrow(newposlist);
                }

                // HERE WE HAVE: LEVEL 5=MATRIX OR IDENT, LEVEL 4=OLD POSITION, LEVEL 3=NEW OBJECT, LEVEL 2=NEW MATRIX, LEVEL 1=UPDATED POSITION

                rplOverwriteData(4,rplPeekData(1)); // UPDATE POSITION

                if(var) {
                    *(var+1)=rplPeekData(2);
                } else {
                    rplOverwriteData(5,rplPeekData(2));
                }
                rplDropData(3);
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

            WORDPTR comp=rplPeekData(2);
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

            WORDPTR posobj,listelem;
            BINT ndims,k,position;
            if(ISLIST(*rplPeekData(1))) {
                ndims=rplListLength(rplPeekData(1));
            }
            else ndims=1;

            // EXTRACT ELEMENTS IN ALL DIMS BUT THE LAST
            listelem=comp;

            for(k=1;k<=ndims;++k)
            {
                if(!ISLIST(*rplPeekData(1))) posobj=rplPeekData(1);
                    else posobj=rplGetListElement(rplPeekData(1),k);
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
                    if(position!=1) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                    }
                } else listelem=rplGetListElement(listelem,position);
                if(!listelem) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }
            }


            // listelem = GOT ELEMENT

            if(var) { *(var+1)=listelem; rplDropData(2); }
            else { rplOverwriteData(2,listelem);  rplDropData(1); }

            return;
            }

            if(ISMATRIX(*comp)) {
                // DO IT FOR VECTORS AND MATRICES
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
                if(ISLIST(*rplPeekData(1))) {
                    BINT nelem=rplListLengthFlat(rplPeekData(1));

                    if( (nelem!=ndims) && !( (ndims==2)&&(nelem==1))) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    posobj=rplGetListElementFlat(rplPeekData(1),1);
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
                      posobj=rplGetListElementFlat(rplPeekData(1),2);
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
                    posobj=rplPeekData(1);
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

                WORDPTR item=rplMatrixGet(comp,posrow,poscol);
                if(!item) {
                    rplError(ERR_INDEXOUTOFBOUNDS);
                    return;
                }

                if(var) {
                    *(var+1)=item;
                    rplDropData(2);
                } else {
                    rplOverwriteData(2,item);
                    rplDropData(1);
                }

            return;
            }

            rplError(ERR_COMPOSITEEXPECTED);

            return;
    }



    case GETI:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

            WORDPTR comp=rplPeekData(2);
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
            if(ISLIST(*rplPeekData(1))) {
                ndims=rplListLength(rplPeekData(1));
            }
            else ndims=1;

            // EXTRACT ELEMENTS IN ALL DIMS BUT THE LAST
            listelem=comp;

            for(k=1;k<=ndims;++k)
            {
                if(!ISLIST(*rplPeekData(1))) posobj=rplPeekData(1);
                else posobj=rplGetListElement(rplPeekData(1),k);
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
                    if(position!=1) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                    }
                } else listelem=rplGetListElement(listelem,position);
                if(!listelem) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }
            }

            // HERE k==ndims= LAST DIMENSION
            // listelem = OBJECT

            WORDPTR newobj;
            ScratchPointer3=comp;

            rplPushData(listelem);


            // UPDATE THE INDEX, NOW AT LEVEL 2

            if(ISLIST(*rplPeekData(2))) rplExplodeList2(rplPeekData(2));
            else rplPushData(rplPeekData(2));

            BINT carry=1,llen,tpos;
            for(k=1;(k<=ndims)&&carry;++k)
                {
                    position=rplReadNumberAsBINT(rplPeekData(k))+carry; // INCREASE POSITION

                // NOW GET THE PARENT LIST
                    int j;
                    listelem=ScratchPointer3;
                    for(j=1;j<ndims-(k-1);++j) {

                    tpos=rplReadNumberAsBINT(rplPeekData(ndims-(j-1)));
                    if(ISLIST(*listelem)) listelem=rplGetListElement(listelem,tpos);
                    }

                    if(ISLIST(*listelem)) llen=rplListLength(listelem);
                    else llen=1;
                    if(position>llen) { position=1; carry=1; }
                    else carry=0;

                    WORDPTR newnum=rplNewBINT(position,DECBINT);
                    if(!newnum) {
                        DSTop=stksave;
                        return;
                    }

                    rplOverwriteData(k,newnum);


            }

            if(carry) rplSetSystemFlag(FL_INDEXWRAP);
            else rplClrSystemFlag(FL_INDEXWRAP);

            if(ISLIST(*rplPeekData(2+ndims))) newobj=rplCreateListN(ndims,1,1);
            else newobj=rplPopData();

            if(!newobj) {
                DSTop=stksave;
                return;
            }

            // HERE newobj HAS THE UPDATED INDEX
            rplOverwriteData(2,newobj);

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
                if(ISLIST(*rplPeekData(1))) {
                    nelem=rplListLengthFlat(rplPeekData(1));

                    if( (nelem!=ndims) && !( (ndims==2)&&(nelem==1))) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }
                    posobj=rplGetListElementFlat(rplPeekData(1),1);
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
                      posobj=rplGetListElementFlat(rplPeekData(1),2);
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
                    posobj=rplPeekData(1);
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

                WORDPTR item=rplMatrixGet(comp,posrow,poscol);
                if(!item) {
                    rplError(ERR_INDEXOUTOFBOUNDS);
                    return;
                }

                rplPushData(item);

                ++poscol;
                if(poscol>cols) {
                    poscol=1;
                    ++posrow;
                    if(posrow>rows) { poscol=posrow=1; rplSetSystemFlag(FL_INDEXWRAP); }
                    else rplClrSystemFlag(FL_INDEXWRAP);
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

                WORDPTR newposlist;
                if(nelem) {
                    newposlist=rplCreateListN(nelem,1,1);
                    if(!newposlist) {
                        DSTop=stksave;
                        return;
                    }
                    rplPushDataNoGrow(newposlist);
                }

                // HERE WE HAVE: LEVEL 4=MATRIX OR IDENT, LEVEL 3=OLD POSITION, LEVEL 2=item, LEVEL 1=UPDATED POSITION

                rplOverwriteData(3,rplPeekData(1)); // UPDATE POSITION

                rplDropData(1);
                return;
            }


            rplError(ERR_COMPOSITEEXPECTED);

            return;
    }
        return;


    case HEAD:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR comp=rplPeekData(1);

        if(ISLIST(*comp)) {
        BINT nitems=rplListLength(comp);
        if(nitems>0) rplOverwriteData(1,rplGetListElement(comp,1));
        else rplError(ERR_EMPTYLIST);
        return;
        }

        if(ISSTRING(*comp)) {

        BYTEPTR start=(BYTEPTR) (comp+1);
        BYTEPTR end=start+rplStrSize(comp);
        BYTEPTR ptr=(BYTEPTR)utf8skipst((char *)start,(char *)end);
        if(ptr==start) {
            rplError(ERR_EMPTYSTRING);
            return;
        }
        WORDPTR newstring=rplCreateString(start,ptr);
        if(!newstring) return;
        rplOverwriteData(1,newstring);
        return;
        }


        rplError(ERR_COMPOSITEEXPECTED);

        return;

    }


    case TAIL:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR comp=rplPeekData(1);

        if(ISLIST(*comp)) {
        BINT nitems=rplExplodeList2(comp);
        WORDPTR newlist;

        if(Exceptions) return;
        if(nitems<2) {
            newlist=(WORDPTR)empty_list;
        }
        else {
        newlist=rplCreateListN(nitems-1,1,1);
        if(!newlist) return;
        }
        if(nitems>0) rplDropData(1);
        rplOverwriteData(1,newlist);
        return;
        }

        if(ISSTRING(*comp)) {
            BYTEPTR start=(BYTEPTR) (comp+1);
            BYTEPTR end=start+rplStrSize(comp);
            BYTEPTR ptr=(BYTEPTR)utf8skipst((char *)start,(char *)end);

            WORDPTR newstring=rplCreateString(ptr,end);
            if(!newstring) return;
            rplOverwriteData(1,newstring);
            return;
        }

        rplError(ERR_COMPOSITEEXPECTED);

        return;
    }


    case OBJDECOMP:
    {
        // EXPLODE ANY COMPOSITE INTO COMPONENTS
        // CHECK ARGUMENTS
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR comp=rplPeekData(1);


        if(ISCOMPLEX(*comp)) {
            rplCallOperator(CMD_CPLX2REAL);
            return;
        }

        if(ISLIST(*comp)) {
            rplCallOperator(CMD_INNERCOMP);
            return;
        }

        if(ISMATRIX(*comp)) {
            rplCallOperator(CMD_ARRAYDECOMP);
            return;
        }

        if(ISSTRING(*comp)) {
            rplCallOperator(CMD_FROMSTR);
            return;
        }

        if(ISUNIT(*comp)) {
            // SPLIT VALUE AND UNIT

            BINT nitems=rplUnitExplode(comp);
            rplOverwriteData(nitems+1,rplPeekData(nitems)); // REPLACE ORIGINAL UNIT WITH THE VALUE
            rplOverwriteData(nitems,(WORDPTR)one_bint); //  CHANGE VALUE TO 1
            WORDPTR newunit=rplUnitAssemble(nitems);
            rplDropData(nitems);
            if(!newunit) return;
            rplPushData(newunit);
            return;
        }

        if(ISSYMBOLIC(*comp)) {
            // SPLIT MAIN OPERATION INTO:
            // ARG1 ... ARGN
            // N
            // OPERATOR (PUSHED IN THE STACK AS OBJECT)
            WORDPTR *savestk=DSTop;
            BINT nitems=rplSymbExplodeOneLevel(comp);
            if(nitems>1) {

                if(*rplPeekData(1)==CMD_OVR_FUNCEVAL) {
                    --nitems;
                    WORDPTR number=rplNewSINT(nitems-2,DECBINT);
                    if(Exceptions || (!number)) { DSTop=savestk; return; }

                    // MAKE IT COMPATIBLE BY SHOWING THE IDENT AS THE OPERATOR
                    rplDropData(1);
                    rplOverwriteData(1,rplPeekData(2)); // MAKE THE IDENT OF THE FUNCTION CALL THE OPERATOR
                    rplOverwriteData(2,number);
                }
            }
            rplRemoveAtData(nitems+1,1);

            return;


        }


        // TODO: ADD TAG OBJECTS HERE (NOT YET IMPLEMENTED)


        rplError(ERR_COMPOSITEEXPECTED);

        return;
    }



    case REPL:
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
                        if(position!=1) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                        }
                    } else listelem=rplGetListElement(listelem,position);
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

                    if(!ISLIST(*rplPeekData(2))) posobj=rplPeekData(2);
                    else posobj=rplGetListElement(rplPeekData(2),k);
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
                        if(position!=1) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                        }
                        // KEEP THE PREVIOUS OBJECT
                    } else {
                        if(newobj==rplPeekData(1)) newobj=rplListReplaceMulti(listelem,position,newobj);
                        else newobj=rplListReplace(listelem,position,newobj);
                    }
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

                        if(ISLIST(*listelem)) listelem=rplGetListElement(listelem,position);
                        else if(position!=1) {
                            DSTop=stksave;
                            rplError(ERR_INVALIDPOSITION);
                            return;
                        }
                        if(!listelem) {
                            DSTop=stksave;
                            rplError(ERR_INVALIDPOSITION);
                            return;
                        }
                        }


                }

                // HERE newobj = NEW LIST WITH OBJECT REPLACED

                if(var) { *(var+1)=newobj; rplDropData(3); }
                else { rplOverwriteData(3,newobj);  rplDropData(2); }

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
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }

                    // CHECK IF THE OBJECT TO PUT IS VALID INSIDE A MATRIX
                    BINT putnrows,putncols,ismatrix;

                    if(!ISMATRIX(*rplPeekData(1))) {
                    if(!rplMatrixIsAllowed(rplPeekData(1))) {
                        rplError(ERR_NOTALLOWEDINMATRIX);
                        return;
                    }
                    putnrows=putncols=1;
                    ismatrix=0;
                    } else {
                        putnrows=rplMatrixRows(rplPeekData(1));
                        putncols=rplMatrixCols(rplPeekData(1));
                        if(!putnrows) putnrows=1;
                        ismatrix=1;
                    }

                    // CHECK IF OBJECT FITS
                    if((posrow+putnrows-1>rows)||(poscol+putncols-1>cols)) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }

                    // HERE POSITION POINTS TO THE FIRST ELEMENT
                    rplPushDataNoGrow(comp);

                    WORDPTR *first=rplMatrixExplode();
                    if(Exceptions) { DSTop=stksave; return; }
                    BINT i,j;

                    for(i=1;i<=putnrows;++i) {
                        for(j=1;j<=putncols;++j) {
                            WORDPTR *elem=rplMatrixFastGetEx(first,cols,posrow+i-1,poscol+j-1);

                            *elem=(ismatrix)? rplMatrixFastGet(*(first-2),i,j) : *(first-2) ;   // REPLACE THE ELEMENT

                        }
                    }

                    WORDPTR newmatrix=rplMatrixCompose((ndims==2)? rows:0,cols);

                    if( (!newmatrix) || Exceptions) { DSTop=stksave; return; }

                    DSTop=stksave-2;    // ALREADY DROPPED LAST 2 ARGUMENTS HERE
                    rplOverwriteData(1,newmatrix);

                    if(var) {
                        *(var+1)=rplPopData();
                    }

                return;
                }


                if(ISSTRING(*comp)) {
                    // REPLACE IN STRINGS

                    if(!ISSTRING(*rplPeekData(1))) {
                        rplError(ERR_STRINGEXPECTED);
                        return;
                    }

                    BINT orglen,replen,replpos;
                    replpos=rplReadNumberAsBINT(rplPeekData(2));
                    if(Exceptions) return;

                    orglen=rplStrLen(comp);

                    if( (replpos<1) || (replpos>orglen)) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }

                    replen=rplStrLen(rplPeekData(1));
                    BINT posptr,posendptr;
                    BINT strend,repsize;
                    BYTEPTR strstart,newstart,repstart;

                    strstart=(BYTEPTR) (comp+1);
                    strend=rplStrSize(comp);
                    repsize=rplStrSize(rplPeekData(1));
                    posptr=(BYTEPTR)utf8nskipst((char *)strstart,(char *)strstart+strend,replpos-1)-strstart;   // START REPLACING
                    posendptr=(BYTEPTR)utf8nskipst((char *)strstart+posptr,(char *)strstart+strend,replen)-strstart;   // END REPLACING

                    BINT totalsize=(posptr)+repsize+(strend-posendptr);

                    WORDPTR newstring=rplCreateStringBySize(totalsize);
                    if(!newstring) return;
                    // REREAD STRINGS FROM STACK, POSSIBLE GC

                    strstart=(BYTEPTR) (rplPeekData(3)+1);
                    newstart=(BYTEPTR) (newstring+1);
                    repstart=(BYTEPTR) (rplPeekData(1)+1);

                    // COPY THE STRING TO FINAL DESTINATION
                    memmoveb(newstart,strstart,posptr);
                    memmoveb(newstart+posptr,repstart,repsize);
                    memmoveb(newstart+posptr+repsize,strstart+posendptr,strend-posendptr);

                    rplOverwriteData(3,newstring);
                    rplDropData(2);
                    return;
                }



                // TODO: ADD SUPPORT FOR GROBS!




                rplError(ERR_COMPOSITEEXPECTED);

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

