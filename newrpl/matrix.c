/*
 * Copyright (c) 2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "cmdcodes.h"
#include "libraries.h"
#include "ui.h"

// FAST MACRO TO GET A POINTER TO AN ELEMENT
// INTERNAL USE ONLY
#define GETELEMENT(matrix,index) ((matrix)+(matrix)[2+(index)])

// GET A POINTER TO AN OBJECT WITHIN THE MATRIX/VECTOR
// RETURN 0 ON OUT-OF-RANGE
// VECTORS ARE AUTO-ROTATED
// ROWS AND COLUMNS ARE 1-BASED

WORDPTR rplMatrixGet(WORDPTR matrix, BINT row, BINT col)
{
    if(!ISMATRIX(*matrix))
        return 0;
    BINT rows = MATROWS(*(matrix + 1)), cols = MATCOLS(*(matrix + 1));

    if(!rows) {
        // THIS IS A VECTOR
        if(row == 1) {
            if(col > 0 && col <= cols)
                return matrix + matrix[col + 1];
            else
                return 0;
        }
        else if(col == 1) {
            if(row > 0 && row <= cols)
                return matrix + matrix[row + 1];
            else
                return 0;
        }
        else
            return 0;
    }

    if(row < 1 || row > rows)
        return 0;
    if(col < 1 || col > cols)
        return 0;

    return matrix + matrix[(row - 1) * cols + col + 1];

}

// RETURN THE NUMBER OF COLUMNS IN THE MATRIX - NO SAFETY CHECKS
BINT rplMatrixCols(WORDPTR matrix)
{
    return MATCOLS(*(matrix + 1));
}

// RETURN THE NUMBER OF COLUMNS IN THE MATRIX - NO SAFETY CHECKS
// ROWS=0 MEANS A VECTOR WITH cols ELEMENTS
BINT rplMatrixRows(WORDPTR matrix)
{
    return MATROWS(*(matrix + 1));
}

// GET A POINTER TO THE FIRST OBJECT WITHIN A MATRIX

WORDPTR rplMatrixGetFirstObj(WORDPTR matrix)
{
    BINT rows = MATROWS(*(matrix + 1)), cols = MATCOLS(*(matrix + 1));
    if(!rows)
        rows = 1;
    return matrix + 2 + rows * cols;
}

// GET AN ELEMENT OF AN ARRAY - LOW-LEVEL, NO CHECKS OF ANY KIND DONE

WORDPTR rplMatrixFastGet(WORDPTR matrix, BINT row, BINT col)
{
    BINT rows = MATROWS(*(matrix + 1)), cols = MATCOLS(*(matrix + 1));
    if(!rows)
        return matrix + matrix[row + col];
    return matrix + matrix[(row - 1) * cols + col + 1];
}

// GET AN ELEMENT OF AN ARRAY - LOW-LEVEL, NO CHECKS OF ANY KIND DONE - FLAT INDEX 0..(n-1)

WORDPTR rplMatrixFastGetFlat(WORDPTR matrix, BINT index)
{
    return GETELEMENT(matrix, index);
}

// FAST GET THE PTR TO AN ELEMENT IN A MATRIX EXPLODED IN THE STACK

WORDPTR *rplMatrixFastGetEx(WORDPTR * first, BINT cols, BINT i, BINT j)
{
    return first + (i - 1) * cols + (j - 1);
}

// CREATE A NEW MATRIX EXPLODED IN THE STACK, FILLED WITH ZEROS
// RETURN THE POINTER TO THE FIRST ELEMENT IN THE STACK

WORDPTR *rplMatrixNewEx(BINT rows, BINT cols)
{
    if(!rows)
        ++rows;

    if((rows < 1) || (cols < 1))
        return 0;

    WORDPTR *Firstelem = DSTop;
    BINT k, nelem;

    nelem = rows * cols;

    for(k = 0; k < nelem; ++k) {
        rplPushData((WORDPTR) zero_bint);
        if(Exceptions) {
            DSTop = Firstelem;
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
    WORDPTR *matrix = DSTop - 1;
    BINT rows = MATROWS(*(*matrix + 1)), cols = MATCOLS(*(*matrix + 1));
    if(!rows)
        ++rows;

    BINT k, nelem;

    nelem = rows * cols;

    for(k = 0; k < nelem; ++k)
        rplPushData(*matrix + (*matrix)[2 + k]);

    if(Exceptions) {
        DSTop = matrix + 1;
        return 0;
    }

    return matrix + 1;
}

// EXPLODES A MATRIX IN THE STACK TRANSPOSED
// PUTS A POINTER TO THE MATRIX, THEN THE ELEMENTS IN COLUMN ORDER
// RETURNS A POINTER TO THE DATA STACK WHERE THE FIRST ELEMENT IS
// LEAVES THE ORIGINAL MATRIX POINTER IN THE STACK
WORDPTR *rplMatrixExplodeByCols()
{
    WORDPTR *matrix = DSTop - 1;
    BINT rows = MATROWS(*(*matrix + 1)), cols = MATCOLS(*(*matrix + 1));
    if(!rows)
        return rplMatrixExplode();

    BINT i, j;

    for(i = 1; i <= cols; ++i) {
        for(j = 1; j <= rows; ++j) {
            rplPushData(*matrix + (*matrix)[(j - 1) * cols + i + 1]);
            if(Exceptions) {
                DSTop = matrix + 1;
                return 0;
            }
        }
    }
    return matrix + 1;
}

// COMPOSES A NEW MATRIX OBJECT FROM OBJECTS IN THE STACK STARTING AT level
// OBJECTS MUST BE IN ROW-ORDER, level IS THE LAST OBJECT IN THE MATRIX (LOWEST NUMBER)
// RETURNS 0 IF ERROR, AND SETS Exceptions AND ExceptionPtr.
// CREATES A VECTOR IF ROWS == 0, OTHERWISE A MATRIX

WORDPTR rplMatrixComposeN(BINT level, BINT rows, BINT cols)
{
    BINT totalelements = (rows) ? rows * cols : cols;

// CHECK IF ENOUGH ELEMENTS IN THE STACK
    if(rplDepthData() < level + totalelements - 1) {
        rplError(ERR_BADARGCOUNT);
        return 0;
    }

    if((rows < 0) || (rows > 65535) || (cols < 1) || (cols > 65535)) {
        rplError(ERR_INVALIDDIMENSION);
        return 0;
    }

//   CHECK VALIDITY OF ALL ELEMENTS
    BINT k, j, totalsize = 0;
    WORDPTR obj;

    for(k = level; k < level + totalelements; ++k) {
        obj = rplPeekData(k);
        if(!(ISNUMBERCPLX(*obj)
                    || ISSYMBOLIC(*obj)
                    || ISIDENT(*obj)
                    || ISANGLE(*obj)
                )) {
            rplError(ERR_NOTALLOWEDINMATRIX);
            return 0;
        }
        totalsize += rplObjSize(obj);
    }

    WORDPTR matrix = rplAllocTempOb(totalsize + 1 + totalelements);
    WORDPTR newobj = matrix + 2 + totalelements;        // POINT TO THE NEXT OBJECT TO STORE

    if(!matrix)
        return 0;

// FINALLY, ASSEMBLE THE OBJECT
    for(k = 0; k < totalelements; ++k) {
        obj = rplPeekData(level - 1 + totalelements - k);
        for(j = 0; j < k; ++j)
            if(rplCompareObjects(obj,
                        rplPeekData(level - 1 + totalelements - j)))
                break;
        if(j != k) {
            // ADD THE ORIGINAL OBJECT
            matrix[2 + k] = matrix[2 + j];
        }
        else {
            // ADD A NEW OBJECT
            matrix[2 + k] = newobj - matrix;
            rplCopyObject(newobj, obj);
            newobj = rplSkipOb(newobj);
        }
    }

    rplTruncateLastObject(newobj);

    matrix[0] = MKPROLOG(DOMATRIX, newobj - matrix - 1);
    matrix[1] = MATMKSIZE(rows, cols);

    return matrix;

}

WORDPTR rplMatrixCompose(BINT rows, BINT cols)
{
    return rplMatrixComposeN(1, rows, cols);
}

// APPLIES ANY OVERLOADABLE BINARY OPERATOR THAT WORKS ELEMENT-BY-ELEMENT (ADD/SUBTRACT)

void rplMatrixBinary(WORD Opcode)
{
    WORDPTR *Savestk, *a, *b;
// DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
// AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk = DSTop;
    a = DSTop - 2;
    b = DSTop - 1;

// NO TYPE CHECK, THAT SHOULD BE DONE BY HIGHER LEVEL FUNCTIONS

// CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(*a + 1)), colsa = MATCOLS(* (*a + 1));
    BINT rowsb = MATROWS(*(*b + 1)), colsb = MATCOLS(* (*b + 1));

    if(rowsa != rowsb) {
        // CHECK IF ONE OF THEM IS A VECTOR AND AUTOTRANSPOSE
        if(!rowsa) {
            if(rowsb == 1)
                rowsa = 1;
            else {
                // TRANSPOSE A AND TRY
                rowsa = colsa;
                colsa = 1;
                if(rowsa != rowsb) {
                    rplError(ERR_INCOMPATIBLEDIMENSION);
                    return;
                }
            }
        }
        else {
            if(!rowsb) {
                if(rowsa == 1)
                    rowsb = 1;
                else {
                    // TRANSPOSE B AND TRY
                    rowsb = colsb;
                    colsb = 1;
                    if(rowsa != rowsb) {
                        rplError(ERR_INCOMPATIBLEDIMENSION);
                        return;
                    }
                }
            }
            else {
                rplError(ERR_INCOMPATIBLEDIMENSION);
                return;
            }

        }

    }

// CHECK COLUMN SIZE
    if(colsa != colsb) {
        rplError(ERR_INCOMPATIBLEDIMENSION);
        return;
    }

// HERE WE HAVE COMPATIBLE SIZE VECTOR/MATRIX

    BINT totalelements = (rowsa) ? rowsa * colsa : colsa;

    BINT j;

// DO THE ELEMENT-BY-ELEMENT OPERATION

    for(j = 0; j < totalelements; ++j) {
        rplPushData(GETELEMENT(*a, j));
        rplPushData(GETELEMENT(*b, j));
        rplCallOperator(Opcode);
        if(Exceptions) {
            DSTop = Savestk;
            return;
        }
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions) {
                DSTop = Savestk;
                return;
            }
        }

    }

    WORDPTR newmat = rplMatrixCompose(rowsa, colsa);
    DSTop = Savestk;
    if(!newmat)
        return;
    rplOverwriteData(2, newmat);
    rplDropData(1);
}

// ADD/SUBTRACT TWO MATRICES, A+B, WITH A IN LEVEL 2 AND B IN LEVEL 1 OF THE STACK
void rplMatrixAdd()
{
    rplMatrixBinary((CMD_OVR_ADD));
}

void rplMatrixSub()
{
    rplMatrixBinary((CMD_OVR_SUB));
}

void rplMatrixHadamard()
{
    rplMatrixBinary((CMD_OVR_MUL));
}

void rplMatrixMulScalar()
{
    WORDPTR *Savestk, *a, *b;
// DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
// AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk = DSTop;
    a = DSTop - 2;
    b = DSTop - 1;

// MAKE SURE THAT b IS THE SCALAR VALUE, a IS THE MATRIX

    if(!ISMATRIX(**a)) {
        WORDPTR *tmp = a;
        a = b;
        b = tmp;
    }

// CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(*a + 1)), colsa = MATCOLS(*(*a + 1));

    BINT totalelements = (rowsa) ? rowsa * colsa : colsa;

    BINT j;

// DO THE ELEMENT-BY-ELEMENT OPERATION

    for(j = 0; j < totalelements; ++j) {
        rplPushData(GETELEMENT(*a, j));
        rplPushData(*b);
        rplCallOvrOperator((CMD_OVR_MUL));
        if(Exceptions) {
            DSTop = Savestk;
            return;
        }
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions) {
                DSTop = Savestk;
                return;
            }
        }

    }

    WORDPTR newmat = rplMatrixCompose(rowsa, colsa);
    DSTop = Savestk;
    if(!newmat)
        return;
    rplOverwriteData(2, newmat);
    rplDropData(1);
}

void rplMatrixDivScalar()
{
    WORDPTR *Savestk, *a, *b;
// DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
// AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk = DSTop;
    a = DSTop - 2;
    b = DSTop - 1;

// MAKE SURE THAT b IS THE SCALAR VALUE, a IS THE MATRIX

    if(!ISMATRIX(**a)) {
        WORDPTR *tmp = a;
        a = b;
        b = tmp;
    }

// CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(*a + 1)), colsa = MATCOLS(*(*a + 1));

    BINT totalelements = (rowsa) ? rowsa * colsa : colsa;

    BINT j;

// DO THE ELEMENT-BY-ELEMENT OPERATION

    for(j = 0; j < totalelements; ++j) {
        rplPushData(GETELEMENT(*a, j));
        rplPushData(*b);
        rplCallOvrOperator((CMD_OVR_DIV));
        if(Exceptions) {
            DSTop = Savestk;
            return;
        }
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions) {
                DSTop = Savestk;
                return;
            }
        }

    }

    WORDPTR newmat = rplMatrixCompose(rowsa, colsa);
    DSTop = Savestk;
    if(!newmat)
        return;
    rplOverwriteData(2, newmat);
    rplDropData(1);
}

// APPLY UNARY OPERATOR THAT WORKS ELEMENT-BY-ELEMENT (NEG, EVAL, ->NUM, ETC)
void rplMatrixUnary(WORD Opcode)
{
    WORDPTR *Savestk, *a;
// DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
// AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk = DSTop;
    a = DSTop - 1;

// a IS THE MATRIX

// CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(*a + 1)), colsa = MATCOLS(* (*a + 1));

    BINT totalelements = (rowsa) ? rowsa * colsa : colsa;

    BINT j;

// DO THE ELEMENT-BY-ELEMENT OPERATION

    for(j = 0; j < totalelements; ++j) {
        rplPushData(GETELEMENT(*a, j));
        rplCallOperator(Opcode);
        if(Exceptions) {
            DSTop = Savestk;
            return;
        }
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions) {
                DSTop = Savestk;
                return;
            }
        }

    }

    WORDPTR newmat = rplMatrixCompose(rowsa, colsa);
    DSTop = Savestk;
    if(!newmat)
        return;
    rplOverwriteData(1, newmat);
}

void rplMatrixNeg()
{
    rplMatrixUnary((CMD_OVR_NEG));
}

void rplMatrixConj()
{
    rplMatrixUnary((CMD_CONJ));
}

// MATRIX BY MATRIX MULTIPLICATION
// DOES A*B WITH A ON LEVEL 2 AND B ON LEVEL 1
// DOES AUTOTRANSPOSE OF VECTORS

void rplMatrixMul()
{

    WORDPTR *Savestk, *a, *b;
    // DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
    // AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk = DSTop;
    a = DSTop - 2;
    b = DSTop - 1;

    // NO TYPE CHECK, DO THAT AT HIGHER LEVEL

    // CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(*a + 1)), colsa = MATCOLS(* (*a + 1));
    BINT rowsb = MATROWS(*(*b + 1)), colsb = MATCOLS(* (*b + 1));
    BINT rrows, rcols;

    if(rowsa == 0 && rowsb == 0) {
        // VECTOR BY VECTOR MULTIPLICATION
        // DO A DOT PRODUCT
        if(colsa != colsb) {
            rplError(ERR_INCOMPATIBLEDIMENSION);

            return;
        }

        // DIMENSION OF THE RESULTING OUTPUT
        rowsa = 1;
        rowsb = colsb;
        colsb = 1;

        // THIS IS A SCALAR VALUE, NOT A MATRIX
        rrows = 0;
        rcols = 1;
    }
    else {

        if(!rowsa) {
            // VECTOR x MATRIX = VECTOR

            if(colsa == rowsb) {
                // USE IT AS ROW VECTOR (1xN)x(NxM)
                rowsa = 1;
                // RESULT IS A VECTOR
                rrows = 0;
                rcols = colsb;

            }
            else {
                if(rowsb == 1) {
                    // TRANSPOSE THE VECTOR (Nx1)x(1xM)=(NxM)
                    rowsa = colsa;
                    colsa = 1;
                    // RESULTING ELEMENT IS A MATRIX
                    rrows = rowsa;
                    rcols = colsb;
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
                if(colsa == colsb) {
                    // TRANSPOSE THE VECTOR (NxM)x(Mx1)
                    rowsb = colsb;
                    colsb = 1;

                    // RESULT IS A VECTOR
                    rrows = 0;
                    rcols = rowsa;

                }
                else {
                    if(colsa == 1) {
                        // (Nx1)x(1xM)
                        rowsb = 1;

                        // RESULT IS A MATRIX
                        rrows = rowsa;
                        rcols = colsb;
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
                if(colsa != rowsb) {
                    rplError(ERR_INCOMPATIBLEDIMENSION);

                    return;
                }

                rrows = rowsa;
                rcols = colsb;

            }

        }

    }

    // HERE WE HAVE PROPER DIMENSIONS TO DO THE MULTIPLICATION

    BINT i, j, k;

    for(i = 0; i < rowsa; ++i) {

        for(j = 0; j < colsb; ++j) {
            for(k = 0; k < colsa; ++k) {
                rplPushData(GETELEMENT(*a, i * colsa + k));
                rplPushData(GETELEMENT(*b, k * colsb + j));
                rplCallOvrOperator((CMD_OVR_MUL));
                if(Exceptions) {
                    // CLEAN UP THE STACK AND RETURN
                    DSTop = Savestk;
                    return;
                }
                if(k) {
                    rplCallOvrOperator((CMD_OVR_ADD));
                    if(Exceptions) {
                        // CLEAN UP THE STACK AND RETURN
                        DSTop = Savestk;
                        return;
                    }
                }
            }
            if(ISSYMBOLIC(*rplPeekData(1))) {
                rplSymbAutoSimplify();
                if(Exceptions) {
                    DSTop = Savestk;
                    return;
                }
            }

        }

    }

    // HERE WE HAVE ALL THE ELEMENTS IN ROW ORDER
    WORDPTR newmat;
    if(rrows + rcols > 1) {
        newmat = rplMatrixCompose(rrows, rcols);
        if(!newmat) {
            DSTop = Savestk;
            return;
        }
    }
    else
        newmat = rplPopData();

    DSTop = Savestk;
    rplOverwriteData(2, newmat);
    rplDropData(1);

}

// LOW-LEVEL FRACTION-FREE GAUSS ELIMINATION
// WORKS ON MATRIX a EXPLODED IN THE STACK
// a POINTS TO THE MATRIX ON THE STACK, WITH ELEMENTS
// IMMEDIATELY AFTER
// RETURNS THE SAME MATRIX WITH ALTERED ELEMENTS
// EXPLODED IN THE STACK
// RETURNS FALSE IF MATRIX IS SINGULAR OR THERE WAS AN ERROR, TRUE OTHERWISE

BINT rplMatrixBareissEx(WORDPTR * a, WORDPTR * index, BINT rowsa, BINT colsa,
        BINT upperonly)
{
    BINT i, j, k, q, startrow = 1;

    /*
       Single step Bareiss
       bareiss := procedure(~M)
       n := NumberOfRows(M);
       for k in [1..n-1] do
       for i in [k+1..n] do
       q=i+1
       for j in [k+1..n] do

       D := M[k][k]*M[i][j] - M[k][j]*M[i][k];

       if i==j==k+1 && D==0 then
       if q>n --> Error: Singular matrix
       swap row i and q
       q=q+1
       recompute D

       endif;

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

    // ELEMENT 1,1 MUST BE NON-ZERO

    if(rplSymbIsZero(STACKELEM(1, 1))) {

        q = 2;
        while((q <= rowsa) && rplSymbIsZero(STACKELEM(q, 1)))
            ++q;
        if(q > rowsa) {
            // ENTIRE FIRST COLUMN IS FULL OF ZEROS...
            startrow = 2;
        }
        else {
            // ZERO ELEMENT IN THE DIAGONAL, TRY SWAPPING ROWS
            int s;
            WORDPTR tmp;
            for(s = 1; s <= colsa; ++s) {
                tmp = STACKELEM(1, s);
                STACKELEM(1, s) = STACKELEM(q, s);
                STACKELEM(q, s) = tmp;
            }
            if(index) {
                // NOW UPDATE THE INDEX
                s = (*index)[1];
                (*index)[1] = (*index)[q];
                (*index)[q] = s;
            }
            ++q;
        }
    }

    for(k = startrow; k < rowsa; ++k) {
        for(i = k + 1; i <= rowsa; ++i) {
            q = i + 1;
            for(j = k + 1; j <= colsa; ++j) {

                do {

                    rplPushData(STACKELEM(k, k));       // M[k][k];
                    rplPushData(STACKELEM(i, j));       // M[i][j];
                    rplCallOvrOperator((CMD_OVR_MUL));
                    if(Exceptions)
                        return 0;
                    rplPushData(STACKELEM(k, j));       // M[k][j];
                    rplPushData(STACKELEM(i, k));       // M[i][k];
                    rplCallOvrOperator((CMD_OVR_MUL));
                    if(Exceptions)
                        return 0;
                    rplCallOvrOperator((CMD_OVR_SUB));
                    if(Exceptions)
                        return 0;

                    // HERE WE HAVE D, NOW CHECK IF IT'S ZERO
                    // USES rplSymbIsZero WHICH EXPLICITLY IS AN ATOMIC OPERATION
                    // AND DOESN'T NEED THE RPL LOOP
                    if(ISSYMBOLIC(*rplPeekData(1))) {
                        rplSymbAutoSimplify();
                        if(Exceptions)
                            return 0;
                    }

                    if((i == k + 1) && (j == k + 1)
                            && rplSymbIsZero(rplPeekData(1))) {
                        if(q > rowsa) {
                            startrow = k + 1;
                            break;
                            // rplError(ERR_SINGULARMATRIX);
                            //return;
                        }
                        else {
                            // ZERO ELEMENT IN THE DIAGONAL, TRY SWAPPING ROWS
                            int s;
                            WORDPTR tmp;
                            for(s = k; s <= colsa; ++s) {
                                tmp = STACKELEM(i, s);
                                STACKELEM(i, s) = STACKELEM(q, s);
                                STACKELEM(q, s) = tmp;
                            }
                            if(index) {
                                // NOW UPDATE THE INDEX
                                s = (*index)[i];
                                (*index)[i] = (*index)[q];
                                (*index)[q] = s;
                            }
                            ++q;

                            // TRY AGAIN WITH THE NEW ROW
                            rplDropData(1);
                            continue;
                        }

                    }
                    break;      // ALWAYS BREAK OUT OF THE LOOP, UNLESS WE HAVE A ZERO IN THE DIAGONAL
                }
                while(1);

                if(k > startrow) {
                    rplPushData(STACKELEM(k - 1, k - 1));
                    rplCallOvrOperator((CMD_OVR_DIV));
                    if(Exceptions)
                        return 0;
                }

                // SIMPLIFY IF IT'S A SYMBOLIC TO KEEP THE OBJECTS SMALL
                if(ISSYMBOLIC(*rplPeekData(1))) {
                    rplSymbAutoSimplify();
                    if(Exceptions)
                        return 0;
                }

                // PUT THE ELEMENT IN ITS PLACE M[i][j]
                STACKELEM(i, j) = rplPopData();

            }
        }

    }

    if(upperonly) {
// IF WE GOT HERE WITHOUT DIVISION BY ZERO THEN THE MARIX IS INVERTIBLE
// FILL THE LOWER TRIANGULAR PORTION WITH ZEROS

        for(i = 2; i <= rowsa; i++) {
            for(j = 1; j < i; ++j) {
                STACKELEM(i, j) = (WORDPTR) zero_bint;
            }

        }

    }

#undef STACKELEM
    return (startrow == 1) ? 1 : 0;
}

// FRACTION-FREE GAUSSIAN ELIMINATION
// TAKES MATRIX ON THE STACK AND PERFORMS BAREISS ELIMINATION
// LEAVES NEW MATRIX ON THE STACK

void rplMatrixReduce()
{
    WORDPTR *Savestk, *a;
    // DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
    // AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk = DSTop;
    a = DSTop - 1;

    // NO TYPE CHECK, DO THAT AT HIGHER LEVEL

    // CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(*a + 1)), colsa = MATCOLS(* (*a + 1));

    // SIZE CHECK, ONLY AUGMENTED SQUARE MATRICES ALLOWED
    if(rowsa > colsa) {
        rplError(ERR_INVALIDDIMENSION);
        return;
    }

    /*
       // RESERVE SPACE FOR THE ROW INDEX LIST

       WORDPTR IdxList=rplAllocTempOb(rowsa);

       if(!IdxList) {
       return;
       }

       IdxList[0]=MKPROLOG(DOBINDATA,rowsa);
       rplPushDataNoGrow(IdxList);
       rplPushDataNoGrow(*a);
       idx=a+1;
       a+=2;
     */
    rplMatrixExplode();
    if(Exceptions) {
        DSTop = Savestk;
        return;
    }

    rplMatrixBareissEx(a, 0, rowsa, colsa, 1);
    if(Exceptions) {
        DSTop = Savestk;
        return;
    }

    rplMatrixBackSubstEx(a, rowsa, colsa);
    if(Exceptions) {
        DSTop = Savestk;
        return;
    }

    WORDPTR newmat = rplMatrixCompose(rowsa, colsa);
    DSTop = Savestk;
    if(Exceptions)
        return;
    rplOverwriteData(1, newmat);

    // LEAVES THE INTERNAL PERMUTATION VECTOR ON LEVEL 2 AND

}

