
#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// LIB 66 PROVIDES COMMANDS THAT DEAL WITH TRANSCENDENTAL FUNCTIONS

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  66
#define LIB_ENUM lib66_enum
#define LIB_NAMES lib66_names
#define LIB_HANDLER lib66_handler
#define LIB_NUMBEROFCMDS LIB66_NUMBEROFCMDS

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(SIN), \
    CMD(COS), \
    CMD(TAN), \
    CMD(ASIN), \
    CMD(ACOS), \
    CMD(ATAN), \
    CMD(ATAN2), \
    CMD(LN), \
    CMD(EXP), \
    CMD(SINH), \
    CMD(COSH), \
    CMD(TANH), \
    CMD(ASINH), \
    CMD(ACOSH), \
    CMD(ATANH)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
/*#define CMD_EXTRANAME \
    "VOID",

#define CMD_EXTRAENUM \
    VOID
*/

// INTERNAL DECLARATIONS


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST /*, CMD_EXTRAENUM */, LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
char *LIB_NAMES[]= { CMD_LIST /*, CMD_EXTRANAME*/ };
#undef CMD


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case SIN:
    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        trig_sincos(&dec);
        RReg[1].exp+=Context.prec;
        mpd_round_to_intx(&RReg[7],&RReg[1],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=Context.prec;
        mpd_reduce(&RReg[1],&RReg[7],&Context);

        rplDropData(1);
        rplRRegToRealPush(1);       // SIN
        return;

    }
    case COS:
    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        trig_sincos(&dec);
        RReg[0].exp+=Context.prec;
        mpd_round_to_intx(&RReg[7],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=Context.prec;
        mpd_reduce(&RReg[0],&RReg[7],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);       // COS
        return;

    }
    case TAN:
    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        trig_sincos(&dec);
        RReg[0].exp+=Context.prec;
        mpd_round_to_intx(&RReg[6],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[6].exp-=Context.prec;
        RReg[0].exp-=Context.prec;
        if(mpd_iszero(&RReg[6])) {
            mpd_set_infinity(&RReg[2]);
            if(mpd_isnegative(&RReg[1]))  mpd_set_negative(&RReg[2]);
        }
        else {
            mpd_div(&RReg[2],&RReg[1],&RReg[0],&Context);
        }
        rplDropData(1);
        rplRRegToRealPush(2);       // SIN/COS

        return;

    }
    case ASIN:
        // CALCULATE ASIN FROM ASIN(Y) = 2* ATAN2(1+SQRT(1-Y^2),Y)
    {
        mpd_t y;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);
        mpd_mul(&RReg[6],&y,&y,&Context);
        if(mpd_cmp(&RReg[6],&RReg[7],&Context)==1) {
            // TODO: INCLUDE COMPLEX ARGUMENTS HERE
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }
        mpd_sub(&RReg[5],&RReg[7],&RReg[6],&Context);
        mpd_sqrt(&RReg[6],&RReg[5],&Context);
        mpd_add(&RReg[5],&RReg[6],&RReg[7],&Context);

        trig_atan2(&RReg[5],&y);
        mpd_add(&RReg[7],&RReg[0],&RReg[0],&Context);
        RReg[7].exp+=Context.prec;
        mpd_round_to_intx(&RReg[6],&RReg[7],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[6].exp-=Context.prec;
        mpd_reduce(&RReg[0],&RReg[6],&Context);
        rplDropData(1);
        rplRRegToRealPush(0);       // RESULTING ANGLE

        return;

    }

    case ACOS:
        // CALCULATE ASIN FROM ACOS(X) = 2* ATAN2(SQRT(1-X^2),1+X)
    {
        mpd_t y;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);

        mpd_add(&RReg[4],&y,&RReg[7],&Context);

        mpd_mul(&RReg[6],&y,&y,&Context);
        if(mpd_cmp(&RReg[6],&RReg[7],&Context)==1) {
            // TODO: INCLUDE COMPLEX ARGUMENTS HERE
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }
        mpd_sub(&RReg[5],&RReg[7],&RReg[6],&Context);
        mpd_sqrt(&RReg[6],&RReg[5],&Context);

        trig_atan2(&RReg[6],&RReg[4]);
        mpd_add(&RReg[7],&RReg[0],&RReg[0],&Context);
        RReg[7].exp+=Context.prec;
        mpd_round_to_intx(&RReg[6],&RReg[7],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[6].exp-=Context.prec;
        mpd_reduce(&RReg[0],&RReg[6],&Context);
        rplDropData(1);
        rplRRegToRealPush(0);       // RESULTING ANGLE

        return;

    }



    case ATAN:
        // CALCULATE ATAN FROM ATAN2(Y,1)
    {
        mpd_t y;
        if(rplDepthData()<2) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);


        trig_atan2(&RReg[7],&y);
        RReg[0].exp+=Context.prec;
        mpd_round_to_intx(&RReg[6],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[6].exp-=Context.prec;
        mpd_reduce(&RReg[0],&RReg[6],&Context);
        \
        rplDropData(1);
        rplRRegToRealPush(0);       // RESULTING ANGLE

        return;

    }
    case ATAN2:
        // CALCULATE ATAN IN THE RANGE -PI,+PI
    {
        mpd_t y,x;
        if(rplDepthData()<2) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&y);
        rplReadReal(rplPeekData(2),&x);

        if(Exceptions) return;

        trig_atan2(&x,&y);
        RReg[0].exp+=Context.prec;
        mpd_round_to_intx(&RReg[6],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[6].exp-=Context.prec;
        mpd_reduce(&RReg[0],&RReg[6],&Context);

        rplDropData(2);
        rplRRegToRealPush(0);       // RESULTING ANGLE

        return;

    }

    case LN:
    {
        mpd_t x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&x);

        if(Exceptions) return;

        mpd_ln(&RReg[0],&x,&Context);
        if(Exceptions) return;

        rplDropData(1);
        rplRRegToRealPush(0);
        return;

    }

    case EXP:
    {
        mpd_t x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&x);

        if(Exceptions) return;

        mpd_exp(&RReg[0],&x,&Context);
        if(Exceptions) return;

        rplDropData(1);
        rplRRegToRealPush(0);
        return;

    }
    case SINH:
    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);

        BINT exponent=RReg[7].exp;
        RReg[7].exp=Context.prec-RReg[7].digits;
        mpd_round_to_intx(&RReg[2],&RReg[7],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[2].exp=exponent+(RReg[7].digits-RReg[2].digits);
        mpd_reduce(&RReg[0],&RReg[2],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);       // SINH
        return;

    }

    case COSH:
    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);
        RReg[6].exp+=Context.prec;
        mpd_round_to_intx(&RReg[1],&RReg[6],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[1].exp-=Context.prec;
        mpd_reduce(&RReg[0],&RReg[1],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);       // COSH
        return;

    }

    case TANH:

    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);

        // TANH=SINH/COSH
        mpd_div(&RReg[0],&RReg[7],&RReg[6],&Context);
        RReg[0].exp+=Context.prec;
        mpd_round_to_intx(&RReg[7],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=Context.prec;
        mpd_reduce(&RReg[0],&RReg[7],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);       // TANH
        return;

    }


    case ASINH:
    case ACOSH:
    case ATANH:
        // IMPLEMENTED USING THE DEFINITION: atanh(x)= 1/2*ln( (1+x)/(1-x) )
    {
        mpd_t x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&x);

        if(Exceptions) return;

        rplOneToRReg(0);
        mpd_sub(&RReg[2],&RReg[0],&x,&Context); // 1-X

        if(mpd_iszero(&RReg[2])) {
            // TODO: IMPLEMENT INFINITY FLAGS TO THROW EXCEPTION ON INFINITY
            mpd_set_infinity(&RReg[0]);
            if(mpd_isnegative(&x)) mpd_set_negative(&RReg[0]);

            rplDropData(1);
            rplRRegToRealPush(0);
            return;
        }

        mpd_add(&RReg[1],&x,&RReg[0],&Context); // X+1
        mpd_div(&RReg[0],&RReg[1],&RReg[2],&Context);

        if(mpd_isnegative(&RReg[0])) {
            // TODO: ADD COMPLEX RESULTS HERE
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }

        mpd_ln(&RReg[1],&RReg[0],&Context);
        rplBINTToRReg(0,5);     // RREG = 5
        RReg[0].exp--;          // MAKE IT 0.5
        mpd_mul(&RReg[2],&RReg[0],&RReg[1],&Context);

        rplDropData(1);
        rplRRegToRealPush(2);
        return;

    }


    return;

    // ADD MORE OPCODES HERE

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
        libCompileCmds(LIBRARY_NUMBER,LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

     return;

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds(LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // TokenStart = token string
        // TokenLen = token length
        // ArgNum2 = Opcode/WORD of object

        // VALIDATE RETURNS:
        // RetNum =  enum CompileErrors



        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;

}




