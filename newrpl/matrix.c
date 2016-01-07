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
// RETURN 0 ON OUT-OF-RANGE
// VECTORS ARE AUTO-ROTATED
// ROWS AND COLUMNS ARE 1-BASED

WORDPTR rplMatrixGet(WORDPTR matrix,BINT row,BINT col)
{
    if(!ISMATRIX(matrix)) return 0;
    BINT rows=MATROWS(*(matrix+1)),cols=MATCOLS(*(matrix+1));

    if(!rows) {
        // THIS IS A VECTOR
        if(row==1) {
            if(col>0 && col<=cols) return matrix+matrix[col+1];
            else return 0;
        } else if(col==1) {
            if(row>0 && row<=cols) return matrix+matrix[row+1];
            else return 0;
            } else return 0;
    }

    if(row<1 || row>rows) return 0;
    if(col<1 || col>cols) return 0;

    return matrix+matrix[(row-1)*cols+col+1];

}

// GET A POINTER TO THE FIRST OBJECT WITHIN A MATRIX

WORDPTR rplMatrixGetFirstObj(WORDPTR matrix)
{
    BINT rows=MATROWS(*(matrix+1)),cols=MATCOLS(*(matrix+1));
    if(!rows) rows=1;
    return matrix+2+rows*cols;
}



// GET AN ELEMENT OF AN ARRAY - LOW-LEVEL, NO CHECKS OF ANY KIND DONE

WORDPTR rplMatrixFastGet(WORDPTR matrix,BINT row,BINT col)
{
    BINT rows=MATROWS(*(matrix+1)),cols=MATCOLS(*(matrix+1));
    if(!rows) return matrix+matrix[row+col];
    return matrix+matrix[(row-1)*cols+col+1];
}

// FAST GET THE PTR TO AN ELEMENT IN A MATRIX EXPLODED IN THE STACK

WORDPTR *rplMatrixFastGetEx(WORDPTR *first,BINT cols,BINT i,BINT j)
{
    return first+(i-1)*cols+(j-1);
}


// CREATE A NEW MATRIX EXPLODED IN THE STACK, FILLED WITH ZEROS
// RETURN THE POINTER TO THE FIRST ELEMENT IN THE STACK

WORDPTR *rplMatrixNewEx(BINT rows,BINT cols)
{
    if(!rows) ++rows;

    if( (rows<1)||(cols<1)) return 0;

    WORDPTR *Firstelem=DSTop;
    BINT k,nelem;

    nelem=rows*cols;

    for(k=0;k<nelem;++k) {
        rplPushData(zero_bint);
        if(Exceptions) {
            DSTop=Firstelem;
            return 0;
        }

    }

    return Firstelem;
}



// EXPLODES A MATRIX IN THE STACK
// PUTS A POINTER TO THE MATRIX, THEN THE ELEMENTS IN ROW ORDER
// RETURNS A POINTER TO THE DATA STACK WHERE THE FIRST ELEMENT IS
// LEAVES THE ORIGINAL MATRIX POINTER IN THE STACK
WORDPTR *rplMatrixExplode()
{
    WORDPTR *matrix=DSTop-1;
    BINT rows=MATROWS(*(*matrix+1)),cols=MATCOLS(*(*matrix+1));
    if(!rows) ++rows;

    BINT k,nelem;

    nelem=rows*cols;

    for(k=0;k<nelem;++k) rplPushData(*matrix+(*matrix)[2+k]);

    if(Exceptions) {
        DSTop=matrix+1;
        return 0;
    }

    return matrix+1;
}

// COMPOSES A NEW MATRIX OBJECT FROM OBJECTS IN THE STACK
// OBJECTS MUST BE IN ROW-ORDER
// RETURNS 0 IF ERROR, AND SETS Exceptions AND ExceptionPtr.
// CREATES A VECTOR IF ROWS == 0, OTHERWISE A MATRIX