// BACK SUBSTITUTION ON MATRIX AFTER GAUSS ELIMINATION
// WORKS ON MATRIX a EXPLODED IN THE STACK
// a POINTS TO THE MATRIX ON THE STACK, WITH ELEMENTS
// IMMEDIATELY AFTER
// RETURNS THE SAME MATRIX WITH ALTERED ELEMENTS
// EXPLODED IN THE STACK

void rplMatrixBackSubstEx(WORDPTR * a, BINT rowsa, BINT colsa)
{
    BINT i, j, k, l;

// CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
// a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*colsa+(c)]

    for(i = rowsa; i >= 1; --i) {
        for(k = i; k <= colsa; ++k) {
            if(!rplSymbIsZero(STACKELEM(i, k)))
                break;
        }
        if(k <= colsa) {
            // HERE k HAS THE FIRST NON-ZERO COLUMN ELEMENT FOR THIS ROW
            for(j = colsa; j >= k; --j) {
                rplPushData(STACKELEM(i, j));
                rplPushData(STACKELEM(i, k));
                rplCallOvrOperator((CMD_OVR_DIV));
                if(Exceptions)
                    return;
                // SIMPLIFY IF IT'S A SYMBOLIC TO KEEP THE OBJECTS SMALL
                if(ISSYMBOLIC(*rplPeekData(1))) {
                    rplSymbAutoSimplify();
                    if(Exceptions)
                        return;
                }

                // PUT THE ELEMENT IN ITS PLACE
                STACKELEM(i, j) = rplPopData();
            }

            for(l = i - 1; l >= 1; --l) {
                for(j = colsa; j > k; --j) {
                    rplPushData(STACKELEM(l, j));
                    rplPushData(STACKELEM(l, k));
                    rplPushData(STACKELEM(i, j));
                    rplCallOvrOperator((CMD_OVR_MUL));
                    if(Exceptions)
                        return;
                    rplCallOvrOperator((CMD_OVR_SUB));
                    if(Exceptions)
                        return;

                    // SIMPLIFY IF IT'S A SYMBOLIC TO KEEP THE OBJECTS SMALL
                    if(ISSYMBOLIC(*rplPeekData(1))) {
                        rplSymbAutoSimplify();
                        if(Exceptions)
                            return;
                    }

                    // PUT THE ELEMENT IN ITS PLACE
                    STACKELEM(l, j) = rplPopData();
                }
                STACKELEM(l, k) = (WORDPTR) zero_bint;
            }
        }

    }

#undef STACKELEM

}

