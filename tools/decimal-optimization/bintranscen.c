
#include <newrpl.h>

#include <bindecimal.h>

#define MACROZeroToRReg(n) { RReg[n].data[0]=0; RReg[n].exp=0; RReg[n].flags=0; RReg[n].len=1; }
#define MACROOneToRReg(n) { RReg[n].data[0]=1; RReg[n].exp=0; RReg[n].flags=0; RReg[n].len=1; }
#define MACRONANToRReg(n) { RReg[n].data[0]=0; RReg[n].exp=0; RReg[n].flags=F_NOTANUMBER; RReg[n].len=1; }



// RETURN ATAN(1*10^-exponent) IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/9) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS

static void binatan_table(int exponent,int sysexp,REAL *real)
{

    // WARNING: 0<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>=CORDIC_MAXSYSEXP/2) {

        int pos=CORDIC_MAXSYSEXP-sysexp;
        int words=(pos)>>5;
        zero_words(real->data,CORDIC_TABLEWORDS-words);
        real->len=CORDIC_TABLEWORDS-words;
        if(exponent&31) real->data[CORDIC_TABLEWORDS-words-1]=1LL<<(32-(exponent&31));
        else { real->data[CORDIC_TABLEWORDS-words]=1; ++real->len; }
        real->exp=0;
        real->flags=0;
        return;
    }

    //uint8_t *byte=(uint8_t *)&(atan_1_8_stream[atan_1_8_offsets[exponent]]);
    // TODO: ADD TABLE COMPRESSION

    BINT *tabledata=(BINT *)atan_binary+exponent*CORDIC_TABLEWORDS;

    int pos=CORDIC_MAXSYSEXP-sysexp;
    int words=(pos)>>5;
    copy_words(real->data,tabledata+words,CORDIC_TABLEWORDS-words);
    real->len=CORDIC_TABLEWORDS-words;
    //decompress_number(byte,(uint32_t *)atan_1_8_dict,(uint32_t *)real->data,words);
    real->exp=0;
    real->flags=0;

}


static void binconst_K_table(int exponent,int sysexp,REAL *real)
{

    // WARNING: 0<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>=CORDIC_MAXSYSEXP/2) exponent=CORDIC_MAXSYSEXP/2;


    //uint8_t *byte=(uint8_t *)&(atan_1_8_stream[atan_1_8_offsets[exponent]]);
    // TODO: ADD TABLE COMPRESSION

    BINT *tabledata=(BINT *)K_binary+(CORDIC_MAXSYSEXP/2-1-exponent)*CORDIC_TABLEWORDS+((CORDIC_MAXSYSEXP-sysexp)>>5);

    copy_words(real->data,tabledata,sysexp>>5);
    real->exp=0;
    real->len=sysexp>>5;
    real->flags=0;

    //decompress_number(byte,(uint32_t *)atan_1_8_dict,(uint32_t *)real->data,words);

}

// CONSTANTS EMBEDDED IN ROM, DO NOT USE WITH RReg
void decconst_2Sysexp(REAL *real,int sysexp)
{
    real->len=two_exp_offset[(sysexp>>5)-1];
    real->data=(BINT *)two_exp_binary+(real->len&0xffff);
    real->len>>=16;
    real->exp=0;
    real->flags=0;
}



// CORDIC LOOP IN ROTATIONAL MODE

// BINARY CORDIC ROUTINE DROP-IN REPLACEMENT


// TAKES INITIAL PARAMETERS IN RREG[0], RREG[1] AND RREG[2]
// RETURNS RESULTS IN RREG[5], RREG[6], RREG[7]


