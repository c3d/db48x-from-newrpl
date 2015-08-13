/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LIBRARY 48 IMPLEMENTS THE MATRIX (AND VECTOR) OBJECTS

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  48
#define LIB_ENUM lib48_enum
#define LIB_NAMES lib48_names
#define LIB_HANDLER lib48_handler
#define LIB_NUMBEROFCMDS LIB48_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(AUGMENT), \
    CMD(AXL), \
    CMD(AXM), \
    CMD(BASIS), \
    CMD(CHOLESKY), \
    CMD(CNRM), \
    CMD(CON), \
    CMD(COND), \
    CMD(CROSS), \
    CMD(CSWP), \
    CMD(DET), \
    CMD(DIAGMAP), \
    CMD(DOT), \
    CMD(EGV), \
    CMD(EGVL), \
    CMD(GRAMSCHMIDT), \
    CMD(HADAMARD), \
    CMD(HILBERT), \
    CMD(IBASIS), \
    CMD(IDN), \
    CMD(IMAGE), \
    CMD(ISOM), \
    CMD(JORDAN), \
    CMD(KER), \
    CMD(LQ), \
    CMD(LSQ), \
    CMD(LU), \
    CMD(MAD), \
    CMD(MKISOM), \
    CMD(PMINI), \
    CMD(QR), \
    CMD(RANK), \
    CMD(RANM), \
    CMD(RCI), \
    CMD(RCIJ), \
    CMD(RDM), \
    CMD(REF), \
    CMD(RNRM), \
    CMD(RREF), \
    CMD(RREFMOD), \
    CMD(RSD), \
    CMD(RSWP), \
    CMD(SCHUR), \
    CMD(SNRM), \
    CMD(SRAD), \
    CMD(SVD), \
    CMD(SVL), \
    CMD(SYLVESTER), \
    CMD(TRACE), \
    CMD(TRAN), \
    CMD(TRN), \
    CMD(VANDERMONDE)



// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    "→ARRY", \
    "ARRY→", \
    "→COL", \
    "COL+", \
    "COL-", \
    "COL→", \
    "→DIAG", \
    "DIAG→", \
    "→ROW", \
    "ROW-", \
    "ROW+", \
    "ROW→", \
    "→V2", \
    "→V3", \
    "V→"



#define CMD_EXTRAENUM \
    TOARRAY, \
    ARRAYDECOMP, \
    TOCOL, \
    ADDCOL, \
    REMCOL, \
    FROMCOL, \
    TODIAG, \
    FROMDIAG, \
    TOROW, \
    ADDROW, \
    REMROW, \
    FROMROW, \
    TOV2, \
    TOV3, \
    FROMV




// INTERNAL DECLARATIONS


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_EXTRAENUM , CMD_LIST ,  LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= { CMD_EXTRANAME , CMD_LIST  };
#undef CMD