// TAKE A SQUARE MATRIX FROM THE STACK, AUGMENT IT BY THE IDENTITY MATRIX
// AND INVERT IT

void rplMatrixInvert()
{
    WORDPTR *Savestk, *a;
    // DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
    // AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk = DSTop;
    a = DSTop - 1;

    // NO TYPE CHECK, DO THAT AT HIGHER LEVEL

    // CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(*a + 1)), colsa = MATCOLS(* (*a + 1));

    // SIZE CHECK, ONLY SQUARE MATRICES ALLOWED
    if(rowsa != colsa) {
        rplError(ERR_INVALIDDIMENSION);
        return;
    }

    /*
       // RESERVE SPACE FOR THE ROW INDEX LIST

       WORDPTR IdxList=rplAllocTempOb(rowsa);

       if(!IdxList) {
       return;
       }

       IdxList[0]=MKPROLOG(DOBINDATA,rowsa);
       rplPushDataNoGrow(IdxList);
       rplPushDataNoGrow(*a);
       idx=a+1;
       a+=2;
     */
    rplMatrixExplode();
    if(Exceptions) {
        DSTop = Savestk;
        return;
    }

    // AUGMENT WITH IDENTITY MATRIX

    rplExpandStack(rowsa * colsa);
    if(Exceptions) {
        DSTop = Savestk;
        return;
    }

    // WE HAVE THE SPACE IN THE STACK, EXPAND THE ROWS

    BINT i, j;
    // CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
    // a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*colsa+(c)]
#define NEWSTACKELEM(r,c) a[((r)-1)*(colsa<<1)+(c)]

    // SPREAD THE ROWS
    for(i = rowsa; i >= 1; --i) {
        for(j = colsa; j >= 1; --j)
            NEWSTACKELEM(i, j) = STACKELEM(i, j);
    }

    // AND WRITE AN IDENTITY MATRIX
    for(i = rowsa; i >= 1; --i) {
        for(j = colsa << 1; j > colsa; --j)
            NEWSTACKELEM(i, j) =
                    (WORDPTR) ((i == j - colsa) ? one_bint : zero_bint);
    }

    DSTop = &(NEWSTACKELEM(rowsa, colsa << 1)) + 1;

#undef NEWSTACKELEM
#undef STACKELEM

    /*
       // INITIALIZE THE ROW PERMUTATION INDEX
       for(i=1;i<=rowsa;++i) (*idx)[i]=i;
     */

    if(!rplMatrixBareissEx(a, 0, rowsa, colsa << 1, 1))
        rplError(ERR_SINGULARMATRIX);
    if(Exceptions) {
        DSTop = Savestk;
        return;
    }

    rplMatrixBackSubstEx(a, rowsa, colsa << 1);
    if(Exceptions) {
        DSTop = Savestk;
        return;
    }

    // REMOVE THE ORIGINAL MATRIX TO OBTAIN THE INVERSE ONLY

    // CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
    // a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*colsa+(c)]
#define NEWSTACKELEM(r,c) a[((r)-1)*(colsa<<1)+(c)]

    // RE-PACK THE ROWS
    for(i = 1; i <= rowsa; ++i) {
        for(j = 1; j <= colsa; ++j)
            STACKELEM(i, j) = NEWSTACKELEM(i, j + colsa);
    }

    // TRIM THE STACK
    DSTop = &(STACKELEM(rowsa, colsa)) + 1;
    /*
       // NOW WE HAVE INV(A)*INV(P) ON THE STACK

       // REVERSE ALL THE ROW SWAPPING FROM THE INDEX
       for(i=1;i<=rowsa;++i) {
       if( (*idx)[i]!=(WORD)i) {
       // FIND WHERE IS THE CORRECT ROW
       for(k=i+1;k<=rowsa;++k) if((*idx)[k]==(WORD)i) break;
       // SWAP IT
       WORDPTR tmp;
       for(j=1;j<=colsa;++j) {
       tmp=STACKELEM(i,j);
       STACKELEM(i,j)=STACKELEM(k,j);
       STACKELEM(k,j)=tmp;
       }
       // AND UPDATE THE INDEX
       (*idx)[k]=(*idx)[i];
       }
       }
     */

    // WE FINALLY HAVE INV(A) HERE

    // AND COMPOSE A NEW MATRIX
    WORDPTR newmat = rplMatrixCompose(rowsa, colsa);
    DSTop = Savestk;
    if(Exceptions)
        return;
    rplOverwriteData(1, newmat);

}