WORDPTR rplMatrixCompose(BINT rows,BINT cols)
{
BINT totalelements=(rows)? rows*cols:cols;

// CHECK IF ENOUGH ELEMENTS IN THE STACK
if(rplDepthData()<totalelements) {
    rplError(ERR_BADARGCOUNT);
    return 0;
}

if((rows<0) || (rows>65535) || (cols<1) || (cols>65535)) {
    rplError(ERR_INVALIDDIMENSION);
    return 0;
}

//   CHECK VALIDITY OF ALL ELEMENTS
BINT k,j,totalsize=0;
WORDPTR obj;

for(k=1;k<=totalelements;++k) {
obj=rplPeekData(k);
if(! (ISNUMBERCPLX(*obj)
      || ISSYMBOLIC(*obj)
      || ISIDENT(*obj))) {
    rplError(ERR_NOTALLOWEDINMATRIX);
    return 0;
    }
totalsize+=rplObjSize(obj);
}

WORDPTR matrix=rplAllocTempOb(totalsize+1+totalelements);
WORDPTR newobj=matrix+2+totalelements;  // POINT TO THE NEXT OBJECT TO STORE

if(!matrix) return 0;

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

// NO TYPE CHECK, THAT SHOULD BE DONE BY HIGHER LEVEL FUNCTIONS


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
                rplError(ERR_INCOMPATIBLEDIMENSION);
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
                    rplError(ERR_INCOMPATIBLEDIMENSION);
                    return;
                }
            }
        } else {
            rplError(ERR_INCOMPATIBLEDIMENSION);
            return;
        }

    }


}