void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        rplPushData(IPtr);
        return;
    }


    if(ISUNARYOP(CurOpcode)) {
        // UNARY OPERATORS
        switch(OPCODE(CurOpcode))
        {

        case OVR_INV:
        {
            if(rplDepthData()<1) {
                Exceptions|=EX_BADARGCOUNT;
                ExceptionPointer=IPtr;
                return;
            }
            WORDPTR a=rplPeekData(1);

            if(!ISMATRIX(*a)) {
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }

            rplMatrixInvert();
            return;
        }
        case OVR_ABS:
        {
           // COMPUTE THE FROBENIUS NORM ON MATRICES
            if(rplDepthData()<1) {
                Exceptions|=EX_BADARGCOUNT;
                ExceptionPointer=IPtr;
                return;
            }
            WORDPTR a=rplPeekData(1);

            if(!ISMATRIX(*a)) {
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }

            rplMatrixNorm();
            return;

        }
        case OVR_NEG:
        {
            if(rplDepthData()<1) {
                Exceptions|=EX_BADARGCOUNT;
                ExceptionPointer=IPtr;
                return;
            }
            WORDPTR a=rplPeekData(1);

            if(!ISMATRIX(*a)) {
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }

            rplMatrixNeg();
            return;

        }
        }

    }

    if(ISBINARYOP(CurOpcode)) {

        // TODO: IMPLEMENT BINARY OPERATORS

        switch(OPCODE(CurOpcode))
        {
        case OVR_ADD:
        {
            if(rplDepthData()<2) {
                Exceptions|=EX_BADARGCOUNT;
                ExceptionPointer=IPtr;
                return;
            }
            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            if(!ISMATRIX(*a) || !ISMATRIX(*b)) {
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }

            rplMatrixAdd();
            return;
        }
        case OVR_SUB:
        {
            if(rplDepthData()<2) {
                Exceptions|=EX_BADARGCOUNT;
                ExceptionPointer=IPtr;
                return;
            }
            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            if(!ISMATRIX(*a) || !ISMATRIX(*b)) {
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }

            rplMatrixSub();
            return;
        }
        case OVR_MUL:
        {
            if(rplDepthData()<2) {
                Exceptions|=EX_BADARGCOUNT;
                ExceptionPointer=IPtr;
                return;
            }

            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            // NEED TO DETECT SCALAR * MATRIX AND MATRIX * SCALAR CASES
            if((ISNUMBERCPLX(*a)
                  || ISSYMBOLIC(*a)
                  || ISIDENT(*a))) {
                // SCALAR BY MATRIX
                rplMatrixMulScalar();

                return;
            }
            if((ISNUMBERCPLX(*b)
                  || ISSYMBOLIC(*b)
                  || ISIDENT(*b))) {
                // SCALAR BY MATRIX
                rplMatrixMulScalar();

                return;
            }

            // HERE IT HAS TO BE MATRIX x MATRIX

            if(!ISMATRIX(*a) || !ISMATRIX(*b)) {
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }

            rplMatrixMul();
            return;
        }

        case OVR_DIV:
        {
            if(rplDepthData()<2) {
                Exceptions|=EX_BADARGCOUNT;
                ExceptionPointer=IPtr;
                return;
            }

            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            // NEED TO DETECT SCALAR / MATRIX AND MATRIX / SCALAR CASES
            if((ISNUMBERCPLX(*a)
                  || ISSYMBOLIC(*a)
                  || ISIDENT(*a))) {
                // SCALAR BY MATRIX
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }
            if((ISNUMBERCPLX(*b)
                  || ISSYMBOLIC(*b)
                  || ISIDENT(*b))) {
                // MATRIX DIV BY SCALAR
                rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_INV));
                if(Exceptions) return;
                rplMatrixMulScalar();

                return;
            }

            // HERE IT HAS TO BE MATRIX / MATRIX

            if(!ISMATRIX(*a) || !ISMATRIX(*b)) {
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }

            // PERFORM A*INV(B)

            rplMatrixInvert();
            if(Exceptions) return;
            rplMatrixMul();
            return;


        }

        case OVR_POW:
        {
            if(rplDepthData()<2) {
                Exceptions|=EX_BADARGCOUNT;
                ExceptionPointer=IPtr;
                return;
            }

            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            // ONLY MATRIX RAISED TO NUMERIC POWER IS SUPPORTED
            if(!ISMATRIX(*a) || !ISNUMBER(*b)) {
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }

            if(ISREAL(*b)) {
                REAL real;
                rplReadReal(b,&real);
                if(!isintegerReal(&real)) {
                    Exceptions|=EX_BADARGVALUE;
                    ExceptionPointer=IPtr;
                    return;
                }

            }
                BINT rows=MATROWS(a[1]),cols=MATCOLS(a[1]);

                if(rows!=cols) {
                    Exceptions|=EX_INVALID_DIM;
                    ExceptionPointer=IPtr;
                    return;
                }

                // TODO: CHECK FOR INTEGER RANGE AND ISSUE "Integer too large" ERROR
                BINT64 exp=rplReadNumberAsBINT(b);
                if(Exceptions) return;
                rplPopData();
                if(exp<0) {
                 rplMatrixInvert();
                 if(Exceptions) return;
                 exp=-exp;
                }

                BINT hasresult=0;
                while(exp) {
                if(exp&1) {
                    if(!hasresult) { rplPushData(rplPeekData(1));   // DUP THE CURRENT MATRIX
                                     hasresult=1;
                                }
                                else {
                                        rplPushData(rplPeekData(2));
                                        rplPushData(rplPeekData(2));
                                        rplMatrixMul();
                                        if(Exceptions) return;
                                        rplOverwriteData(3,rplPeekData(1));
                                        rplPopData();
                                 }
                }
                exp>>=1;
                if(exp) {
                rplPushData(rplPeekData(1));
                rplMatrixMul();
                if(Exceptions) return;
                }
                }

                rplPopData();
                return;
            }

        break;
        }
    }

    switch(OPCODE(CurOpcode))
    {
    case TOARRAY:
    {
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        BINT64 rows,cols;
        WORDPTR *Savestk=DSTop;

        if(ISLIST(*rplPeekData(1))) {
            rplExplodeList(rplPeekData(1));
            BINT ndims=rplReadNumberAsBINT(rplPopData());
            if((ndims<1) || (ndims>2)) {
                DSTop=Savestk;
                Exceptions|=EX_INVALID_DIM;
                ExceptionPointer=IPtr;
                return;
            }

            if(!ISNUMBER(*rplPeekData(1))) rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NUM));

            cols=rplReadNumberAsBINT(rplPopData());
            if(Exceptions) {
                DSTop=Savestk;
                Exceptions|=EX_INVALID_DIM;
                ExceptionPointer=IPtr;
                return;
            }

            if(ndims==2) {

                if(!ISNUMBER(*rplPeekData(1))) rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NUM));

                rows=rplReadNumberAsBINT(rplPopData());
                if(Exceptions) {
                    DSTop=Savestk;
                    Exceptions|=EX_INVALID_DIM;
                    ExceptionPointer=IPtr;
                    return;
                }



            } else rows=0;


            if( (rows<0)||(rows>65535)||(cols<1)||(cols>65535))  {
                DSTop=Savestk;
                Exceptions|=EX_INVALID_DIM;
                ExceptionPointer=IPtr;
                return;
            }

            // REMOVE THE LIST
            rplDropData(1);

            }
        else {
            // IT HAS TO BE A NUMBER
            if(!ISNUMBER(*rplPeekData(1))) rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NUM));

            cols=rplReadNumberAsBINT(rplPopData());
            if(Exceptions) {
                DSTop=Savestk;
                Exceptions|=EX_INVALID_DIM;
                ExceptionPointer=IPtr;
                return;
            }

            rows=0;

            if((cols<1)||(cols>65535))  {
                DSTop=Savestk;
                Exceptions|=EX_INVALID_DIM;
                ExceptionPointer=IPtr;
                return;
            }

        }

        // HERE WE HAVE PROPER ROWS AND COLUMNS
        BINT elements=(rows)? rows*cols:cols;

        if(rplDepthData()<elements) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }

        WORDPTR newmat=rplMatrixCompose(rows,cols);

        if(newmat) {
            rplDropData(elements);
            rplPushData(newmat);
        }

        return;
        
       }
    case ARRAYDECOMP:
    {
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(!ISMATRIX(*rplPeekData(1))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        WORDPTR matrix=rplPeekData(1);
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

        WORDPTR *elem=rplMatrixExplode();
        if(Exceptions) return;

        // NOW REMOVE THE ORIGINAL MATRIX FROM THE STACK
        memmovew(elem-1,elem,(DSTop-elem)*(sizeof(WORDPTR *)/sizeof(WORD))); // ADDED sizeof() ONLY FOR 64-BIT COMPATIBILITY

        DSTop--;


        if(rows) rplNewBINTPush(rows,DECBINT);
        rplNewBINTPush(cols,DECBINT);
        rplPushData((rows)? two_bint : one_bint);
        rplCreateList();


        return;
    }
        // ADD MORE OPCODES HERE
    
    case TOCOL:
    {
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(!ISMATRIX(*rplPeekData(1))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        WORDPTR matrix=rplPeekData(1);
        WORDPTR *matptr=DSTop-1;
        WORDPTR elem;
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
        BINT nrows=(rows)? rows:1;
        BINT i,j;

        for(j=1;j<=cols;++j) {
        for(i=1;i<=nrows;++i) rplPushData(rplMatrixFastGet(*matptr,i,j));
        if(rows) {
            elem=rplMatrixCompose(0,nrows);
            if(!elem) return;
            rplDropData(nrows);
        } else elem=rplPopData();

        rplPushData(*matptr);
        rplOverwriteData(2,elem);
        matptr=DSTop-1;
        }

        rplDropData(1);
        rplNewBINTPush(cols,DECBINT);

        return;

    }

    case ADDCOL:
    {
        if(rplDepthData()<3) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(!ISMATRIX(*rplPeekData(3))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }
        if(!ISMATRIX(*rplPeekData(2)) && !ISNUMBER(*rplPeekData(2))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        BINT64 nelem=rplReadNumberAsBINT(rplPeekData(1));





        WORDPTR matrix=rplPeekData(3);
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

        WORDPTR *matptr=DSTop-3;
        WORDPTR elem;

        if( (nelem<1)||(nelem>cols+1)) {
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }


        if(!rows) {
            // ADD ELEMENTS TO A VECTOR

            // CHECK VALID TYPES FOR MATRIX ELEMENTS
            if(!ISNUMBERCPLX(*rplPeekData(2)) && !ISSYMBOLIC(*rplPeekData(2))
                  && !ISIDENT(*rplPeekData(2)))
             {
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }

            WORDPTR *first=rplMatrixNewEx(1,cols+1);

            if(!first) {
                return;
            }


            BINT j;
            WORDPTR *stkelem;
            for(j=1;j<nelem;++j) {
                stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
                *stkelem=rplMatrixFastGet(*matptr,1,j);
            }

            stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
            *stkelem=matptr[1]; // THE NEW ELEMENT MIGHT HAVE MOVED, SO GET IT FROM THE STACK
            ++j;

            for(;j<=cols+1;++j) {
                stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
                *stkelem=rplMatrixFastGet(*matptr,1,j-1);
            }

            // MAKE A NEW VECTOR

            WORDPTR newmat=rplMatrixCompose(0,cols+1);
            if(!newmat) {
                DSTop=matptr+3;
                return;
            }

            rplDropData(cols+4);
            rplPushData(newmat);
            return;

        }

        // ADD A VECTOR OR A MATRIX TO A MATRIX

        if(!ISMATRIX(*rplPeekData(2))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }



        WORDPTR mat2=rplPeekData(2);
        BINT rows2=MATROWS(mat2[1]),cols2=MATCOLS(mat2[1]);


        // CHECK PROPER SIZE

        if(!rows2) {
            // MAKE IT A COLUMN VECTOR
            rows2=cols2;
            cols2=1;
        }

        if(rows2!=rows) {
            Exceptions|=EX_INVALID_DIM;
            ExceptionPointer=IPtr;
            return;
        }

        // ADD THE COLUMNS

        WORDPTR *first=rplMatrixNewEx(rows,cols+cols2);

        if(!first) {
            return;
        }


        BINT i,j;
        WORDPTR *stkelem;
        for(j=1;j<nelem;++j) {
            for(i=1;i<=rows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols+cols2,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j);
            }
        }

        for(;j<nelem+cols2;++j) {
            for(i=1;i<=rows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols+cols2,i,j);
            *stkelem=rplMatrixFastGet(matptr[1],i,j-nelem+1);
            }
        }

        for(;j<=cols+cols2;++j) {
            for(i=1;i<=rows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols+cols2,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j-cols2);
            }
        }

        // MAKE A NEW VECTOR

        WORDPTR newmat=rplMatrixCompose(rows,cols+cols2);
        if(!newmat) {
            DSTop=matptr+3;
            return;
        }

        rplDropData(rows*(cols+cols2)+3);
        rplPushData(newmat);
        return;
    }

    case REMCOL:
    {
        if(rplDepthData()<2) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(!ISMATRIX(*rplPeekData(2))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        BINT64 ncol=rplReadNumberAsBINT(rplPeekData(1));

        WORDPTR matrix=rplPeekData(2);
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
        BINT nrows=(rows)? rows:1;

        WORDPTR *matptr=DSTop-2;
        WORDPTR elem;

        if(cols<=1) {
            Exceptions|=EX_INVALID_DIM;
            ExceptionPointer=IPtr;
            return;
        }


        if( (ncol<1)||(ncol>cols)) {
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }

        // MAKE THE NEW MATRIX WITHOUT ONE COLUMN

        WORDPTR *first=rplMatrixNewEx(rows,cols-1);

        if(!first) {
            return;
        }


        BINT i,j;
        WORDPTR *stkelem;
        for(j=1;j<ncol;++j) {
            for(i=1;i<=nrows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols-1,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j);
            }
        }

        // SEPARATE THE COLUMN VECTOR/ELEMENT
        for(i=1;i<=nrows;++i) {
        rplPushData(rplMatrixFastGet(*matptr,i,j));
        }


        for(;j<=cols-1;++j) {
            for(i=1;i<=nrows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols-1,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j+1);
            }
        }


        // MAKE THE VECTOR FROM THE ELEMENTS
        WORDPTR newmat;

        if(rows) {
            newmat=rplMatrixCompose(0,nrows);
            if(!newmat) {
                DSTop=matptr+2;
                return;
            }
            rplDropData(nrows);

        } else newmat=rplPopData();

        matptr[1]=newmat;   //  OVERWRITE THE FIRST ARGUMENT WITH THE VECTOR


        // MAKE A NEW VECTOR/MATRIX

        newmat=rplMatrixCompose(rows,cols-1);
        if(!newmat) {
            DSTop=matptr+2;
            return;
        }

        rplDropData(nrows*(cols-1));

        rplOverwriteData(2,newmat);

        return;

    }
    case FROMCOL:
        {
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        BINT64 nelem=rplReadNumberAsBINT(rplPeekData(1));

        if(rplDepthData()<nelem+1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }

        BINT i;
        BINT veclen=0;

        for(i=2;i<=nelem+1;++i) {
            if(ISMATRIX(*rplPeekData(i))) {
                WORDPTR matrix=rplPeekData(i);
                BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

                if(rows) {
                    Exceptions|=EX_INVALID_DIM;
                    ExceptionPointer=IPtr;
                    return;
                }

                if(veclen) {
                    if(cols!=veclen) {
                        Exceptions|=EX_INVALID_DIM;
                        ExceptionPointer=IPtr;
                        return;
                    }
                } else {
                    if(i==2) veclen=cols;
                    else {
                        // DON'T ALLOW MIX OF VECTOR/NUMBERS
                        Exceptions|=EX_INVALID_DIM;
                        ExceptionPointer=IPtr;
                        return;
                    }
                }
            }
            else {
                if(! (ISNUMBERCPLX(*LastCompiledObject)
                      || ISSYMBOLIC(*LastCompiledObject)
                      || ISIDENT(*LastCompiledObject))) {
                            Exceptions|=EX_BADARGTYPE;
                            ExceptionPointer=IPtr;
                            return;
                }

                if(veclen) {
                    Exceptions|=EX_INVALID_DIM;
                    ExceptionPointer=IPtr;
                    return;
                }

            }
        }

        // HERE WE HAVE ALL ELEMENTS PROPERLY VALIDATED





        }
    case TODIAG:
    case FROMDIAG:
    case TOROW:
    case ADDROW:
    case REMROW:
    case FROMROW:
    case TOV2:
    case TOV3:
    case FROMV:











    break;



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

        // CHECK IF THE TOKEN IS THE OPEN BRACKET

        if(*((char * )TokenStart)=='[')
        {
            if(LIBNUM(CurrentConstruct)==LIBRARY_NUMBER) {
                // WE ARE COMPILING OBJECTS INSIDE A MATRIX ALREADY
               if(CurrentConstruct==MKPROLOG(LIBRARY_NUMBER,0)) {
                // WE ARE IN THE OUTER DIMENSION
                // INCREASE DEPTH OF DIMENSION AND ACCEPT
                // WARNING, THIS USES INTERNAL COMPILER WORKINGS
                WORDPTR matrix=*(ValidateTop-1);
                ++*matrix;
                // CHECK IF THERE IS A SIZE WORD YET
                if(CompileEnd==matrix+1) {
                    // THIS IS THE FIRST OBJECT IN THE ARRAY
                    // ADD A DUMMY WORD
                    rplCompileAppend(MATMKSIZE(1,0));
                }
                else {
                    // THERE SHOULD BE A SIZE WORD ALREADY
                    // INCREASE THE ROW COUNT
                    BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
                    if(!rows) {
                        // VECTOR CAN'T OPEN A SECOND DIMENSION
                        RetNum=ERR_SYNTAX;
                        return;
                    }
                    matrix[1]=MATMKSIZE(rows+1,cols);
                }



                if(TokenLen>1) NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_CONTINUE_NOVALIDATE;
                return;
                }
                else {
                    // MORE THAN 2 DIMENSIONS ARE NOT SUPPORTED
                    RetNum=ERR_NOTMINE;
                    return;
               }
            }

            rplCompileAppend((WORD) MKPROLOG(LIBRARY_NUMBER,0));
            if(TokenLen>1) {
                NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_STARTCONSTRUCT;
            }
            else RetNum=OK_STARTCONSTRUCT;
            return;
        }
        // CHECK IF THE TOKEN IS THE CLOSING BRACKET

        if(((char * )TokenStart)[TokenLen-1]==']')
        {

            if(TokenLen>1) {
                BlankStart=NextTokenStart=(WORDPTR)(((char * )TokenStart)+TokenLen-1);
                RetNum=ERR_NOTMINE_SPLITTOKEN;
                return;
            }

            if(LIBNUM(CurrentConstruct)!=LIBRARY_NUMBER) {
                RetNum=ERR_SYNTAX;
                return;
            }
            WORDPTR matrix=*(ValidateTop-1);
            BINT rows,cols;
            BINT totalelements;

            if(CompileEnd>matrix+1) {
            rows=MATROWS(matrix[1]);
            cols=MATCOLS(matrix[1]);
            if(rows==0) totalelements=cols;
            else totalelements=rows*cols;
            } else rows=cols=totalelements=0;

            if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,0)) {
                // CLOSED AN INNER DIMENSION

                // CAN'T CLOSE AN EMPTY MATRIX
                if(!totalelements) {
                    RetNum=ERR_SYNTAX;
                    return;
                }

                // DECREASE DIMENSION COUNT
                --*matrix;


                // CHECK FULL ROW SIZE IS CORRECT
                // BY CHECKING THE NEXT EMPTY OBJECT IS THE START OF A ROW

                BINT count;
                WORDPTR index=matrix+2;

                count=0;
                while((count<totalelements) && (index<CompileEnd)) { ++count; index=rplSkipOb(index); }

                if(count%cols) {
                    // INVALID MATRIX SIZE
                    RetNum=ERR_SYNTAX;
                    return;
                }


                if(TokenLen>1) NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_CONTINUE_NOVALIDATE;
                return;

            }

            // CLOSE THE MATRIX OBJECT
            if(!totalelements) {
                RetNum=ERR_SYNTAX;
                return;
            }

            // STRETCH THE OBJECT, ADD THE INDEX AND REMOVE DUPLICATES
            WORDPTR endofobjects=rplCompileAppendWords(totalelements);
            if(Exceptions) return;

            // MAKE HOLE IN MEMORY
            memmovew(matrix+2+totalelements,matrix+2,endofobjects-(matrix+2));
            endofobjects+=totalelements;

            // NOW WRITE THE INDICES. ALL OFFSETS ARE RELATIVE TO MATRIX PROLOG!
            WORDPTR ptr=matrix+2,objptr=ptr+totalelements,nextobj,index;
            BINT count=0;

            while( (objptr<endofobjects)&&(count<totalelements)) {
                *ptr=objptr-matrix;
                ++ptr;
                ++count;
                objptr=rplSkipOb(objptr);
            }

            if( (count!=totalelements)||(objptr!=endofobjects)) {
                // MALFORMED MATRIX IS MISSING OBJECTS
                RetNum=ERR_INVALID;
                return;
            }

            // COMPACT MATRIX BY REMOVING DUPLICATED OBJECTS
            index=matrix+2;
            objptr=matrix+2+totalelements;

            while(objptr<endofobjects) {
                // CHECK AND REMOVE DUPLICATES OF CURRENT OBJECT
                ptr=rplSkipOb(objptr);
                while(ptr<endofobjects) {
                    if(rplCompareObjects(ptr,objptr)) {
                        // OBJECTS ARE IDENTICAL, REMOVE

                        // REPLACE ALL REFERENCES TO THIS COPY WITH REFERENCES TO THE ORIGINAL
                        for(count=0;count<totalelements;++count) if(index[count]==ptr-matrix) index[count]=objptr-matrix;

                        // AND REMOVE THE COPY
                        nextobj=rplSkipOb(ptr);
                        if(nextobj<endofobjects) {
                            // THERE'S MORE OBJECTS, MOVE ALL MEMORY AND FIX ALL INDICES
                            memmovew(ptr,nextobj,endofobjects-nextobj);
                            for(count=0;count<totalelements;++count) if(index[count]>ptr-matrix) index[count]-=nextobj-ptr;
                        }
                        endofobjects-=nextobj-ptr;
                        rplCompileRemoveWords(nextobj-ptr);
                        // DO NOT ADVANCE ptr, SINCE OBJECTS MOVED
                    }
                    else ptr=rplSkipOb(ptr);
                    }
                objptr=rplSkipOb(objptr);
                }


            RetNum=OK_ENDCONSTRUCT;
            return;
        }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

        return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors



        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendString((BYTEPTR)"[ ");
            BINT rows=MATROWS(*(DecompileObject+1)),cols=MATCOLS(*(DecompileObject+1));
            BINT doublebracket=rows;

            if(!rows) ++rows;

            // SCAN THE INDEX AND OUTPUT ALL OBJECTS INSIDE
            BINT i,j;

            for(i=0;i<rows;++i)
            {
                if(doublebracket) rplDecompAppendString((BYTEPTR)"[ ");
                if(Exceptions) { RetNum=ERR_INVALID; return; }
                for(j=0;j<cols;++j)
                {
                    BINT offset=*(DecompileObject+2+i*cols+j);

                    rplDecompile(DecompileObject+offset,DECOMP_EMBEDDED | ((CurOpcode==OPCODE_DECOMPEDIT)? DECOMP_EDIT:0));    // RUN EMBEDDED
                 if(Exceptions) { RetNum=ERR_INVALID; return; }
                 rplDecompAppendChar(' ');
                }
                if(doublebracket) rplDecompAppendString((BYTEPTR)"] ");
                if(Exceptions) { RetNum=ERR_INVALID; return; }
            }

            rplDecompAppendChar(']');
            if(Exceptions) { RetNum=ERR_INVALID; return; }

            RetNum=OK_CONTINUE;
            return;



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

        // FIRST, CHECK THAT THE OBJECT IS ALLOWED WITHIN AN ARRAY
    {

        if(! (ISNUMBERCPLX(*LastCompiledObject)
              || ISSYMBOLIC(*LastCompiledObject)
              || ISIDENT(*LastCompiledObject))) {
                RetNum=ERR_INVALID;
                return;
            }

        WORDPTR matrix=*(ValidateTop-1);
        if(LastCompiledObject==matrix+1) {
            // THIS IS THE FIRST OBJECT IN THE ARRAY
            // ADD A DUMMY WORD
            rplCompileAppend(0);
            // MOVE THE FIRST OBJECT UP IN MEMORY TO MAKE ROOM FOR THE SIZE WORD
            memmovew(LastCompiledObject+1,LastCompiledObject,CompileEnd-1-LastCompiledObject);

            matrix[1]=MATMKSIZE(0,1);

        }

        else {
            // IF THIS IS THE FIRST ROW, INCREASE THE COLUMN COUNT
            BINT dimlevel=OBJSIZE(CurrentConstruct);
            BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
            
            if(rows) {
                // THIS IS A MATRIX, ONLY ALLOW ELEMENTS INSIDE LEVEL 1
                if(!dimlevel) {
                    RetNum=ERR_INVALID;
                    return;
                }
            } else {
                // THIS IS A VECTOR, ONLY ALLOW ELEMENTS IN THE OUTER LEVEL
                if(dimlevel) {
                    RetNum=ERR_INVALID;
                    return;
                }

            }
            if(rows<=1) { matrix[1]=MATMKSIZE(rows,cols+1); }

        }



        RetNum=OK_CONTINUE;
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
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;


 }