// TAKE A MATRIX FROM THE STACK AND RETURN THE FROBENIUS NORM

void rplMatrixNorm()
{
    WORDPTR *Savestk, *a;
// DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
// AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk = DSTop;
    a = DSTop - 1;

// NO TYPE CHECK, THAT SHOULD BE DONE BY HIGHER LEVEL FUNCTIONS

// CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(*a + 1)), colsa = MATCOLS(* (*a + 1));

    if(!rowsa)
        rowsa = 1;

    BINT i, j;
    rplPushData((WORDPTR) zero_bint);
// DO THE ELEMENT-BY-ELEMENT OPERATION
    for(i = 0; i < rowsa; ++i) {
        for(j = 0; j < colsa; ++j) {
            rplPushData(GETELEMENT(*a, i * colsa + j));
            rplCallOvrOperator((CMD_OVR_ABS));
            if(Exceptions) {
                DSTop = Savestk;
                return;
            }
            rplPushData(rplPeekData(1));        // DUP
            rplCallOvrOperator((CMD_OVR_MUL));
            if(Exceptions) {
                DSTop = Savestk;
                return;
            }
            rplCallOvrOperator((CMD_OVR_ADD));
            if(Exceptions) {
                DSTop = Savestk;
                return;
            }

        }

// SIMPLIFY IF IT'S A SYMBOLIC TO KEEP THE OBJECTS SMALL
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions) {
                DSTop = Savestk;
                return;
            }
        }

    }

// COMPUTE THE SQUARE ROOT
    rplCallOperator(CMD_SQRT);
    if(Exceptions) {
        DSTop = Savestk;
        return;
    }

    rplOverwriteData(2, rplPeekData(1));
    rplDropData(1);
}

// RETURN NON-ZERO IF A MATRIX IS A POLAR VECTOR
// DOES NOT EXPLODE OR USE THE STACK
// NO TYPE CHECK, MAKE SURE IT'S A MATRIX

BINT rplMatrixIsPolar(WORDPTR matobj)
{
    // CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(matobj + 1)), colsa = MATCOLS(*(matobj + 1));
    BINT k;
    WORDPTR item;

    if(rowsa)
        return 0;       // NOT A VECTOR

    for(k = 0; k < colsa; ++k) {
        item = GETELEMENT(matobj, k);
        if(ISANGLE(*item))
            return 1;
    }
    return 0;

}

// GET THE POLAR MODE TEMPLATE FOR A MATRIX (UP TO 32 DIMENSIONS SUPPORTED IN POLAR MODE)
// BIT0=1 MEANS x2 IS AN ANGLE, BIT1=x3, .... BITn=x(n+1)

WORD rplMatrixPolarGetTemplate(WORDPTR matrix)
{
    WORD templ = 0;

    // CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(matrix + 1)), colsa = MATCOLS(*(matrix + 1));
    BINT k;
    WORDPTR item;

    if(rowsa)
        return 0;       // NOT A VECTOR

    for(k = 1; k < colsa; ++k) {
        item = GETELEMENT(matrix, k);
        if(ISANGLE(*item))
            templ |= (1 << (k - 1));
    }
    return templ;

}

// GET THE ANGLE MODE

BINT rplMatrixPolarGetAngMode(WORDPTR matrix)
{
    // CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(matrix + 1)), colsa = MATCOLS(*(matrix + 1));
    BINT k;
    WORDPTR item;

    if(rowsa)
        return ANGLENONE;       // NOT A VECTOR

    for(k = 1; k < colsa; ++k) {
        item = GETELEMENT(matrix, k);
        if(ISANGLE(*item))
            return ANGLEMODE(*item);
    }
    return ANGLENONE;

}

// CONVERT VECTOR IN POLAR FORM TO CARTESIAN COORDINATES
// MATRIX IS CONSIDERED A LIST OF ROW VECTORS IF rowsa>1
// WORKS ON MATRIX a EXPLODED IN THE STACK
// a POINTS TO THE MATRIX ON THE STACK, WITH ELEMENTS
// IMMEDIATELY AFTER
// RETURNS THE SAME MATRIX WITH ALTERED ELEMENTS
// EXPLODED IN THE STACK

void rplMatrixPolarToRectEx(WORDPTR * a, BINT rowsa, BINT colsa)
{
    BINT i, j, k;
    WORDPTR *stacksave = DSTop;

// CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
// a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*colsa+(c)]

    for(i = rowsa; i >= 1; --i) {
        // COMPUTE THE TOTAL VECTOR LENGTH (SQUARED) UP TO HERE
        rplPushData((WORDPTR) zero_bint);
        for(j = 1; j <= colsa; ++j) {

            if(ISANGLE(*STACKELEM(i, j))) {
                rplPushData(rplPeekData(1));
                rplCallOperator(CMD_SQRT);
                if(Exceptions) {
                    DSTop = stacksave;
                    return;
                }
                // HERE WE HAVE SQRT(x(1)^2 + ... x(n-1)^2) IN THE STACK
                rplPushData(STACKELEM(i, j));
                rplCallOperator(CMD_SIN);
                rplCallOvrOperator(CMD_OVR_MUL);
                if(Exceptions) {
                    DSTop = stacksave;
                    return;
                }
                // REPLACE THE ANGLE
                WORDPTR angle = STACKELEM(i, j);
                STACKELEM(i, j) = rplPeekData(1);
                // NOW MULTIPLY EVERY PREVIOUS VALUE BY THE COS(angle)
                rplOverwriteData(1, angle);
                rplCallOperator(CMD_COS);
                if(Exceptions) {
                    DSTop = stacksave;
                    return;
                }

                for(k = 1; k < j; ++k) {
                    rplPushData(STACKELEM(i, k));
                    rplPushData(rplPeekData(2));
                    rplCallOvrOperator(CMD_OVR_MUL);
                    if(Exceptions) {
                        DSTop = stacksave;
                        return;
                    }
                    STACKELEM(i, k) = rplPopData();
                }
                rplDropData(1); // DROP THE angle

            }
            else {
                // JUST ADD THE SQUARE TO THE TOTAL
                rplPushData(STACKELEM(i, j));
                rplPushData(rplPeekData(1));
                rplCallOvrOperator(CMD_OVR_MUL);
                rplCallOvrOperator(CMD_OVR_ADD);
            }
        }
        // DONE, REMOVE THE VECTOR LENGTH
        rplDropData(1);
    }

#undef STACKELEM

}

// CONVERT VECTOR IN CARTESIAN FORM TO POLAR COORDINATES
// MATRIX IS CONSIDERED A LIST OF ROW VECTORS IF rowsa>1
// WORKS ON MATRIX a EXPLODED IN THE STACK
// a POINTS TO THE MATRIX ON THE STACK, WITH ELEMENTS
// IMMEDIATELY AFTER
// RETURNS THE SAME MATRIX WITH ALTERED ELEMENTS
// EXPLODED IN THE STACK
// CONVERSION IS DONE BASED ON A TEMPLATE (UP TO 32 DIMENSIONS)
// EACH BIT SET TO ONE INDICATES THAT THIS COORDINATE SHOULD BE AN ANGLE
// BIT 0 => x2, BIT 1 => x3, ETC.
// FOR EXAMPLE, 2D POLAR VECTOR IS 1, 3D CYLINDRICAL IS ALSO 1
// 3D SPHERICAL IS 3
// A SINGLE angmode APPLIES TO ALL ANGLES IN THE VECTOR (ANGLE_XXX CONSTANTS)