// CHECK COLUMN SIZE
if(colsa!=colsb) {
    rplError(ERR_INCOMPATIBLEDIMENSION);
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
void rplMatrixAdd() { rplMatrixBinary((CMD_OVR_ADD)); }
void rplMatrixSub() { rplMatrixBinary((CMD_OVR_SUB)); }



void rplMatrixMulScalar()
{
WORDPTR *Savestk,*a,*b;
// DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
// AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
Savestk=DSTop;
a=DSTop-2;
b=DSTop-1;

// MAKE SURE THAT b IS THE SCALAR VALUE, a IS THE MATRIX

if(!ISMATRIX(**a)) {
    WORDPTR *tmp=a;
    a=b;
    b=tmp;
}

// CHECK DIMENSIONS

BINT rowsa=MATROWS(*(*a+1)),colsa=MATCOLS(*(*a+1));

BINT totalelements=(rowsa)? rowsa*colsa:colsa;

BINT j;

// DO THE ELEMENT-BY-ELEMENT OPERATION

for(j=0;j<totalelements;++j) {
 rplPushData(GETELEMENT(*a,j));
 rplPushData(*b);
 rplCallOvrOperator((CMD_OVR_MUL));
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


// APPLY UNARY OPERATOR THAT WORKS ELEMENT-BY-ELEMENT (NEG, EVAL, ->NUM, ETC)
void rplMatrixUnary(WORD Opcode)
{
WORDPTR *Savestk,*a;
// DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
// AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
Savestk=DSTop;
a=DSTop-1;

// a IS THE MATRIX

// CHECK DIMENSIONS

BINT rowsa=MATROWS(*(*a+1)),colsa=MATCOLS(*(*a+1));

BINT totalelements=(rowsa)? rowsa*colsa:colsa;

BINT j;

// DO THE ELEMENT-BY-ELEMENT OPERATION

for(j=0;j<totalelements;++j) {
 rplPushData(GETELEMENT(*a,j));
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
rplOverwriteData(1,newmat);
}



void rplMatrixNeg()
{
    rplMatrixUnary((CMD_OVR_NEG));
}






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

    // NO TYPE CHECK, DO THAT AT HIGHER LEVEL

    // CHECK DIMENSIONS

    BINT rowsa=MATROWS(*(*a+1)),colsa=MATCOLS(*(*a+1));
    BINT rowsb=MATROWS(*(*b+1)),colsb=MATCOLS(*(*b+1));
    BINT rrows,rcols;

    if(rowsa==0 && rowsb==0) {
        // VECTOR BY VECTOR MULTIPLICATION
        // DO A DOT PRODUCT
        if(colsa!=colsb) {
            rplError(ERR_INCOMPATIBLEDIMENSION);

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
                    rplError(ERR_INCOMPATIBLEDIMENSION);

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
                        rplError(ERR_INCOMPATIBLEDIMENSION);

                        return;
                    }



                }


            }
            else {
                // MATRIX x MATRIX MULTIPLICATION

                // NO AUTOTRANSPOSE HERE
                if(colsa!=rowsb) {
                    rplError(ERR_INCOMPATIBLEDIMENSION);

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
            rplCallOvrOperator((CMD_OVR_MUL));
            if(Exceptions) {
                // CLEAN UP THE STACK AND RETURN
                DSTop=Savestk;
                return;
            }
            if(k) {
                rplCallOvrOperator((CMD_OVR_ADD));
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



// LOW-LEVEL FRACTION-FREE GAUSS ELIMINATION
// WORKS ON MATRIX a EXPLODED IN THE STACK
// a POINTS TO THE MATRIX ON THE STACK, WITH ELEMENTS
// IMMEDIATELY AFTER
// RETURNS THE SAME MATRIX WITH ALTERED ELEMENTS
// EXPLODED IN THE STACK

void rplMatrixBareissEx(WORDPTR *a,BINT rowsa,BINT colsa)
{
    BINT i,j,k;

    /*
    Single step Bareiss
    bareiss := procedure(~M)
    n := NumberOfRows(M);
    for k in [1..n-1] do
    for i in [k+1..n] do
    for j in [k+1..n] do
    D := M[k][k]*M[i][j] - M[k][j]*M[i][k];
    M[i][j] := k eq 1 select D div 1
                        else D div M[k-1][k-1];
    end for;
    end for;
    end for;
    print M;
    end procedure;
    */


// CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
// a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*colsa+(c)]

    for(k=1;k<rowsa;++k) {
        for(i=k+1;i<=rowsa;++i) {
            for(j=k+1;j<=colsa;++j) {
                rplPushData(STACKELEM(k,k));  // M[k][k];
                rplPushData(STACKELEM(i,j));  // M[i][j];
                rplCallOvrOperator((CMD_OVR_MUL));
                if(Exceptions) return;
                rplPushData(STACKELEM(k,j));  // M[k][j];
                rplPushData(STACKELEM(i,k));  // M[i][k];
                rplCallOvrOperator((CMD_OVR_MUL));
                if(Exceptions) return;
                rplCallOvrOperator((CMD_OVR_SUB));
                if(Exceptions) return;
                if(k>1) {
                    rplPushData(STACKELEM(k-1,k-1));
                    rplCallOvrOperator((CMD_OVR_DIV));
                    if(Exceptions) return;
                }

                // SIMPLIFY IF IT'S A SYMBOLIC TO KEEP THE OBJECTS SMALL
                if(ISSYMBOLIC(*rplPeekData(1))) {
                    rplSymbAutoSimplify();
                    if(Exceptions) return;
                }

                // PUT THE ELEMENT IN ITS PLACE M[i][j]
                STACKELEM(i,j)=rplPopData();

            }
        }


    }


// IF WE GOT HERE WITHOUT DIVISION BY ZERO THEN THE MARIX IS INVERTIBLE
// FILL THE LOWER TRIANGULAR PORTION WITH ZEROS

for(i=2;i<=rowsa;i++) {
for(j=1;j<i;++j) {
    STACKELEM(i,j)=(WORDPTR)zero_bint;
}

}

#undef STACKELEM

}



// FRACTION-FREE GAUSSIAN ELIMINATION
// TAKES MATRIX ON THE STACK AND PERFORMS BAREISS ELIMINATION
// LEAVES NEW MATRIX ON THE STACK

void rplMatrixBareiss()
{
    WORDPTR *Savestk,*a;
    // DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
    // AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk=DSTop;
    a=DSTop-1;

    // NO TYPE CHECK, DO THAT AT HIGHER LEVEL

    // CHECK DIMENSIONS

    BINT rowsa=MATROWS(*(*a+1)),colsa=MATCOLS(*(*a+1));

    // SIZE CHECK, ONLY AUGMENTED SQUARE MATRICES ALLOWED
    if(rowsa>colsa) {
        rplError(ERR_INVALIDDIMENSION);
        return;
    }

    rplMatrixExplode();
    if(Exceptions) {
        DSTop=Savestk;
        return;
    }

    rplMatrixBareissEx(a,rowsa,colsa);
    if(Exceptions) {
        DSTop=Savestk;
        return;
    }

    WORDPTR newmat=rplMatrixCompose(rowsa,colsa);
    DSTop=Savestk;
    if(Exceptions) return;
    rplOverwriteData(1,newmat);

}


// BACK SUBSTITUTION ON MATRIX AFTER GAUSS ELIMINATION
// WORKS ON MATRIX a EXPLODED IN THE STACK
// a POINTS TO THE MATRIX ON THE STACK, WITH ELEMENTS
// IMMEDIATELY AFTER
// RETURNS THE SAME MATRIX WITH ALTERED ELEMENTS
// EXPLODED IN THE STACK

void rplMatrixBackSubstEx(WORDPTR *a,BINT rowsa,BINT colsa)
{
    BINT i,j,k;

// CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
// a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*colsa+(c)]

        for(i=rowsa;i>=1;--i) {
            for(j=colsa;j>=i;--j) {
                rplPushData(STACKELEM(i,j));
                rplPushData(STACKELEM(i,i));
                rplCallOvrOperator((CMD_OVR_DIV));
                if(Exceptions) return;
                // SIMPLIFY IF IT'S A SYMBOLIC TO KEEP THE OBJECTS SMALL
                if(ISSYMBOLIC(*rplPeekData(1))) {
                    rplSymbAutoSimplify();
                    if(Exceptions) return;
                }

                // PUT THE ELEMENT IN ITS PLACE
                STACKELEM(i,j)=rplPopData();
            }
            for(k=i-1;k>=1;--k) {
                for(j=colsa;j>=i;--j) {
                    rplPushData(STACKELEM(k,j));
                    rplPushData(STACKELEM(k,i));
                    rplPushData(STACKELEM(i,j));
                    rplCallOvrOperator((CMD_OVR_MUL));
                    if(Exceptions) return;
                    rplCallOvrOperator((CMD_OVR_SUB));
                    if(Exceptions) return;

                    // SIMPLIFY IF IT'S A SYMBOLIC TO KEEP THE OBJECTS SMALL
                    if(ISSYMBOLIC(*rplPeekData(1))) {
                        rplSymbAutoSimplify();
                        if(Exceptions) return;
                    }

                    // PUT THE ELEMENT IN ITS PLACE
                    STACKELEM(k,j)=rplPopData();
                }

            }
        }

// FILL THE UPPER DIAGONAL WITH ZEROS
for(i=1;i<rowsa;++i) {
    for(j=i+1;j<=rowsa;++j) STACKELEM(i,j)=(WORDPTR)zero_bint;
}


#undef STACKELEM


}

// TAKE A SQUARE MATRIX FROM THE STACK, AUGMENT IT BY THE IDENTITY MATRIX
// AND INVERT IT

void rplMatrixInvert()
{
    WORDPTR *Savestk,*a;
    // DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
    // AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk=DSTop;
    a=DSTop-1;

    // NO TYPE CHECK, DO THAT AT HIGHER LEVEL

    // CHECK DIMENSIONS

    BINT rowsa=MATROWS(*(*a+1)),colsa=MATCOLS(*(*a+1));

    // SIZE CHECK, ONLY SQUARE MATRICES ALLOWED
    if(rowsa!=colsa) {
        rplError(ERR_INVALIDDIMENSION);
        return;
    }

    rplMatrixExplode();
    if(Exceptions) {
        DSTop=Savestk;
        return;
    }


    // AUGMENT WITH IDENTITY MATRIX

    rplExpandStack(rowsa*colsa);
    if(Exceptions) {
        DSTop=Savestk;
        return;
    }

    // WE HAVE THE SPACE IN THE STACK, EXPAND THE ROWS

    BINT i,j;
    // CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
    // a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
    #define STACKELEM(r,c) a[((r)-1)*colsa+(c)]
    #define NEWSTACKELEM(r,c) a[((r)-1)*(colsa<<1)+(c)]

    // SPREAD THE ROWS
    for(i=rowsa;i>=1;--i) {
        for(j=colsa;j>=1;--j) NEWSTACKELEM(i,j)=STACKELEM(i,j);
    }

    // AND WRITE AN IDENTITY MATRIX
    for(i=rowsa;i>=1;--i) {
        for(j=colsa<<1;j>colsa;--j) NEWSTACKELEM(i,j)=(WORDPTR) ((i==j-colsa)? one_bint:zero_bint);
    }

    DSTop=&(NEWSTACKELEM(rowsa,colsa<<1))+1;

   #undef NEWSTACKELEM
   #undef STACKELEM

    rplMatrixBareissEx(a,rowsa,colsa<<1);
    if(Exceptions) {
        DSTop=Savestk;
        return;
    }

    rplMatrixBackSubstEx(a,rowsa,colsa<<1);
    if(Exceptions) {
        DSTop=Savestk;
        return;
    }


    // REMOVE THE ORIGINAL MATRIX TO OBTAIN THE INVERSE ONLY

    // CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
    // a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
    #define STACKELEM(r,c) a[((r)-1)*colsa+(c)]
    #define NEWSTACKELEM(r,c) a[((r)-1)*(colsa<<1)+(c)]


    // RE-PACK THE ROWS
    for(i=1;i<=rowsa;++i) {
        for(j=1;j<=colsa;++j) STACKELEM(i,j)=NEWSTACKELEM(i,j+colsa);
    }

    // TRIM THE STACK
    DSTop=&(STACKELEM(rowsa,colsa))+1;

    // AND COMPOSE A NEW MATRIX
    WORDPTR newmat=rplMatrixCompose(rowsa,colsa);
    DSTop=Savestk;
    if(Exceptions) return;
    rplOverwriteData(1,newmat);

}



// TAKE A MATRIX FROM THE STACK AND RETURN THE FROBENIUS NORM

void rplMatrixNorm()
{
WORDPTR *Savestk,*a;
// DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
// AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
Savestk=DSTop;
a=DSTop-1;

// NO TYPE CHECK, THAT SHOULD BE DONE BY HIGHER LEVEL FUNCTIONS


// CHECK DIMENSIONS

BINT rowsa=MATROWS(*(*a+1)),colsa=MATCOLS(*(*a+1));

if(!rowsa) rowsa=1;


BINT i,j;
rplPushData((WORDPTR)zero_bint);
// DO THE ELEMENT-BY-ELEMENT OPERATION
for(i=0;i<rowsa;++i) {
for(j=0;j<colsa;++j) {
 rplPushData(GETELEMENT(*a,i*colsa+j));
 rplCallOvrOperator((CMD_OVR_ABS));
 if(Exceptions) {
     DSTop=Savestk;
     return;
 }
 rplPushData(rplPeekData(1));   // DUP
 rplCallOvrOperator((CMD_OVR_MUL));
 if(Exceptions) {
     DSTop=Savestk;
     return;
 }
 rplCallOvrOperator((CMD_OVR_ADD));
 if(Exceptions) {
     DSTop=Savestk;
     return;
 }

}

// SIMPLIFY IF IT'S A SYMBOLIC TO KEEP THE OBJECTS SMALL
if(ISSYMBOLIC(*rplPeekData(1))) {
    rplSymbAutoSimplify();
    if(Exceptions) {
        DSTop=Savestk;
        return;
    }
}

}

// COMPUTE THE SQUARE ROOT
rplCallOperator(CMD_SQRT);
if(Exceptions) {
    DSTop=Savestk;
    return;
}


rplOverwriteData(2,rplPeekData(1));
rplDropData(1);
}