static void binCORDIC_Rotational(int digits,int startindex,int sysexp)
{
int exponent;
REAL *x,*y,*z,*tmp;
REAL *xnext,*ynext,*znext;

z=&RReg[0];
x=&RReg[1];
y=&RReg[2];
znext=&RReg[5];
xnext=&RReg[6];
ynext=&RReg[7];

digits=((digits+16)*217706)>>17; // CONVERT TO BASE-2 ITERATIONS
digits&=~1; // GUARANTEE AN EVEN NUMBER OF ITERATIONS
for(exponent=startindex;exponent<startindex+digits;++exponent)
{

    if(!(z->flags&F_NEGATIVE)) {
        y->flags^=F_NEGATIVE;
        bIntegerAddShift(xnext,x,y,exponent+startindex);
        y->flags^=F_NEGATIVE;
        bIntegerAddShift(ynext,y,x,exponent-startindex);
    }
    else {
        bIntegerAddShift(xnext,x,y,exponent+startindex);
        x->flags^=F_NEGATIVE;
        bIntegerAddShift(ynext,y,x,exponent-startindex);
        x->flags^=F_NEGATIVE;
    }


    binatan_table(exponent,sysexp,&RReg[4]);    // RReg[4]=atan(2^(-exp));


    if(!(z->flags&F_NEGATIVE)) {
        RReg[4].flags=F_NEGATIVE;
        bIntegerAdd(znext,z,&RReg[4]);
    } else bIntegerAdd(znext,z,&RReg[4]);


    if( (exponent&31)==31) {
        copy_words(znext->data+1,znext->data,znext->len);
        znext->data[0]=0;
        znext->len++;
    }

    // WE FINISHED ONE STEP
    // SWAP THE POINTERS TO AVOID COPYING THE NUMBERS
    tmp=znext;
    znext=z;
    z=tmp;

    tmp=xnext;
    xnext=x;
    x=tmp;

    tmp=ynext;
    ynext=y;
    y=tmp;
}


// FINAL ROTATION BY RESIDUAL ANGLE
// Xn=X-Y*tan(Ang)=X-Y*Ang
// Yn=Y+X*tan(Ang)=Y+X*Ang

bIntegerMul(ynext,y,z,sysexp+((exponent>>5)+(startindex>>5))*32);
bIntegerMul(znext,x,z,sysexp+((exponent>>5)-(startindex>>5))*32);
ynext->flags^=F_NEGATIVE;
bIntegerAdd(xnext,x,ynext);
bIntegerAdd(ynext,y,znext);

// THE FINAL RESULTS ARE ALWAYS IN RREG[6] AND RREG[7]

}