void rplMatrixRectToPolarEx(WORDPTR * a, BINT rowsa, BINT colsa,
        WORD angtemplate, BINT angmode)
{
    BINT i, j, k;
    WORDPTR *stacksave = DSTop;

// CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
// a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*colsa+(c)]

    for(i = rowsa; i >= 1; --i) {
        // LEAVE THE COSINE SQUARE MULTIPLIER HERE, INITIALLY ONE
        rplPushData((WORDPTR) one_bint);

        // COMPUTE THE TOTAL VECTOR LENGTH (SQUARED)
        rplPushData((WORDPTR) zero_bint);

        for(k = 1; k <= colsa; ++k) {
            rplPushData(STACKELEM(i, k));
            rplPushData(rplPeekData(1));
            rplCallOvrOperator(CMD_OVR_MUL);
            rplCallOvrOperator(CMD_OVR_ADD);
            if(Exceptions) {
                DSTop = stacksave;
                return;
            }
        }

        // HERE WE HAVE THE SQUARE OF THE VECTOR LENGTH AND COSINE MULTIPLIER IN THE STACK

        BINT negcosine = 0;

        for(j = colsa; j >= 1; --j) {

            if((j > 1) && (angtemplate & (1U << (j - 2)))) {

                if(rplIsFalse(rplPeekData(1))) {
                    // ZERO LENGTH VECTOR? USE ZERO ANGLE BY CONVENTION
                    rplZeroToRReg(0);
                    WORDPTR newangle = rplNewAngleFromReal(&RReg[0], angmode);
                    if(!newangle) {
                        DSTop = stacksave;
                        return;
                    }
                    STACKELEM(i, j) = newangle;
                    // AND LEAVE ALL OTHER VALUES AS THEY ARE 1/COS(0)=1
                }
                else {
                    rplPushData(STACKELEM(i, j));
                    rplPushData(rplPeekData(3));        // GET cos^2 MULTIPLIER
                    rplPushData(rplPeekData(3));        // GET RN^2
                    Context.precdigits += 8;
                    rplCallOvrOperator(CMD_OVR_MUL);
                    rplCallOperator(CMD_SQRT);
                    Context.precdigits -= 8;
                    rplCallOvrOperator(CMD_OVR_DIV);
                    if(negcosine)
                        rplCallOvrOperator(CMD_OVR_NEG);

                    if(Exceptions) {
                        DSTop = stacksave;
                        return;
                    }
                    // HERE WE HAVE x(n)/SQRT(x(1)^2 + ... x(n)^2) IN THE STACK

                    rplCallOperator(CMD_ASIN);
                    if(Exceptions) {
                        DSTop = stacksave;
                        return;
                    }
                    // KEEP ANGLES FROM -90 TO +90 DEGREES, EXCEPT THE FIRST ONE
                    if((j == 2)
                            && (rplIsNegative(STACKELEM(i,
                                        1)) ^ rplIsNegative(rplPeekData(3)))) {
                        // FIRST ANGLE CAN BE FROM -180 TO +180
                        BINT negsine = rplIsNegative(rplPeekData(1));
                        rplPushData(rplPeekData(1));
                        rplOverwriteData(2, (WORDPTR) angle_180);
                        if(negsine)
                            rplCallOvrOperator(CMD_OVR_NEG);
                        rplCallOvrOperator(CMD_OVR_SUB);
                        if(negsine)
                            rplCallOvrOperator(CMD_OVR_NEG);
                        negcosine ^= 1;

                    }

                    if(ISNUMBER(*rplPeekData(1)) || ISANGLE(*rplPeekData(1))) {
                        // CONVERT TO THE DESIRED angmode, OTHERWISE LEAVE ALONE
                        rplConvertAngleObj(rplPeekData(1), angmode);
                        WORDPTR newangle =
                                rplNewAngleFromReal(&RReg[0], angmode);
                        if(!newangle) {
                            DSTop = stacksave;
                            return;
                        }
                        rplOverwriteData(1, newangle);
                    }

                    // REPLACE THE ELEMENT WITH THE NEW ANGLE
                    WORDPTR oldelem = STACKELEM(i, j);
                    STACKELEM(i, j) = rplPeekData(1);

                    // COMPUTE THE COS^2 OF THE ANGLE = 1-SIN^2 = 1-xn^2/(Rn^2*cosmult^2)
                    Context.precdigits += 8;
                    rplOverwriteData(1, oldelem);
                    rplPushData(rplPeekData(1));
                    rplCallOvrOperator(CMD_OVR_MUL);    // xn^2
                    rplPushData(rplPeekData(3));        // cosmult^2
                    rplPushData(rplPeekData(3));        // Rn^2
                    rplCallOvrOperator(CMD_OVR_MUL);
                    rplCallOvrOperator(CMD_OVR_DIV);
                    rplPushData(rplPeekData(1));
                    rplOverwriteData(2, (WORDPTR) & one_bint);
                    rplCallOvrOperator(CMD_OVR_SUB);

                    if(Exceptions) {
                        DSTop = stacksave;
                        return;
                    }

                    if(ISNUMBER(*rplPeekData(1))) {
                        REAL r;

                        rplReadNumberAsReal(rplPeekData(1), &r);
                        BINT tmpdigits;
                        if(iszeroReal(&r))
                            tmpdigits = 0;
                        else {
                            r.exp += Context.precdigits - 8;    // MULTIPLY BY THE DESIRED PRECISION
                            tmpdigits = intdigitsReal(&r);
                        }
                        if(tmpdigits <= 0) {

                            // COS(angle)=0 THEREFORE ANGLE IS EITHER 90 OR 270 EXACT
                            // THE ONLY WAY IS IF ALL Xi PRIOR TO THIS ONE WERE ZERO
                            // THEREFORE PUT THE VECTOR LENGTH IN AT LEAST ONE OF THEM

                            rplOverwriteData(1, rplPeekData(2));        // DUP THE VECTOR LENGTH
                            Context.precdigits -= 8;
                            rplCallOperator(CMD_SQRT);
                            Context.precdigits += 8;
                            STACKELEM(i, 1) = rplPopData();
                            // AND RESET THE cosmult TO 1 SINCE ALL VALUES ARE ZERO AND
                            // THE FIRST VALUE DOESN'T NEED TO BE MODIFIED
                            rplOverwriteData(2, (WORDPTR) one_bint);
                            negcosine = 0;
                        }
                        else {
                            // UPDATE THE cosmult^2
                            rplPushData(rplPeekData(3));
                            rplCallOvrOperator(CMD_OVR_MUL);
                            if(Exceptions) {
                                DSTop = stacksave;
                                return;
                            }

                            rplOverwriteData(2, rplPopData());  // UPDATED FOR THE NEXT ITEM

                        }
                    }
                    else {

                        // UPDATE THE cosmult^2
                        rplPushData(rplPeekData(3));
                        rplCallOvrOperator(CMD_OVR_MUL);
                        if(Exceptions) {
                            DSTop = stacksave;
                            return;
                        }

                        rplOverwriteData(2, rplPopData());      // UPDATED FOR THE NEXT ITEM

                    }

                    Context.precdigits -= 8;
                }

            }
            else {
                // NOT AN ANGLE
                // JUST SUBTRACT THE SQUARE OF THE ELEMENT FROM THE TOTAL
                rplPushData(STACKELEM(i, j));
                rplPushData(rplPeekData(1));
                rplCallOvrOperator(CMD_OVR_MUL);
                rplCallOvrOperator(CMD_OVR_SUB);

                // AND DIVIDE THE ELEMENT BY THE cosmult
                rplPushData(STACKELEM(i, j));
                Context.precdigits += 8;
                rplPushData(rplPeekData(3));    // GET cosmult^2
                rplCallOperator(CMD_SQRT);
                if(negcosine)
                    rplCallOvrOperator(CMD_OVR_NEG);    // ADD THE SIGN
                Context.precdigits -= 8;
                rplCallOvrOperator(CMD_OVR_DIV);
                STACKELEM(i, j) = rplPopData();
            }
        }
        // DONE, REMOVE THE VECTOR LENGTH AND COSINE MULTIPLIER
        rplDropData(2);
    }

#undef STACKELEM

}

// SAME AS ADD, BUT ADDS VECTORS IN POLAR MODE

void rplMatrixAddPolar(BINT negv2)
{
    WORDPTR *v1, *v2, *savestk = DSTop;

    v1 = DSTop - 2;
    v2 = DSTop - 1;

    // CHECK DIMENSIONS

    BINT rows1 = MATROWS((*v1)[1]), cols1 = MATCOLS((*v1)[1]);
    BINT rows2 = MATROWS((*v2)[1]), cols2 = MATCOLS((*v2)[1]);
    BINT elem;

    if(rows1 > 1) {
        if(cols1 != 1) {
            // NOT A VECTOR OPERATION!
            rplError(ERR_INCOMPATIBLEDIMENSION);
            return;
        }
    }

    if(rows2 > 1) {
        if(cols2 != 1) {
            // NOT A VECTOR OPERATION!
            rplError(ERR_INCOMPATIBLEDIMENSION);
            return;
        }
    }

    elem = (rows1 > 1) ? rows1 : cols1;

    if(rows2 > 1) {
        if(rows2 != elem) {
            rplError(ERR_INCOMPATIBLEDIMENSION);
            return;
        }
    }
    else {
        if(cols2 != elem) {
            rplError(ERR_INCOMPATIBLEDIMENSION);
            return;
        }
    }

    // HERE WE KNOW THE RESULT WILL BE A VECTOR OF elem ELEMENTS

    //  RESULTING VECTOR WILL FOLLOW THE TEMPLATE OF THE FIRST ARGUMENT BY CONVENTION
    WORD templ = rplMatrixPolarGetTemplate(*v1);
    BINT angmode = rplMatrixPolarGetAngMode(*v1);

    rplPushData(*v1);
    WORDPTR *firstv1 = rplMatrixExplode() - 1;  // EXPLODE V1
    if(Exceptions) {
        DSTop = savestk;
        return;
    }
    if(templ) {
        rplMatrixPolarToRectEx(firstv1, 1, elem);       // CONVERT TO RECT COORDINATES
        if(Exceptions) {
            DSTop = savestk;
            return;
        }
    }

    rplPushData(*v2);
    WORDPTR *firstv2 = rplMatrixExplode() - 1;  // EXPLODE V2
    if(Exceptions) {
        DSTop = savestk;
        return;
    }
    if(rplMatrixIsPolar(*v2)) {
        rplMatrixPolarToRectEx(firstv2, 1, elem);       // CONVERT TO RECT COORDINATES
        if(Exceptions) {
            DSTop = savestk;
            return;
        }
    }

    // OPERATE ELEMENT BY ELEMENT
    BINT k;

    for(k = 1; k <= elem; ++k) {
        rplPushData(firstv1[k]);
        rplPushData(firstv2[k]);
        if(Exceptions) {
            DSTop = savestk;
            return;
        }
        rplCallOvrOperator(negv2 ? CMD_OVR_SUB : CMD_OVR_ADD);
        if(Exceptions) {
            DSTop = savestk;
            return;
        }
        firstv1[k] = rplPopData();
    }

    // DROP THE SECOND VECTOR
    DSTop = firstv2;

    // CONVERT BACK TO POLAR IF NEEDED, TO MATCH v1 CONFIGURATION
    if(templ) {
        rplMatrixRectToPolarEx(firstv1, 1, elem, templ, angmode);
        if(Exceptions) {
            DSTop = savestk;
            return;
        }
    }

    WORDPTR newmat = rplMatrixCompose(rows1, cols1);
    if(Exceptions) {
        DSTop = savestk;
        return;
    }

    DSTop = savestk - 1;
    rplOverwriteData(1, newmat);

    return;
}

