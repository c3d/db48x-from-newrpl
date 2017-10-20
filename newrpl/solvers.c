/*
 * Copyright (c) 2014-2017, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"

// THIS MODULE ORGANIZES THE NUMERIC SOLVERS



// EVALUATE POLYNOMIAL EXPLODED IN THE STACK, USING HORNER'S METHOD
// first POINTS TO THE HIGHEST DEGREE COEFFICIENT EXPLODED IN THE STACK
// value POINTS TO THE VALUE, IN ANY LOCATION IN THE STACK
// COEFFICIENTS CAN BE REAL, COMPLEX OR SYMBOLIC

// LOW-LEVEL - NO CHECKS DONE HERE

WORDPTR rplPolyEvalEx(WORDPTR *first,BINT degree,WORDPTR *value)
{
    BINT k;

    rplPushData(first[0]);
    for(k=1;k<=degree;++k)
    {
        rplPushData(*value);
        rplCallOvrOperator(CMD_OVR_MUL);
        if(Exceptions) return 0;
        rplPushData(first[k]);
        rplCallOvrOperator(CMD_OVR_ADD);
        if(Exceptions) return 0;
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions) return 0;
        }
    }
    return rplPopData();

}

// EVALUATE POLYNOMIAL DERIVATIVE FROM COEFFICIENTS EXPLODED IN THE STACK, USING HORNER'S METHOD
// deriv=1 FOR FIRST DERIVATIVE, 2 FOR SECOND, ETC.
// first POINTS TO THE HIGHEST DEGREE COEFFICIENT EXPLODED IN THE STACK
// value POINTS TO THE VALUE, IN ANY LOCATION IN THE STACK
// COEFFICIENTS CAN BE REAL, COMPLEX OR SYMBOLIC

// LOW-LEVEL - NO CHECKS DONE HERE

WORDPTR rplPolyEvalDerivEx(BINT deriv,WORDPTR *first,BINT degree,WORDPTR *value)
{
    BINT k,j,c;

    rplPushData(first[0]);
    for(c=degree,j=1;j<deriv;++j) c*=(c-j);
    rplNewBINTPush(c,DECBINT);
    if(Exceptions) return 0;
    rplCallOvrOperator(CMD_OVR_MUL);
    if(Exceptions) return 0;

    for(k=1;k<=degree-deriv;++k)
    {
        rplPushData(*value);
        rplCallOvrOperator(CMD_OVR_MUL);
        if(Exceptions) return 0;
        rplPushData(first[k]);
        for(c=degree-k,j=1;j<deriv;++j) c*=(c-j);
        rplNewBINTPush(c,DECBINT);
        if(Exceptions) return 0;
        rplCallOvrOperator(CMD_OVR_MUL);
        if(Exceptions) return 0;
        rplCallOvrOperator(CMD_OVR_ADD);
        if(Exceptions) return 0;
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions) return 0;
        }
    }
    return rplPopData();

}

// FIND ONE ROOT OF NUMERIC POLYNOMIAL
// MAKE SURE ALL COEFFICIENTS ARE NUMERIC (REAL OR COMPLEX) - NO CHECKS DONE HERE
// USES LAGUERRE METHOD, IT FINDS COMPLEX ROOTS
// CALLER NEEDS TO SET FLAGS: COMPLEX MODE AND DON'T ERROR ON INFINITE RESULT TO AVOID PREMATURE EXIT

WORDPTR rplPolyRootEx(WORDPTR *first,BINT degree)
{
    BINT k;

    // EXTRACT THE MAGNITUDE OF ALL COEFFICIENTS
    for(k=0;k<=degree;++k)
    {
        rplPushData(first[k]);
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions) return 0;
    }
    // UPPER BOUND= 1+1/|an|*max(a0...a(n-1)) [THIS IS THE CAUCHY BOUND]

    WORDPTR *firstabs=DSTop-(degree+1);

    // GET THE MAXIMUM VALUE
    for(k=1;k<=degree;++k)
    {
        rplPushData(firstabs[k]);
        if(k>1) {
            rplCallOperator(CMD_MAX);
            if(Exceptions) return 0;
        }
    }
    rplPushData(firstabs[0]);   // an COEFFICIENT
    rplCallOvrOperator(CMD_OVR_DIV);
    if(Exceptions) return 0;
    rplPushData((WORDPTR)one_bint);
    rplCallOvrOperator(CMD_OVR_ADD);
    if(Exceptions) return 0;

    // USE HALF OF THAT CIRCLE AS INITIAL GUESS
    newRealFromBINT(&RReg[0],5,-1);
    rplNewRealFromRRegPush(0);
    if(Exceptions) return 0;
    rplCallOvrOperator(CMD_OVR_MUL);
    firstabs[0]=(WORDPTR)one_bint;
    firstabs[1]=rplPeekData(1);
    DSTop=firstabs+2;           //    DROP ALL OTHER INTERMEDIATE VALUES

    // HERE WE HAVE THE POLYNOMIAL EXPLODED AND AN INITIAL GUESS

    BINT oldprec=Context.precdigits;
    WORDPTR pk;
    REAL err;
    // TEMPORARILY INCREASE PRECISION
    if(Context.precdigits<=REAL_PRECISION_MAX-8) Context.precdigits+=8;

    do {

        rplPushData(firstabs[0]);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplReadNumberAsReal(rplPopData(),&err);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        if(intdigitsReal(&err)<-(oldprec/2+8))
            break;    // FOUND A ROOT!

        pk=rplPolyEvalEx(first,degree,firstabs+1);
        if(!pk) { Context.precdigits=oldprec; return 0; }
        rplPushData(pk);
        // ALSO CHECK FOR ZERO pk() SINCE IT CAN BLOW THE ALGORITHM
        rplPushData(rplPeekData(1));
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplReadNumberAsReal(rplPopData(),&err);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        if(iszeroReal(&err) || (intdigitsReal(&err)<-(2*oldprec)))
        { rplDropData(1); break; }    // FOUND A ROOT!

        //   HERE WE HAVE pk ON THE STACK, COMPUTE p'k and p''k

        pk=rplPolyEvalDerivEx(1,first,degree,firstabs+1);
        if(!pk) { Context.precdigits=oldprec; return 0; }
        rplPushData(pk);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplPushData(rplPeekData(2));
        rplCallOvrOperator(CMD_OVR_DIV);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }

        // HERE G=p'k/pk IS IN LEVEL ONE

        rplPushData(rplPeekData(1));
        rplPushData(rplPeekData(1));
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplCallOvrOperator(CMD_OVR_MUL);    // G^2
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplPushData(rplPeekData(1));
        if(Exceptions) { Context.precdigits=oldprec; return 0; }

        pk=rplPolyEvalDerivEx(2,first,degree,firstabs+1);
        if(!pk) { Context.precdigits=oldprec; return 0; }
        rplPushData(pk);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplPushData(rplPeekData(5));        // HERE WE HAVE pk,G,G^2,G^2, p''k AND NOW pk
        rplCallOvrOperator(CMD_OVR_DIV);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplCallOvrOperator(CMD_OVR_SUB);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }

        // HERE WE HAVE pk, G, G^2 AND H ON THE STACK

        rplNewBINTPush(degree,DECBINT);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplOverwriteData(5,rplPeekData(1)); // SAVE n FOR LATER, OVERWRITE pk
        rplCallOvrOperator(CMD_OVR_MUL);    // n*H
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplCallOvrOperator(CMD_OVR_SUB);    // G^2-n*H
        if(Exceptions) { Context.precdigits=oldprec; return 0; }

        rplNewBINTPush(1-degree,DECBINT);   // -(n-1) = (1-n)
        if(Exceptions) { Context.precdigits=oldprec; return 0; }

        rplCallOvrOperator(CMD_OVR_MUL);    // (n-1)*(n*H-G^2)
        if(Exceptions) { Context.precdigits=oldprec; return 0; }

        rplCallOperator(CMD_SQRT);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }            // THIS COULD TRIGGER "ARGUMENT OUT OF BOUNDS" IF COMPLEX MODE IS NOT ENABLED

        // HERE WE HAVE n, G, SQRT( (n-1)*(n*H-G^2) ) ON THE STACK


        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplCallOvrOperator(CMD_OVR_ADD);        // G+SQRT(...)
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplPushData(rplPeekData(3));
        rplPushData(rplPeekData(3));
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplCallOvrOperator(CMD_OVR_SUB);        // G-SQRT(...)
        if(Exceptions) { Context.precdigits=oldprec; return 0; }

        rplPushData(rplPeekData(2));
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplPushData(rplPeekData(2));
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        rplCallOvrOperator(CMD_OVR_GT);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        // PICK THE MAX(ABS(G+SQRT(...)) , ABS(G-SQRT(...))
            // HERE WE HAVE n, G, SQRT( (n-1)*(n*H-G^2) )
            // G+SQRT(...) , G-SQRT(...), ON THE STACK
        rplOverwriteData(4,rplPeekData(rplIsFalse(rplPopData())? 1:2));
        rplDropData(3);

        // LEAVE n, G(+/-)SQRT(...)
        if(rplIsFalse(rplPeekData(1))) {
            // CAN'T DIVIDE BY ZERO, THIS CAN ONLY HAPPEN IF G==0 AND H==0
            // THIS MAY HAPPEN AT A LOCAL EXTREMUM
            // NOT SURE WHAT TO DO, FOR NOW JUST SET a=1000*error TO MOVE IT TO A DIFFERENT POINT
            rplDropData(2);
            newRealFromBINT(&RReg[0],1,-Context.precdigits+12);
            rplNewRealFromRRegPush(0);
        } else rplCallOvrOperator(CMD_OVR_DIV);

        if(Exceptions) { Context.precdigits=oldprec; return 0; }
        // FINALLY, WE HAVE x AND a ON THE STACK
        firstabs[0]=rplPeekData(1);
        rplCallOvrOperator(CMD_OVR_SUB);
        if(Exceptions) { Context.precdigits=oldprec; return 0; }

    } while(1); // ADD SOME LOOP LIMIT HERE IN CASE IT DOESN'T CONVERGE!

    Context.precdigits=oldprec;
    pk=rplPeekData(1);
    rplDropData(2);
    return pk;
}


// DEFLATE POLYNOMIAL EXPLODED IN THE STACK, USING HORNER'S METHOD
// first POINTS TO THE HIGHEST DEGREE COEFFICIENT EXPLODED IN THE STACK
// value POINTS TO THE ROOT VALUE, IN ANY LOCATION IN THE STACK
// COEFFICIENTS CAN BE REAL, COMPLEX OR SYMBOLIC
// MODIFIES THE ORIGINAL COEFFICIENTS ON THE STACK
// THE LAST COEFFICIENT IS THE REMAINDER, WHICH IS ALSO RETURNED

// LOW-LEVEL - NO CHECKS DONE HERE

WORDPTR rplPolyDeflateEx(WORDPTR *first,BINT degree,WORDPTR *value)
{
    BINT k;

    rplPushData(first[0]);
    for(k=1;k<=degree;++k)
    {
        rplPushData(*value);
        rplCallOvrOperator(CMD_OVR_MUL);
        if(Exceptions) return 0;
        rplPushData(first[k]);
        rplCallOvrOperator(CMD_OVR_ADD);
        if(Exceptions) return 0;
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions) return 0;
        }
        first[k]=rplPeekData(1);
    }
    return rplPopData();

}