// DROP-IN REPLACEMENT FOR trig_sincos() BUT USING BINARY INTEGER MATH FOR THE CORDIC LOOP
// CALCULATE RReg[6]=cos(angle) and RReg[7]=sin(angle) BOTH WITH 8 DIGITS MORE THAN CURRENT SYSTEM PRECISION (ABOUT 6 OF THEM ARE GOOD DIGITS, ROUNDING IS NEEDED)
// angmode = one of ANGLERAD, ANGLEDEG, ANGLEGRAD or ANGLEDMS constants
void bintrig_sincos(REAL *angle, BINT angmode)
{
    int negsin,negcos,swap,startexp;
    REAL pi,pi2,pi4,two_sysexp;
    BINT savedprec;

    negcos=negsin=swap=0;

    savedprec=Context.precdigits;
    Context.precdigits=(2*savedprec+8 > REAL_PRECISION_MAX)? REAL_PRECISION_MAX:(2*savedprec+8);
    if(angle->exp>savedprec) {
        // THIS IS A VERY LARGE ANGLE, NEED TO INCREASE THE PRECISION
        // TO GET AN ACCURATE RESULT ON THE MODULO
        BINT minprec=((savedprec+intdigitsReal(angle))+7)&(~7);
        if(minprec>REAL_PRECISION_MAX) {
            // TODO: ISSUE AN ERROR
            // FOR NOW JUST LEAVE IT WITH PARTIAL LOSS OF PRECISION
            minprec=REAL_PRECISION_MAX;
        }
        Context.precdigits=minprec;
    }

    decconst_PI(&pi);
    decconst_PI_2(&pi2);
    decconst_PI_4(&pi4);

    if(angmode==ANGLERAD) {
        // ANGLE IS IN RADIANS, NO NEED FOR CONVERSION
        copyReal(&RReg[0],angle);
        // GET ANGLE MODULO PI
        divmodReal(&RReg[1],&RReg[0],angle,&pi);
    }
    else {
        REAL convfactor;
        BINT modulo;
        if(angmode==ANGLEDMS) {
            // CONVERT TO DEGREES FIRST, SO THAT THERE'S EXACT VALUES AT 90, ETC.
            trig_convertangle(angle,ANGLEDMS,ANGLEDEG);

            swapReal(&RReg[0],&RReg[7]);
            angle=&RReg[7];

            angmode=ANGLEDEG;   // PLAIN DEGREES FROM NOW ON
        }
        if(angmode==ANGLEDEG) {
            // DEGREES
             decconst_PI_180(&convfactor);
             modulo=180;
        } else {
            // GRADS
             decconst_PI_200(&convfactor);
             modulo=200;
        }

        newRealFromBINT(&RReg[2],modulo,0);

        // GET ANGLE MODULO HALF-TURN
        divmodReal(&RReg[1],&RReg[0],angle,&RReg[2]);

        // CHECK FOR SPECIAL CASES: 1 FULL TURN AND HALF TURN

        if(iszeroReal(&RReg[0])) {
            // EXACT MULTIPLE OF PI, RETURN EXACT VALUES
            MACROZeroToRReg(7);
            MACROOneToRReg(6);
            if(isoddReal(&RReg[1])) RReg[6].flags|=F_NEGATIVE;
            // RESTORE PREVIOUS PRECISION
            Context.precdigits=savedprec;

            return;
        }
        RReg[2].data[0]>>=1; // 90 OR 100 DEGREES
        RReg[2].flags|=RReg[0].flags&F_NEGATIVE;

        if(eqReal(&RReg[0],&RReg[2])) {
            // EXACT PI/2 OR 3/2PI, RETURN EXACT VALUES
            MACROZeroToRReg(6);
            MACROOneToRReg(7);
            RReg[7].flags|=RReg[2].flags&F_NEGATIVE;
            if(isoddReal(&RReg[1])) RReg[7].flags^=F_NEGATIVE;

            // RESTORE PREVIOUS PRECISION
            Context.precdigits=savedprec;


            return;
        }





        // CONVERT TO RADIANS
        mulReal(&RReg[0],&RReg[0],&convfactor);


    }


    // HERE RReg[0] HAS THE REMAINDER THAT WE NEED TO WORK WITH


    // CHECK FOR SPECIAL CASES

    if(iszeroReal(&RReg[0])) {
        // EXACT MULTIPLE OF PI, IN RADIANS THIS CAN ONLY HAPPEN IF THE ARGUMENT IS ACTUALLY ZERO
        MACROZeroToRReg(7);
        MACROOneToRReg(6);
        if(isoddReal(&RReg[1])) RReg[6].flags|=F_NEGATIVE;
        // RESTORE PREVIOUS PRECISION
        Context.precdigits=savedprec;

        return;
    }

    // IF THE RESULT OF THE DIVISION IS ODD, THEN WE ARE IN THE OTHER HALF OF THE CIRCLE
    if(isoddReal(&RReg[1])) { negcos=negsin=1; }

    if(RReg[0].flags&F_NEGATIVE) { negsin^=1; RReg[0].flags&=~F_NEGATIVE; }

    if(gtReal(&RReg[0],&pi2)) {
        swap=1;
        negcos^=1;
        sub_real(&RReg[0],&RReg[0],&pi2);
    }
    if(gtReal(&RReg[0],&pi4)) {
        swap^=1;
        sub_real(&RReg[0],&pi2,&RReg[0]);
    }

    normalize(&RReg[0]);

    // LOAD CONSTANT 0.1
    RReg[7].data[0]=1;
    RReg[7].exp=-1;
    RReg[7].flags=0;
    RReg[7].len=1;

    if(ltReal(&RReg[0],&RReg[7])) {
        // WE ARE DEALING WITH SMALL ANGLES


        startexp=-RReg[0].exp-((RReg[0].len-1)<<3)-sig_digits(RReg[0].data[RReg[0].len-1])+1;

        if(startexp<=2) startexp=0; else startexp-=2;
    }
    else startexp=0;


    if(startexp>=savedprec) {
        // VERY SMALL ANGLES
        Context.precdigits=savedprec;

        if(swap) {
            // COS = 1
            RReg[7].data[0]=1;
            RReg[7].exp=0;
            RReg[7].flags=0;
            RReg[7].len=1;
            // SIN = ANGLE
            copyReal(&RReg[6],&RReg[0]);
        }
        else {
        // COS = 1
        RReg[6].data[0]=1;
        RReg[6].exp=0;
        RReg[6].flags=0;
        RReg[6].len=1;
        // SIN = ANGLE
        copyReal(&RReg[7],&RReg[0]);
        }
    }
    else {

    // USE RReg[0]=z; RReg[1]=x; RReg[2]=y;

    // DETERMINE NUMBER OF BINARY WORDS WE NEED TO USE
    int sysexp=((Context.precdigits+8)*217706)>>16;
    sysexp+=31;
    sysexp&=~31;
    if(sysexp>CORDIC_MAXSYSEXP) sysexp=CORDIC_MAXSYSEXP;
    int binstartexp=(startexp*217706)>>16;

    // CONVERSION CONSTANT TO INTEGER

    // CONVERT z TO BINARY
    mul_real(&RReg[1],&RReg[0],&two_sysexp);
    normalize(&RReg[1]);
    roundReal(&RReg[1],&RReg[1],0);
    ipReal(&RReg[1],&RReg[1],1);    // TAKE INTEGER PART AND JUSTIFY THE DIGITS
    bIntegerfromReal(&RReg[0],&RReg[1]);

    // y=0;
    RReg[2].len=sysexp>>5;
    zero_words(RReg[2].data,sysexp>>5);
    RReg[2].exp=0;
    RReg[2].flags=0;

    // x=2^sysexp;
    RReg[1].len=(sysexp>>5)+1;
    zero_words(RReg[1].data,sysexp>>5);
    RReg[1].data[sysexp>>5]=1;
    RReg[1].exp=0;
    RReg[1].flags=0;

    Context.precdigits=savedprec+8;







    binCORDIC_Rotational((Context.precdigits>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+8:Context.precdigits,binstartexp,sysexp);

    // HERE WE HAVE
    // USE RReg[5]=angle_error; RReg[6]=cos(z) RReg[7]=sin(z);
    binconst_K_table(binstartexp,sysexp,&RReg[4]);

    bIntegerMul(&RReg[1],&RReg[7],&RReg[4],sysexp);
    RealfrombInteger(&RReg[5],&RReg[1]);
    divReal(&RReg[7],&RReg[5],&two_sysexp);

    if(binstartexp) decconst_2Sysexp(&two_sysexp,sysexp);

    bIntegerMul(&RReg[1],&RReg[6],&RReg[4],sysexp);
    RealfrombInteger(&RReg[5],&RReg[1]);
    divReal(&RReg[6],&RReg[5],&two_sysexp);

    if(swap) swapReal(&RReg[6],&RReg[7]);

    // RESTORE PREVIOUS PRECISION
    Context.precdigits=savedprec;


    }

    if(negcos) RReg[6].flags|=F_NEGATIVE;
    if(negsin) RReg[7].flags|=F_NEGATIVE;

    // NUMBERS ARE NOT FINALIZED
    // HIGHER LEVEL ROUTINE MUST FINALIZE cos OR sin AS NEEDED
    // TO AVOID WASTING TIME
}