// NEG A POLAR VECTOR:
// EACH ANGLE WILL BE ADDED HALF CIRCLE
// EACH COMPONENT NOT AN ANGLE WILL BE NEGATED
// FIRST COMPONENT NOT AFFECTED

void rplMatrixNegPolar()
{
    // CHECK DIMENSIONS

    WORDPTR *matrix = DSTop - 1;
    BINT rows1 = MATROWS(*(*matrix + 1)), cols1 = MATCOLS(* (*matrix + 1));
    BINT k, elem, negdone;
    WORDPTR item;

    if(rows1 > 1) {
        if(cols1 != 1) {
            // NOT A VECTOR OPERATION!
            rplMatrixNeg();
            return;
        }
    }

    elem = (rows1 > 1) ? rows1 : cols1;

    for(negdone = 0, k = 0; k < elem; ++k) {
        item = GETELEMENT(*matrix, k);
        rplPushData(item);
        if(Exceptions) {
            DSTop = matrix + 1;
            return;
        }

        if(!k)
            continue;
        if(!negdone && ISANGLE(*item)) {
            // ADD HALF CIRCLE TO THE ANGLE
            BINT angmode = ANGLEMODE(*item);
            REAL halfturn, ang;

            switch (angmode) {
            case ANGLERAD:
                decconst_PI(&halfturn);
                break;
            case ANGLEGRAD:
                decconst_200(&halfturn);
                break;
            case ANGLEDEG:
            case ANGLEDMS:
            default:
                decconst_180(&halfturn);
                break;
            }

            rplReadNumberAsReal(item, &ang);

            halfturn.flags |= ang.flags & F_NEGATIVE;

            addReal(&RReg[4], &ang, &halfturn);

            trig_reduceangle(&RReg[4], angmode);

            // HERE RReg[0] HAS THE REDUCED ANGLE

            WORDPTR newangle = rplNewAngleFromReal(&RReg[0], angmode);
            if(Exceptions) {
                DSTop = matrix + 1;
                return;
            }

            rplOverwriteData(1, newangle);

            negdone = 1;

        }
        else {
            if(negdone) {
                // OR SIMPLY NEGATE OTHER COORDINATES
                rplCallOvrOperator(CMD_OVR_NEG);
                if(Exceptions) {
                    DSTop = matrix + 1;
                    return;
                }
            }
        }

    }

    // HERE WE HAVE A VECTOR WITH THE SAME FORMAT AS THE ORIGINAL

    WORDPTR newvec = rplMatrixCompose(rows1, cols1);
    if(Exceptions) {
        DSTop = matrix + 1;
        return;
    }
    DSTop = matrix + 1;
    *matrix = newvec;

}

// SIMILAR TO rplMatrixComposeN BUT USING FLEXIBLE ELEMENTS IN THE STACK
// ELEMENTS CAN BE LOOSE ITEMS OR VECTORS
// IF ANY ELEMENT IS A VECTOR, THE VECTOR LENGTH BECOMES THE NUMBER OF COLUMNS
// ANY LOOSE ELEMENTS WILL FILL THE MATRIX BY ROWS
// ANY VECTOR ELEMENTS CAN ONLY HAPPEN AT THE FIRST ELEMENT IN A ROW
// IF AN ELEMENT IS A MATRIX, IT IS EQUIVALENT TO MULTIPLE ROWS
// ALL VECTORS AND MATRICES MUST HAVE THE SAME NUMBER OF COLUMNS

WORDPTR rplMatrixFlexComposeN(BINT level, BINT totalelements)
{
    BINT rows, cols;

    rows = -1;
    cols = -1;

// CHECK IF ENOUGH ELEMENTS IN THE STACK
    if(rplDepthData() < level + totalelements - 1) {
        rplError(ERR_BADARGCOUNT);
        return 0;
    }

//   CHECK VALIDITY OF ALL ELEMENTS
    BINT k, j, totalsize = 0, nelem = 0;
    WORDPTR obj;

    for(k = level; k < level + totalelements; ++k) {
        obj = rplPeekData(k);
        if(ISMATRIX(*obj)) {
            // GET THE NUMBER OF COLUMNS
            BINT mcols = rplMatrixCols(obj);
            BINT mrows = rplMatrixRows(obj);

            if(cols == -1) {
                // DEFINE THE NUMBER OF COLUMNS FOR THE ENTIRE MATRIX
                cols = mcols;
            }
            else {
                // MAKE SURE THE NUMBER OF COLUMNS MATCH
                if(cols != mcols) {
                    rplError(ERR_INVALIDDIMENSION);
                    return 0;
                }

            }

            // MAKE SURE THE VECTOR IS IN THE RIGHT POSITION
            if(nelem % mcols) {
                rplError(ERR_MALFORMEDMATRIX);
                return 0;
            }

            nelem += (mrows ? mrows * mcols : mcols);
            totalsize += rplSkipOb(obj) - rplMatrixGetFirstObj(obj);    // ONLY COUNT NON-REPEATED OBJECTS

            continue;
        }
        if(!(ISNUMBERCPLX(*obj)
                    || ISSYMBOLIC(*obj)
                    || ISIDENT(*obj)
                    || ISANGLE(*obj)
                )) {
            rplError(ERR_NOTALLOWEDINMATRIX);
            return 0;
        }
        totalsize += rplObjSize(obj);
        ++nelem;
    }

// HERE WE SHOULD KNOW THE NUMBER OF COLUMNS ALREADY

    if(cols == -1) {
        // ALL LOOSE ITEMS, CREATE A VECTOR
        cols = nelem;
        rows = 0;
    }
    else {
        // CHECK WE HAVE ENOUGH ELEMENTS
        if(nelem % cols) {
            rplError(ERR_MALFORMEDMATRIX);
            return 0;
        }
        // AND COMPUTE FINAL SIZE
        rows = nelem / cols;

    }

// HERE WE HAVE A SIZE, AND EVERY OBJECT IS VERIFIED TO BE ALLOWED IN A MATRIX
// DO A FEW MORE CHECKS:
    if((rows < 0) || (rows > 65535) || (cols < 1) || (cols > 65535)) {
        rplError(ERR_INVALIDDIMENSION);
        return 0;
    }

    WORDPTR matrix =
            rplAllocTempOb(totalsize + 1 + (rows ? rows * cols : cols));
    WORDPTR newobj = matrix + 2 + (rows ? rows * cols : cols);  // POINT TO THE NEXT OBJECT TO STORE
    WORDPTR firstobj = newobj, ptr;

    if(!matrix)
        return 0;

// FINALLY, ASSEMBLE THE OBJECT
    nelem = 0;
    for(k = 0; k < totalelements; ++k) {
        obj = rplPeekData(level - 1 + totalelements - k);
        if(ISMATRIX(*obj)) {
            // INSERT ALL ELEMENTS IN THE MATRIX
            BINT i;
            BINT mrows = rplMatrixRows(obj);
            BINT offsetfix =
                    (newobj - matrix) - (rplMatrixGetFirstObj(obj) - obj);
            if(!mrows)
                ++mrows;
            for(i = 0; i < mrows; ++i) {
                for(j = 0; j < cols; ++j) {
                    // FIX THE OFFSETS
                    matrix[2 + nelem + i * cols + j] =
                            obj[2 + i * cols + j] + offsetfix;
                }
            }

            // TODO: COMPARE ONE BY ONE AND FIX OFFSETS
            // INSERT ALL OBJECTS
            BINT objsize = rplSkipOb(obj) - rplMatrixGetFirstObj(obj);
            memmovew(newobj, rplMatrixGetFirstObj(obj), objsize);
            newobj += objsize;
            nelem += mrows * cols;
        }
        else {
            // INSERT SINGLE OBJECT
            // CHECK FOR DUPLICATES
            ptr = firstobj;
            while(ptr != newobj) {
                if(rplCompareObjects(obj, ptr))
                    break;
                ptr = rplSkipOb(ptr);
            }

            if(ptr != newobj) {
                // ADD THE ORIGINAL OBJECT
                matrix[2 + nelem] = ptr - matrix;
            }
            else {
                // ADD A NEW OBJECT
                matrix[2 + nelem] = newobj - matrix;
                rplCopyObject(newobj, obj);
                newobj = rplSkipOb(newobj);
            }
            ++nelem;
        }
    }

    rplTruncateLastObject(newobj);

    matrix[0] = MKPROLOG(DOMATRIX, newobj - matrix - 1);
    matrix[1] = MATMKSIZE(rows, cols);

    return matrix;

}

BINT rplMatrixIsAllowed(WORDPTR object)
{
    if(!(ISNUMBERCPLX(*object)
                || ISSYMBOLIC(*object)
                || ISIDENT(*object)
                || ISANGLE(*object))) {
        return 0;
    }
    return 1;

}

// PUSHES TRUE ON THE STACK IF ALL ELEMENTS IN A MATRIX AT LEVEL 1 ARE ZERO

