
/*
 * Copyright (c) 2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include "libraries.h"
#include "newrpl.h"


// FAST MACRO TO GET A POINTER TO AN ELEMENT
// INTERNAL USE ONLY
#define GETELEMENT(matrix,index) ((matrix)+(matrix)[2+(index)])


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
// RETURNS NULL IF ERROR, AND SETS Exceptions AND ExceptionPtr.
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
    for(j=0;j<k;++j) if(rplCompareObjects(obj,rplPeekData(totalelements-j))) break;
    if(j!=k) {
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


rplTruncateLastObject(newobj);

matrix[0]=MKPROLOG(DOMATRIX,newobj-matrix-1);
matrix[1]=MATMKSIZE(rows,cols);

return matrix;

}


// APPLIES ANY OVERLOADABLE BINARY OPERATOR THAT WORKS ELEMENT-BY-ELEMENT (ADD/SUBTRACT)

void rplMatrixBinary(WORD Opcode)
{
WORDPTR *Savestk,*a,*b;
// DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
// AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
Savestk=DSTop;
a=DSTop-2;
b=DSTop-1;

// CHECK TYPE

if(!ISMATRIX(**a) || !ISMATRIX(**b)) {
    Exceptions|=EX_BADARGTYPE;
    ExceptionPointer=IPtr;
    return;
}

// CHECK DIMENSIONS

BINT rowsa=MATROWS(*(*a+1)),colsa=MATCOLS(*(*a+1));
BINT rowsb=MATROWS(*(*b+1)),colsb=MATCOLS(*(*b+1));

if(rowsa!=rowsb) {
    // CHECK IF ONE OF THEM IS A VECTOR AND AUTOTRANSPOSE
    if(!rowsa) {
        if(rowsb==1) rowsa=1;
        else {
            // TRANSPOSE A AND TRY
            rowsa=colsa;
            colsa=1;
            if(rowsa!=rowsb) {
                Exceptions|=EX_INVALID_DIM;
                ExceptionPointer=IPtr;
                return;
            }
        }
    } else {
        if(!rowsb) {
            if(rowsa==1) rowsb=1;
            else {
                // TRANSPOSE B AND TRY
                rowsb=colsb;
                colsb=1;
                if(rowsa!=rowsb) {
                    Exceptions|=EX_INVALID_DIM;
                    ExceptionPointer=IPtr;
                    return;
                }
            }
        } else {
            Exceptions|=EX_INVALID_DIM;
            ExceptionPointer=IPtr;
            return;
        }

    }


}

// CHECK COLUMN SIZE
if(colsa!=colsb) {
        Exceptions|=EX_INVALID_DIM;
        ExceptionPointer=IPtr;
        return;
    }

// HERE WE HAVE COMPATIBLE SIZE VECTOR/MATRIX

BINT totalelements=(rowsa)? rowsa*colsa:colsa;

BINT j;

// DO THE ELEMENT-BY-ELEMENT OPERATION

for(j=0;j<totalelements;++j) {
 rplPushData(GETELEMENT(*a,j));
 rplPushData(GETELEMENT(*b,j));
 rplCallOvrOperator(Opcode);
 if(Exceptions) {
     DSTop=Savestk;
     return;
 }
 if(ISSYMBOLIC(*rplPeekData(1))) {
     rplSymbAutoSimplify();
     if(Exceptions) {
         DSTop=Savestk;
         return;
     }
 }

}

WORDPTR newmat=rplMatrixCompose(rowsa,colsa);
DSTop=Savestk;
if(!newmat) return;
rplOverwriteData(2,newmat);
rplDropData(1);
}

// ADD/SUBTRACT TWO MATRICES, A+B, WITH A IN LEVEL 2 AND B IN LEVEL 1 OF THE STACK
void rplMatrixAdd() { rplMatrixBinary(MKOPCODE(LIB_OVERLOADABLE,OVR_ADD)); }
void rplMatrixSub() { rplMatrixBinary(MKOPCODE(LIB_OVERLOADABLE,OVR_SUB)); }


// MATRIX BY MATRIX MULTIPLICATION
// DOES A*B WITH A ON LEVEL 2 AND B ON LEVEL 1
// DOES AUTOTRANSPOSE OF VECTORS

void rplMatrixMul()
{

    WORDPTR *Savestk,*a,*b;
    // DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
    // AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk=DSTop;
    a=DSTop-2;
    b=DSTop-1;

    // CHECK TYPE

    if(!ISMATRIX(**a) || !ISMATRIX(**b)) {
        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
        return;
    }

    // CHECK DIMENSIONS

    BINT rowsa=MATROWS(*(*a+1)),colsa=MATCOLS(*(*a+1));
    BINT rowsb=MATROWS(*(*b+1)),colsb=MATCOLS(*(*b+1));
    BINT rrows,rcols;

    if(rowsa==0 && rowsb==0) {
        // VECTOR BY VECTOR MULTIPLICATION
        // DO A DOT PRODUCT
        if(colsa!=colsb) {
            Exceptions|=EX_INVALID_DIM;
            ExceptionPointer=IPtr;
            return;
        }

        // DIMENSION OF THE RESULTING OUTPUT
        rowsa=1;
        rowsb=colsb;
        colsb=1;

        // THIS IS A SCALAR VALUE, NOT A MATRIX
        rrows=0;
        rcols=1;
    }
    else {

        if(!rowsa) {
            // VECTOR x MATRIX = VECTOR

            if(colsa==rowsb) {
                // USE IT AS ROW VECTOR (1xN)x(NxM)
                rowsa=1;
                // RESULT IS A VECTOR
                rrows=0;
                rcols=colsb;

            }
            else {
                if(rowsb==1) {
                    // TRANSPOSE THE VECTOR (Nx1)x(1xM)=(NxM)
                    rowsa=colsa;
                    colsa=1;
                    // RESULTING ELEMENT IS A MATRIX
                    rrows=rowsa;
                    rcols=colsb;
                }
                else {
                    Exceptions|=EX_INVALID_DIM;
                    ExceptionPointer=IPtr;
                    return;
                }
            }


        }
        else {
            if(!rowsb) {
                // MATRIX x VECTOR MULTIPLICATION
                if(colsa==colsb) {
                    // TRANSPOSE THE VECTOR (NxM)x(Mx1)
                    rowsb=colsb;
                    colsb=1;

                    // RESULT IS A VECTOR
                    rrows=0;
                    rcols=rowsa;

                }
                else {
                    if(colsa==1) {
                        // (Nx1)x(1xM)
                        rowsb=1;

                        // RESULT IS A MATRIX
                        rrows=rowsa;
                        rcols=colsb;
                    }
                    else {
                        Exceptions|=EX_INVALID_DIM;
                        ExceptionPointer=IPtr;
                        return;
                    }



                }


            }
            else {
                // MATRIX x MATRIX MULTIPLICATION

                // NO AUTOTRANSPOSE HERE
                if(colsa!=rowsb) {
                    Exceptions|=EX_INVALID_DIM;
                    ExceptionPointer=IPtr;
                    return;
                }

                rrows=rowsa;
                rcols=colsb;

            }

        }

    }

    // HERE WE HAVE PROPER DIMENSIONS TO DO THE MULTIPLICATION

    BINT i,j,k;

    for(i=0;i<rowsa;++i) {

        for(j=0;j<colsb;++j) {
            for(k=0;k<colsa;++k) {
            rplPushData(GETELEMENT(*a,i*colsa+k));
            rplPushData(GETELEMENT(*b,k*colsb+j));
            rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL));
            if(Exceptions) {
                // CLEAN UP THE STACK AND RETURN
                DSTop=Savestk;
                return;
            }
            if(k) {
                rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_ADD));
                if(Exceptions) {
                    // CLEAN UP THE STACK AND RETURN
                    DSTop=Savestk;
                    return;
                }
            }
            }
            if(ISSYMBOLIC(*rplPeekData(1))) {
                rplSymbAutoSimplify();
                if(Exceptions) {
                    DSTop=Savestk;
                    return;
                }
            }


        }

    }


    // HERE WE HAVE ALL THE ELEMENTS IN ROW ORDER
    WORDPTR newmat;
    if(rrows+rcols>1) {
        newmat=rplMatrixCompose(rrows,rcols);
        if(!newmat) {
            DSTop=Savestk;
            return;
        }
    }
    else newmat=rplPopData();

    DSTop=Savestk;
    rplOverwriteData(2,newmat);
    rplDropData(1);

}
