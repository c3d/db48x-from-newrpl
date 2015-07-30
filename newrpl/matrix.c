
/*
 * Copyright (c) 2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include "libraries.h"
#include "newrpl.h"


// GET A POINTER TO AN OBJECT WITHIN THE MATRIX/VECTOR
// RETURN NULL ON OUT-OF-RANGE
// VECTORS ARE AUTO-ROTATED
// ROWS AND COLUMNS ARE 1-BASED

WORDPTR rplMatrixGet(WORDPTR matrix,BINT row,BINT col)
{
    if(!ISMATRIX(matrix)) return NULL;
    BINT rows=MATROWS(*(matrix+1)),cols=MATCOLS(*(matrix+1));

    if(!rows) {
        // THIS IS A VECTOR
        if(row==1) {
            if(col>0 && col<=cols) return matrix+matrix[col+1];
            else return NULL;
        } else if(col==1) {
            if(row>0 && row<=cols) return matrix+matrix[row+1];
            else return NULL;
            } else return NULL;
    }

    if(row<1 || row>rows) return NULL;
    if(col<1 || col>cols) return NULL;

    return matrix+matrix[(row-1)*cols+col+1];

}

// GET AN ELEMENT OF AN ARRAY - LOW-LEVEL, NO CHECKS OF ANY KIND DONE

WORDPTR rplMatrixFastGet(WORDPTR matrix,BINT row,BINT col)
{
    BINT rows=MATROWS(*(matrix+1)),cols=MATCOLS(*(matrix+1));
    if(!rows) return matrix+matrix[row+col];
    return matrix+matrix[(row-1)*cols+col+1];
}


// COMPOSES A NEW MATRIX OBJECT FROM OBJECTS IN THE STACK
// OBJECTS MUST BE IN ROW-ORDER
// RETURNS NULL IF ERROR, AND SETS Exception AND ExceptionPtr.
// CREATES A VECTOR IF ROWS == 0, OTHERWISE A MATRIX

WORDPTR rplMatrixCompose(BINT rows,BINT cols)
{
BINT totalelements=(rows)? rows*cols:cols;

// CHECK IF ENOUGH ELEMENTS IN THE STACK
if(rplDepthData()<totalelements) {
    Exceptions|=EX_BADARGCOUNT;
    ExceptionPointer=IPtr;
    return NULL;
}


//   CHECK VALIDITY OF ALL ELEMENTS
BINT k,j,totalsize=0;
WORDPTR obj;

for(k=1;k<=totalelements;++k) {
obj=rplPeekData(k);
if(! (ISNUMBERCPLX(*obj)
      || ISSYMBOLIC(*obj)
      || ISIDENT(*obj))) {
    Exceptions|=EX_BADARGTYPE;
    ExceptionPointer=IPtr;
    return NULL;
    }
totalsize+=rplObjSize(obj);
}

WORDPTR matrix=rplAllocTempOb(totalsize+1+totalelements);
WORDPTR newobj=matrix+2+totalelements;  // POINT TO THE NEXT OBJECT TO STORE

if(!matrix) return NULL;

// FINALLY, ASSEMBLE THE OBJECT
for(k=0;k<totalelements;++k) {
    obj=rplPeekData(totalelements-k);
    for(j=0;j<k;++j) {
        if(rplCompareObjects(obj,rplPeekData(totalelements-j))) {
            // ADD THE ORIGINAL OBJECT
            matrix[2+k]=matrix[2+j];
        }
        else {
            // ADD A NEW OBJECT
            matrix[2+k]=newobj-matrix;
            rplCopyObject(newobj,obj);
            newobj=rplSkipOb(newobj);
        }
    }
}


rplTruncateLastObject(newobj);

matrix[0]=MKPROLOG(DOMATRIX,newobj-matrix-1);
matrix[1]=MATMKSIZE(rows,cols);

return matrix;

}


// ADD TWO MATRICES, A+B, WITH A IN LEVEL 2 AND B IN LEVEL 1 OF THE STACK
void rplMatrixAdd()
{

}