BINT rplIsZeroMatrix(WORDPTR object)
{
    WORDPTR *Savestk, *a;
    // DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
    // AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk = DSTop;
    rplPushData(object);
    a = DSTop - 1;

    // a IS THE MATRIX

    // CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(*a + 1)), colsa = MATCOLS(*(*a + 1));

    BINT totalelements = (rowsa) ? rowsa * colsa : colsa;

    BINT j;

    // DO THE ELEMENT-BY-ELEMENT OPERATION

    for(j = 0; j < totalelements; ++j) {

        rplPushData(GETELEMENT(*a, j));
        if(ISANGLE(*rplPeekData(1))) {
            // IGNORE ANGLES IN POLAR MATRICES
            // SINCE THEY DON'T CHANGE THE MAGNITUDE
            rplDropData(1);
            continue;
        }
        rplPushData((WORDPTR) zero_bint);
        rplCallOvrOperator(CMD_OVR_EQ);
        if(Exceptions) {
            DSTop = Savestk;
            return 0;
        }
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions) {
                DSTop = Savestk;
                return 0;
            }
        }

        if(rplIsFalse(rplPopData())) {
            DSTop = Savestk;
            return 0;
        }

    }

    // ALL ELEMENTS WERE ZERO
    DSTop = Savestk;
    return 1;

}

void rplMatrixSame()
{

    rplMatrixBinary((CMD_OVR_SAME));

    if(Exceptions)
        return;

    rplMatrixUnary(CMD_OVR_NOT);
    if(Exceptions)
        return;

    if(rplIsZeroMatrix(rplPeekData(1))) {
        rplDropData(1);
        rplPushTrue();
    }
    else {
        rplDropData(1);
        rplPushFalse();
    }

}

void rplMatrixEqual()
{

    rplMatrixBinary((CMD_OVR_EQ));

    if(Exceptions)
        return;

    rplMatrixUnary(CMD_OVR_NOT);
    if(Exceptions)
        return;

    if(rplIsZeroMatrix(rplPeekData(1))) {
        rplDropData(1);
        rplPushTrue();
    }
    else {
        rplDropData(1);
        rplPushFalse();
    }

}

void rplMatrixTranspose()
{
    WORDPTR *Savestk, *a;
    // DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
    // AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
    Savestk = DSTop;
    a = DSTop - 1;

    // NO TYPE CHECK, DO THAT AT HIGHER LEVEL

    // CHECK DIMENSIONS

    BINT rowsa = MATROWS(*(*a + 1)), colsa = MATCOLS(* (*a + 1));

    if(!rowsa)
        return; // NO NEED TO TRANSPOSE VECTORS
    rplMatrixExplodeByCols();
    if(Exceptions) {
        DSTop = Savestk;
        return;
    }
    WORDPTR newmat = rplMatrixCompose(colsa, rowsa);
    DSTop = Savestk;
    if(Exceptions)
        return;
    rplOverwriteData(1, newmat);
}

// COMPOSES A NEW MATRIX OBJECT FROM FILLED WITH GIVEN OBJECT
// CREATES A VECTOR IF ROWS == 0, OTHERWISE A MATRIX

WORDPTR rplMatrixFill(BINT rows, BINT cols, WORDPTR obj)
{
    BINT k, totalelements = (rows) ? rows * cols : cols;

    if((rows < 0) || (rows > 65535) || (cols < 1) || (cols > 65535)) {
        rplError(ERR_INVALIDDIMENSION);
        return 0;
    }

    if(!rplMatrixIsAllowed(obj)) {
        rplError(ERR_NOTALLOWEDINMATRIX);
        return 0;
    }

    ScratchPointer1 = obj;
    WORDPTR matrix = rplAllocTempOb(1 + totalelements + rplObjSize(obj));
    WORDPTR newobj = matrix + 2 + totalelements;        // POINT TO THE NEXT OBJECT TO STORE

    if(!matrix)
        return 0;

    obj = ScratchPointer1;

// FINALLY, ASSEMBLE THE OBJECT
    for(k = 0; k < totalelements; ++k) {
        // ADD A NEW OBJECT
        matrix[2 + k] = newobj - matrix;
    }
    rplCopyObject(newobj, obj);
    newobj = rplSkipOb(newobj);

    rplTruncateLastObject(newobj);

    matrix[0] = MKPROLOG(DOMATRIX, newobj - matrix - 1);
    matrix[1] = MATMKSIZE(rows, cols);

    return matrix;

}

// CREATE TEMPORARY STORAGE FOR ROW INDEX PERMUTATION VECTOR
// USED IN PARTIAL PIVOTING

WORDPTR rplMatrixInitIdx(BINT nrows)
{
    // RESERVE SPACE FOR THE ROW INDEX LIST

    WORDPTR IdxList = rplAllocTempOb(nrows);

    if(!IdxList) {
        return 0;
    }

    IdxList[0] = MKPROLOG(DOBINDATA, nrows);

    BINT k;
    for(k = 1; k <= nrows; ++k)
        IdxList[k] = k;

    return IdxList;
}

// COMPOSES A NEW IDENTITY MATRIX OBJECT

WORDPTR rplMatrixIdent(BINT rows)
{
    BINT k;

    if((rows < 0) || (rows > 65535)) {
        rplError(ERR_INVALIDDIMENSION);
        return 0;
    }

    WORDPTR matrix = rplAllocTempOb(1 + rows * rows + 2);
    WORDPTR newobj = matrix + 2 + rows * rows;  // POINT TO THE NEXT OBJECT TO STORE

    if(!matrix)
        return 0;

    newobj[0] = MAKESINT(0);
    newobj[1] = MAKESINT(1);

// FILL MATRIX WITH ZEROS
    for(k = 0; k < rows * rows; ++k)
        matrix[2 + k] = newobj - matrix;
// AND THE DIAGONAL WITH ONES
    for(k = 0; k < rows * rows; k += rows + 1)
        matrix[2 + k]++;

    matrix[0] = MKPROLOG(DOMATRIX, newobj - matrix + 1);
    matrix[1] = MATMKSIZE(rows, rows);

    return matrix;

}

// COMPOSES A NEW ZERO MATRIX/VECTOR OBJECT

WORDPTR rplMatrixZero(BINT rows, BINT cols)
{
    BINT k;
    if((rows < 0) || (rows > 65535) || (cols < 1) || (cols > 65535)) {
        rplError(ERR_INVALIDDIMENSION);
        return 0;
    }

    BINT isvector = (rows == 0) ? 1 : 0;

    if(isvector)
        rows = 1;
    WORDPTR matrix = rplAllocTempOb(1 + rows * cols + 1);
    WORDPTR newobj = matrix + 2 + rows * cols;  // POINT TO THE NEXT OBJECT TO STORE
    if(!matrix)
        return 0;

    matrix[0] = MKPROLOG(DOMATRIX, 1 + rows * cols + 1);
    matrix[1] = MATMKSIZE(rows - isvector, cols);
    newobj[0] = MAKESINT(0);

// FILL MATRIX WITH ZEROS
    for(k = 0; k < rows * cols; ++k)
        matrix[2 + k] = newobj - matrix;

    return matrix;

}

// QR IMPLICIT ALGORITHM BY HOUSEHOLDER REFLECTIONS
// WORKS ON MATRIX a EXPLODED IN THE STACK
// a POINTS TO THE MATRIX ON THE STACK, WITH ELEMENTS
// IMMEDIATELY AFTER
// RETURNS THE SAME MATRIX WITH ALTERED ELEMENTS
// EXPLODED IN THE STACK
// ADDS AN EXPLODED VECTOR TO THE STACK WITH THE EIGENVALUES (NROWS ELEMENTS + 1 PLACEHOLDER FOR THE VECTOR ITSELF)

// RETURNS AMATRIX WITH:
// [ qii rij ... ... rin ]
// [ ... qii rij ... rin ]
// [ ... ... qmm ... rmn ]
// THE VECTOR ON THE STACK HAS [ rii ... rmm ]

void rplMatrixQREx(WORDPTR * a, BINT rowsa, BINT colsa)
{
    BINT i, j, k;

    /* ALGORITHM:
       for j:= 1 to n do begin
       s:= 0;
       for i:=j to m do s:=s+aij^2;
       s:= sqrt(s);
       dj:= if ajj>0 then −s else s;
       fak := sqrt(s∗(s+ abs(ajj)));
       ajj:=ajj−dj;
       for k:=j to m do akj:=akj/fak;
       for i:=j+1 to n do begin
       s:= 0;
       for k:=j to m do s:=s+akj∗aki;
       for k:=j to m do aki:=aki−akj∗s;
       endfor i;
       endfor j;
     */

// CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
// a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*colsa+(c)]
#define VECTELEM(c) diagv[(c)]
//   STORE THE EIGENVALUE VECTOR d ON THE STACK AS WELL
    WORDPTR *diagv = DSTop;
    WORDPTR vector = rplMatrixZero(0, rowsa);
    if(!vector)
        return;
    rplPushDataNoGrow(vector);  // PLACEHOLDER FOR THE VECTOR OBJECT
    rplExpandStack(rowsa);
    if(Exceptions)
        return;
    for(k = 1; k <= rowsa; ++k)
        rplPushDataNoGrow((WORDPTR) zero_bint); // ALL ELEMENTS OF THE VECTOR

//    for j:= 1 to n do begin
    for(j = 1; j <= colsa; ++j) {
        //     s:= 0;
        rplPushData((WORDPTR) zero_bint);
        //     for i:=j to m do s:=s+a2ij;
        for(i = j; i <= rowsa; ++i) {
            rplPushDataNoGrow(STACKELEM(i, j));
            rplPushDataNoGrow(STACKELEM(i, j));
            rplCallOvrOperator(CMD_OVR_MUL);
            if(Exceptions)
                return;
            rplCallOvrOperator(CMD_OVR_ADD);
            if(Exceptions)
                return;

        }
        //   s:= sqrt(s);
        if(ISSYMBOLIC(*rplPeekData(1)))
            rplSymbAutoSimplify();
        rplCallOperator(CMD_SQRT);
        if(Exceptions)
            return;

        //    fak := sqrt(s∗(s+ abs(ajj)));
        rplPushDataNoGrow(STACKELEM(j, j));
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions)
            return;
        rplPushDataNoGrow(rplPeekData(2));      // s
        rplCallOvrOperator(CMD_OVR_ADD);
        if(Exceptions)
            return;
        rplPushDataNoGrow(rplPeekData(2));      // s
        rplCallOvrOperator(CMD_OVR_MUL);
        if(Exceptions)
            return;
        rplCallOperator(CMD_SQRT);
        if(Exceptions)
            return;
        if(ISSYMBOLIC(*rplPeekData(1)))
            rplSymbAutoSimplify();
        if(Exceptions)
            return;
        // Terminate if fak==0
        if(rplIsFalse(rplPeekData(1)))
            return;     // SHOULD WE CLEANUP SOME ELEMENTS BEFORE?

        //  dj:= if ajj>0 then −s else s;
        //  ajj:=ajj−dj;
        rplPushDataNoGrow(STACKELEM(j, j));
        rplPushDataNoGrow(rplPeekData(3));      // s
        // FOR COMPLEX NUMBERS THIS WILL ALWAYS ADD s
        if(!rplIsNegative(STACKELEM(j, j))) {
            rplCallOvrOperator(CMD_OVR_NEG);
            if(Exceptions)
                return;
        }
        VECTELEM(j) = rplPeekData(1);
        rplCallOvrOperator(CMD_OVR_SUB);
        if(Exceptions)
            return;
        STACKELEM(j, j) = rplPopData();

        // HERE THE STACK HAS L2-->s AND L1-->fak

        // for k:=j to m do akj:=akj/fak;
        for(k = j; k <= rowsa; ++k) {
            rplPushDataNoGrow(STACKELEM(k, j));
            rplPushDataNoGrow(rplPeekData(2));
            rplCallOvrOperator(CMD_OVR_DIV);
            if(Exceptions)
                return;
            STACKELEM(k, j) = rplPopData();
        }

        rplDropData(2);

        // for i:=j+1 to n do begin
        for(i = j + 1; i <= colsa; ++i) {

            //            s:= 0;
            rplPushDataNoGrow((WORDPTR) zero_bint);

            //            for k:=j to m do s:=s+akj∗aki;
            for(k = j; k <= rowsa; ++k) {
                rplPushDataNoGrow(STACKELEM(k, j));
                rplPushDataNoGrow(STACKELEM(k, i));
                rplCallOvrOperator(CMD_OVR_MUL);
                if(Exceptions)
                    return;
                rplCallOvrOperator(CMD_OVR_ADD);
                if(Exceptions)
                    return;
            }

            if(ISSYMBOLIC(*rplPeekData(1)))
                rplSymbAutoSimplify();
            if(Exceptions)
                return;
            //           for k:=j to m do aki:=aki−akj∗s;
            for(k = j; k <= rowsa; ++k) {
                rplPushDataNoGrow(STACKELEM(k, i));
                rplPushDataNoGrow(STACKELEM(k, j));
                rplPushDataNoGrow(rplPeekData(3));
                rplCallOvrOperator(CMD_OVR_MUL);
                if(Exceptions)
                    return;
                rplCallOvrOperator(CMD_OVR_SUB);
                if(Exceptions)
                    return;
                if(ISSYMBOLIC(*rplPeekData(1)))
                    rplSymbAutoSimplify();
                if(Exceptions)
                    return;
                STACKELEM(k, i) = rplPopData();
            }

            rplDropData(1);     // DROP s
            //   endfor i;
        }

        // endfor j;
    }

