/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include "libraries.h"
#include "hal.h"

// COMPARE TWO ITEMS WITHIN AN ARRAY, BY CALLING THE OPERATOR CMP
// OPERATOR CMP MUST RETURN -1, 0 OR 1 IF B>A, B==A, OR A>B RESPECTIVELY
BINT rplArrayItemCompare(WORDPTR a,WORDPTR b)
{

    rplPushData(a);
    rplPushData(b);
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_CMP));
    if(Exceptions) return 0;
    BINT r=rplReadBINT(rplPopData());
    if(r==0) return (BINT)(a-b);
    return r;

}

// PERFORM AN OPERATION BETWEEN 2 ITEMS, POP THE RESULT FROM THE STACK
// AND RETURN IT AS A POINTER TO THE OBJECT.
// KEEPS THE STACK CLEAN EVEN IF THERE ARE EXCEPTIONS
// USES STACK PROTECTION AND PERFORMS PROPER STACK CLEANUP

WORDPTR rplArrayItemBinaryOp(WORDPTR a,WORDPTR b, WORD Opcode)
{
    rplProtectData();
    rplPushData(a);
    rplPushData(b);
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OPCODE(Opcode)));
    if(Exceptions) {
        rplClearData();
        rplUnprotectData();
        return 0;
    }
    WORDPTR result=0;
    if(rplDepthData()>0)   result=rplPopData();
    rplClearData();
    rplUnprotectData();
    return result;
}

WORDPTR rplArrayItemUnaryOp(WORDPTR a, WORD Opcode)
{
    rplProtectData();
    rplPushData(a);
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OPCODE(Opcode)));
    if(Exceptions) {
        rplClearData();
        rplUnprotectData();
        return 0;
    }
    WORDPTR result=0;
    if(rplDepthData()>0)   result=rplPopData();
    rplClearData();
    rplUnprotectData();
    return result;
}


// BASIC LOW-LEVEL GET ITEM
// DOESN'T CHECK FOR ARGUMENT TO BE A MATRIX OR FOR INDEX OUT OF RANGE

// INDEX STARTS AT ZERO!
WORDPTR rplArrayGetItem(WORDPTR matrix,BINT row,BINT col)
{
    return matrix+*(matrix+2+row*MATCOLS(matrix[1])+col);
}



// CREATE A NEW OBJECT WITH A TRANSPOSED ARRAY
// LOW-LEVEL DOESN'T CHECK FOR ITS ARGUMENTS
WORDPTR rplArrayTranspose(WORDPTR matrix)
{
    // PROTECT THE MATRIX FROM GARBAGE COLLECTION BY PUSHING IN THE STACK
    rplPushData(matrix);
    WORDPTR newmat=rplAllocTempOb(rplObjSize(matrix));
    if(!newmat) { rplPopData(); return newmat; }

    matrix=rplPopData();

    newmat[0]=matrix[0];


    BINT cols=MATROWS(matrix[1]),rows=MATCOLS(matrix[1]);
    newmat[1]=MATMKSIZE(rows,cols);

    BINT i,j;
    WORDPTR src=matrix+2,dest=newmat+2;

    for(i=0;i<rows;++i) {
        for(j=0;j<cols;++j) {
            dest[i*cols+j]=src[j*rows+i];
        }
    }

    memmovew(dest+rows*cols,src+rows*cols,rplSkipOb(matrix)-(src+rows*cols));

    return newmat;
}