#undef STACKELEM
#undef VECTELEM

}

// EXTRACT EXPLICIT MATRIX Q FROM IMPLICIT QR DECOMPOSITION
// EXPECTS IMPLICIT QR MATRIX IN a, AND DIAGONAL VECTOR IN diag
// RETURNS Q AS A MATRIX OBJECT

WORDPTR rplMatrixQRGetQ(WORDPTR * a, BINT rowsa, BINT colsa, WORDPTR * diagv)
{
    UNUSED_ARGUMENT(diagv);
    // CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
    // a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*colsa+(c)]
#define VECTELEM(c) diagv[(c)]
#define ZELEM(r) z[(r)-1]
    WORDPTR *stksave = DSTop, *z;
    BINT i, j, k;

    for(i = 1; i <= rowsa; ++i) {

        z = DSTop;
        for(k = 1; k <= rowsa; ++k) {
            rplPushData((i == k) ? (WORDPTR) one_bint : (WORDPTR) zero_bint);
            if(Exceptions) {
                DSTop = stksave;
                return 0;
            }
        }

        for(j = colsa; j >= 1; --j) {
            rplPushDataNoGrow((WORDPTR) zero_bint);
            for(k = j; k <= rowsa; ++k) {
                rplPushDataNoGrow(STACKELEM(k, j));
                rplPushDataNoGrow(ZELEM(k));
                rplCallOvrOperator(CMD_OVR_MUL);
                if(Exceptions) {
                    DSTop = stksave;
                    return 0;
                }
                rplCallOvrOperator(CMD_OVR_ADD);
                if(Exceptions) {
                    DSTop = stksave;
                    return 0;
                }

            }
            if(ISSYMBOLIC(*rplPeekData(1)))
                rplSymbAutoSimplify();
            if(Exceptions) {
                DSTop = stksave;
                return 0;
            }

            for(k = j; k <= rowsa; ++k) {
                rplPushDataNoGrow(ZELEM(k));
                rplPushDataNoGrow(STACKELEM(k, j));
                rplPushDataNoGrow(rplPeekData(3));      // s
                rplCallOvrOperator(CMD_OVR_MUL);
                if(Exceptions) {
                    DSTop = stksave;
                    return 0;
                }
                rplCallOvrOperator(CMD_OVR_SUB);        // zk=zk-akj*s;
                if(Exceptions) {
                    DSTop = stksave;
                    return 0;
                }
                if(ISSYMBOLIC(*rplPeekData(1)))
                    rplSymbAutoSimplify();
                if(Exceptions) {
                    DSTop = stksave;
                    return 0;
                }
                ZELEM(k) = rplPopData();
            }
            rplDropData(1);
        }

    }

#undef STACKELEM
#undef VECTELEM
#undef ZELEM

    WORDPTR newmat = rplMatrixCompose(rowsa, rowsa);
    if(!newmat) {
        DSTop = stksave;
        return 0;
    }
    rplPushDataNoGrow(newmat);
    rplMatrixTranspose();
    newmat = rplPopData();
    DSTop = stksave;
    if(Exceptions || (!newmat))
        return 0;
    return newmat;

}

// EXTRACT EXPLICIT MATRIX Q FROM IMPLICIT QR DECOMPOSITION
// EXPECTS IMPLICIT QR MATRIX IN a, AND DIAGONAL VECTOR IN diag
// RETURNS Q AS A MATRIX OBJECT

WORDPTR rplMatrixQRGetR(WORDPTR * a, BINT rowsa, BINT colsa, WORDPTR * diagv)
{
    // CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
    // a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*colsa+(c)]
#define VECTELEM(c) diagv[(c)]

    WORDPTR *stksave = DSTop;
    BINT i, j;

    for(i = 1; i <= rowsa; ++i) {

        for(j = 1; j <= colsa; ++j) {
            if(j < i)
                rplPushData((WORDPTR) zero_bint);
            else if(j == i)
                rplPushData(VECTELEM(j));
            else
                rplPushData(STACKELEM(i, j));
            if(Exceptions) {
                stksave = DSTop;
                return 0;
            }
        }

    }

#undef STACKELEM
#undef VECTELEM

    WORDPTR newmat = rplMatrixCompose(rowsa, colsa);
    if(!newmat) {
        DSTop = stksave;
        return 0;
    }
    DSTop = stksave;
    if(Exceptions || (!newmat))
        return 0;
    return newmat;

}


// PERFORMS A(k+1) = R*Q after A(k) WAS DECOMPOSED BY QR IMPLICIT
// EXPECTS IMPLICIT QR MATRIX IN a, AND DIAGONAL VECTOR IN diag
// MATRIX MUST BE SQUARE OF nxn AND diag IS THE n ELEMENT DIAGONAL
// RETURNS A(k+1) AS A MATRIX OBJECT

WORDPTR rplMatrixQRDoRQ(WORDPTR * a, BINT n, WORDPTR * diagv)
{
    UNUSED_ARGUMENT(diagv);
    // CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
    // a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*n+(c)]
#define VECTELEM(c) diagv[(c)]
#define ZELEM(r) z[(r)-1]
    WORDPTR *stksave = DSTop, *z;
    BINT i, j, k;

    for(i = 1; i <= n; ++i) {

        z = DSTop;
        for(k = 1; k <= n; ++k) {   // EXTRACT VECTORS FROM THE ROWS OF MATRIX R
            if(k<i) rplPushData((WORDPTR) zero_bint);
            else if(k>i) rplPushData(STACKELEM(i,k));
            else rplPushData(VECTELEM(i));
            if(Exceptions) {
                DSTop = stksave;
                return 0;
            }
        }

        for(j = 1; j <= n; ++j) {       // PERFORM LOOP FOR QT*RT
            rplPushDataNoGrow((WORDPTR) zero_bint);
            for(k = j; k <= n; ++k) {
                rplPushDataNoGrow(STACKELEM(k, j));
                rplPushDataNoGrow(ZELEM(k));
                rplCallOvrOperator(CMD_OVR_MUL);
                if(Exceptions) {
                    DSTop = stksave;
                    return 0;
                }
                rplCallOvrOperator(CMD_OVR_ADD);
                if(Exceptions) {
                    DSTop = stksave;
                    return 0;
                }

            }
            if(ISSYMBOLIC(*rplPeekData(1)))
                rplSymbAutoSimplify();
            if(Exceptions) {
                DSTop = stksave;
                return 0;
            }

            for(k = j; k <= n; ++k) {
                rplPushDataNoGrow(ZELEM(k));
                rplPushDataNoGrow(STACKELEM(k, j));
                rplPushDataNoGrow(rplPeekData(3));      // s
                rplCallOvrOperator(CMD_OVR_MUL);
                if(Exceptions) {
                    DSTop = stksave;
                    return 0;
                }
                rplCallOvrOperator(CMD_OVR_SUB);        // zk=zk-akj*s;
                if(Exceptions) {
                    DSTop = stksave;
                    return 0;
                }
                if(ISSYMBOLIC(*rplPeekData(1)))
                    rplSymbAutoSimplify();
                if(Exceptions) {
                    DSTop = stksave;
                    return 0;
                }
                ZELEM(k) = rplPopData();
            }
            rplDropData(1);
        }

    }

#undef STACKELEM
#undef VECTELEM
#undef ZELEM

    WORDPTR newmat = rplMatrixCompose(n, n);
    if(!newmat) {
        DSTop = stksave;
        return 0;
    }
    return newmat;

}
