#include <stdio.h>
#include <newrpl.h>

#include <time.h>
#include <bindecimal.h>

#define MAX_WORDS 256
#define TOTAL_BREGISTERS 10

#define MACROZeroToRReg(n) { RReg[n].data[0]=0; RReg[n].exp=0; RReg[n].flags=0; RReg[n].len=1; }
#define MACROOneToRReg(n) { RReg[n].data[0]=1; RReg[n].exp=0; RReg[n].flags=0; RReg[n].len=1; }
#define MACRONANToRReg(n) { RReg[n].data[0]=0; RReg[n].exp=0; RReg[n].flags=F_NOTANUMBER; RReg[n].len=1; }





WORD BRegData[MAX_WORDS*2*TOTAL_BREGISTERS];

REAL BReg[TOTAL_BREGISTERS];




WORD atanbin_table[MAX_WORDS*CORDIC_TABLESIZE];





int generate_atantable(void)
{

    // INITIALIZE REGISTERS STORAGE

    int k,j;
    BINT extranumber[REAL_REGISTER_STORAGE*2];



    initContext(2016);

    for(k=0;k<REAL_REGISTERS;++k) {
        RReg[k].data=allocRegister();
        newRealFromBINT(&RReg[k],0,0);
    }

    for(k=0;k<TOTAL_BREGISTERS;++k) {
        BReg[k].data=(BINT *)&(BRegData[k*MAX_WORDS*2]);
    }



    REAL one,two_6720,two_6688;

    two_6720.data=extranumber;
    two_6688.data=extranumber+REAL_REGISTER_STORAGE;

    decconst_One(&one);

    newRealFromBINT(&RReg[0],6720,0);
    newRealFromBINT(&RReg[1],2,0);

    Context.precdigits=2024;

    // COMPUTE THE CONSTANT 2^6720
    powReal(&two_6720,&RReg[1],&RReg[0]);

    newRealFromBINT(&RReg[1],2,0);
    newRealFromBINT(&RReg[0],6688,0);
    powReal(&two_6688,&RReg[1],&RReg[0]);

    Context.precdigits=2536;



    printf("#include <stdint.h>\n\n\n// Start of atan(2^-k) table generator!\n\n\n");


    printf("const uint32_t const atan_binary[%d*%d]= {\n",CORDIC_TABLESIZE,CORDIC_TABLEWORDS);


    // START FROM 2^0 = 1
    newRealFromBINT(&RReg[8],1,0);

    int shift_table[CORDIC_TABLESIZE];
    int shift=0;

    for(k=0;k<CORDIC_TABLESIZE;++k)
    {
        // COMPUTE ATAN FROM SERIES X-X^3/3+X^5/5-X^7/7 USING HORNER
        if(k>0) {
        Context.precdigits=2536;
        MACROOneToRReg(2);

        int nterms=1+CORDIC_MAXSYSEXP/k;
        if(nterms<5) nterms=5;
        if(!(nterms&1)) ++nterms;
        //nterms+=50;  // MAKE SURE WE HAVE ENOUGH

        newRealFromBINT(&RReg[1],nterms,0);
        divReal(&RReg[0],&RReg[2],&RReg[1]);

        mulReal(&RReg[4],&RReg[8],&RReg[8]);    // X^2



        for(j=nterms-2;j>0;j-=2)
        {
            mulReal(&RReg[1],&RReg[0],&RReg[4]);    // R(j-1)*x^2
            newRealFromBINT(&RReg[5],j,0);
            divReal(&RReg[3],&RReg[2],&RReg[5]);    // 1/j
            if(j&2) RReg[3].flags|=F_NEGATIVE;
            addReal(&RReg[0],&RReg[1],&RReg[3]);    // R(j)=R(j-1)*x^2 (+/-) 1/j
        }

        mulReal(&RReg[0],&RReg[0],&RReg[8]);    // R(j)*=x

        }

        else {
        trig_atan2(&RReg[8],&one,ANGLERAD);

        // HERE RREG[0] HAS THE RESULT WITH EXTRA DIGITS
        normalize(&RReg[0]);
        }


        Context.precdigits=2536;
        // CONVERT TO AN INTEGER
        mulReal(&RReg[1],&RReg[0],&two_6720);
        roundReal(&RReg[1],&RReg[1],0);
        ipReal(&RReg[2],&RReg[1],1);    // TAKE INTEGER PART AND JUSTIFY THE DIGITS

        if(ltReal(&RReg[1],&two_6688)) {
        newRealFromBINT64(&RReg[3],4294967296L,0);
        mul_real(&RReg[1],&two_6720,&RReg[3]);   // INCREASE THE SHIFT by 32 bits
        normalize(&RReg[1]);
        shift+=32;
        copyReal(&two_6720,&RReg[1]);

        // CONVERT TO AN INTEGER AGAIN WITH THE NEW SHIFT
        mulReal(&RReg[1],&RReg[0],&two_6720);
        roundReal(&RReg[1],&RReg[1],0);
        ipReal(&RReg[2],&RReg[1],1);    // TAKE INTEGER PART AND JUSTIFY THE DIGITS

        }

        shift_table[k]=shift;

        Context.precdigits=2016;

        // THEN CONVERT TO BINARY FORM

        bIntegerfromReal(&BReg[0],&RReg[2]);

        // CHEAT FOR k==0
        if(k==0) BReg[0].data[0]=2751691783U;   // WE DON'T HAVE SUFFICIENT PRECISION TO COMPUTE THIS FIRST ONE ACCURATELY


        while(BReg[0].data[BReg[0].len-1]==0) --BReg[0].len;

        if(BReg[0].len>CORDIC_TABLEWORDS) {
            printf("Bad number of words!");
            return 0;
        }

        // AND STORE ALL 210 WORDS

        printf("/* atan(2^(-%d)) */\n",k);

        for(j=0;j<BReg[0].len;++j) {
            atanbin_table[k*MAX_WORDS+j]=BReg[0].data[j];
            printf("%uU%c",atanbin_table[k*MAX_WORDS+j],( (j==CORDIC_TABLEWORDS-1)&&(k==CORDIC_TABLESIZE-1))? ' ':',');
            if(j>0 && ((j%21)==20)) printf("\n");
        }
        for(;j<CORDIC_TABLEWORDS;++j) {
            atanbin_table[k*MAX_WORDS+j]=0;
            printf("0U%c",( (j==CORDIC_TABLEWORDS-1)&&(k==CORDIC_TABLESIZE-1))? ' ':',');
            if(j>0 && ((j%21)==20)) printf("\n");
        }


        Context.precdigits=2536;


        // HALF THE ANGLE AND GO AGAIN
        newRealFromBINT(&RReg[1],5,-1);
        mulReal(&RReg[8],&RReg[8],&RReg[1]);

    }

    // BINARY CORDIC TABLE READY
    printf("\n\n};\n\n");


    // OUTPUT SHIFT TABLE
/*
    // NO NEED FOR A SHIFT TABLE, USE shift=INDEX>>5;
    printf("const uint32_t const atan_shift[%d]= {\n",CORDIC_TABLESIZE);

    for(k=0;k<CORDIC_TABLESIZE;++k)
    {
        printf("%d%c%c",shift_table[k],(k==CORDIC_TABLESIZE-1)? ' ':',' , (((k&31)==0)&&(k>0))? '\n':' ');
    }

    printf("\n};\n\n\n");
*/
    return 0;
}




int generate_Ktable(void)
{

    // INITIALIZE REGISTERS STORAGE

    int k,j;
    BINT extranumber[REAL_REGISTER_STORAGE];



    initContext(2016);

    for(k=0;k<REAL_REGISTERS;++k) {
        RReg[k].data=allocRegister();
        newRealFromBINT(&RReg[k],0,0);
    }

    for(k=0;k<TOTAL_BREGISTERS;++k) {
        BReg[k].data=(BINT *)&(BRegData[k*MAX_WORDS*2]);
    }



    REAL one,two_6720;

    two_6720.data=extranumber;

    decconst_One(&one);

    newRealFromBINT(&RReg[0],6720,0);
    newRealFromBINT(&RReg[1],2,0);

    Context.precdigits=2024;

    // COMPUTE THE CONSTANT 2^6720
    powReal(&two_6720,&RReg[1],&RReg[0]);


    Context.precdigits=2016;



    printf("#include <stdint.h>\n\n\n\n// Start of Product(1/sqrt(1+2^-2k)) table generator!\n\n\n");


    printf("const uint32_t const K_binary[%d*%d]= {\n",CORDIC_TABLESIZE,CORDIC_TABLEWORDS);


    // START FROM 1
    newRealFromBINT(&RReg[8],1,0);

    Context.precdigits=2536;

    // 2^(2*K) FOR K=CORDIC_TABLESIZE-1
    newRealFromBINT(&RReg[0],2,0);
    newRealFromBINT(&RReg[1],(CORDIC_TABLESIZE-1)*2,0);
    powReal(&RReg[9],&RReg[0],&RReg[1]);


    for(k=CORDIC_TABLESIZE-1;k>=0;--k)
    {

        Context.precdigits=2536;

        divReal(&RReg[3],&RReg[8],&RReg[9]);    // Prod/2^(2k)
        addReal(&RReg[8],&RReg[8],&RReg[3]);    // Prod+Prod/2^(2k)= Prod*(1+2^(-2k))

        newRealFromBINT(&RReg[0],25,-2);
        mulReal(&RReg[9],&RReg[9],&RReg[0]);    // 0.25*(2)^(2*k) = 2^-2 * (2)^(2*(k-1)) = 2^(2*k-2) = 2^ 2*(k-1)

        // FIND THE SQUARE ROOT OF RReg[8] BY NEWTON RAPHSON

        newRealFromBINT(&RReg[3],5,-1);  // 0.5
        copyReal(&RReg[1],&RReg[8]);
        copyReal(&RReg[0],&RReg[8]);

        do {
        divReal(&RReg[2],&RReg[8],&RReg[1]);    // R/x
        addReal(&RReg[4],&RReg[1],&RReg[2]);    // (x+R/x)
        mulReal(&RReg[0],&RReg[3],&RReg[4]);    // 1/2*(x+R/x)
        swapReal(&RReg[0],&RReg[1]);
        } while(!eqReal(&RReg[0],&RReg[1]));


        // HERE RREG[0] HAS THE RESULT WITH EXTRA DIGITS

        // CONVERT TO AN INTEGER
        divReal(&RReg[1],&two_6720,&RReg[0]);
        roundReal(&RReg[1],&RReg[1],0);
        ipReal(&RReg[2],&RReg[1],1);    // TAKE INTEGER PART AND JUSTIFY THE DIGITS

        // THEN CONVERT TO BINARY FORM

        bIntegerfromReal(&BReg[0],&RReg[2]);

        while(BReg[0].data[BReg[0].len-1]==0) --BReg[0].len;

        if(BReg[0].len>CORDIC_TABLEWORDS) {
            printf("Bad number of words!");
            return 0;
        }

        // AND STORE ALL 210 WORDS

        printf("/* Product(1/sqrt(1+2^-2k)) with k=%d...3359 */\n",k);

        for(j=0;j<BReg[0].len;++j) {
            atanbin_table[k*MAX_WORDS+j]=BReg[0].data[j];
            printf("%uU%c",atanbin_table[k*MAX_WORDS+j],( (j==CORDIC_TABLEWORDS-1)&&(k==CORDIC_TABLESIZE-1))? ' ':',');
            if(j>0 && ((j%21)==20)) printf("\n");
        }
        for(;j<CORDIC_TABLEWORDS;++j) {
            atanbin_table[k*MAX_WORDS+j]=0;
            printf("0U%c",( (j==CORDIC_TABLEWORDS-1)&&(k==CORDIC_TABLESIZE-1))? ' ':',');
            if(j>0 && ((j%21)==20)) printf("\n");
        }

    }

    // BINARY CORDIC TABLE READY
    printf("\n\n};\n\n");

    return 0;
}


// GENERATE A TABLE WITH THE CONSTANT 2^K, FOR K=1... 6720 STEP 32

void generate_two_k()
{



    // INITIALIZE REGISTERS STORAGE

    int k,j;
    BINT extranumber[REAL_REGISTER_STORAGE];



    initContext(2016);

    for(k=0;k<REAL_REGISTERS;++k) {
        RReg[k].data=allocRegister();
        newRealFromBINT(&RReg[k],0,0);
    }

    for(k=0;k<TOTAL_BREGISTERS;++k) {
        BReg[k].data=(BINT *)&(BRegData[k*MAX_WORDS*2]);
    }



int two_k_lentable[316];
int two_k_offtable[316];

printf("#include <stdint.h>\n\n\n\n// Start of 2^(32*k) table generator!\n\n\n");


printf("const uint32_t const two_exp_binary[]= {\n");

two_k_offtable[0]=0;

Context.precdigits=4000;


for(k=1;k<=(CORDIC_MAXSYSEXP+CORDIC_MAXSYSEXP/2)/32;++k)
{


    newRealFromBINT(&RReg[2],2,0);
    newRealFromBINT(&RReg[1],k*32,0);
    powReal(&RReg[0],&RReg[2],&RReg[1]);

    // AND STORE ALL 210 WORDS

    printf("\n/* 2^%d */\n",k*32);

    for(j=0;j<RReg[0].len;++j) {
        atanbin_table[k*MAX_WORDS+j]=RReg[0].data[j];
        printf("%uU%c",atanbin_table[k*MAX_WORDS+j],( (j==RReg[0].len-1)&&(k==(CORDIC_MAXSYSEXP+CORDIC_MAXSYSEXP/2)/32))? ' ':',');
        if(j>0 && ((j%21)==20)) printf("\n");
    }
    two_k_lentable[k-1]=RReg[0].len;
    two_k_offtable[k]=two_k_offtable[k-1]+RReg[0].len;

}

// BINARY CORDIC TABLE READY
printf("\n\n};\n\n");

// OUTPUT THE LENGTH AND OFFSET TABLE


printf("const uint32_t const two_exp_offset[]= {\n");

for(j=0;j<(CORDIC_MAXSYSEXP+CORDIC_MAXSYSEXP/2)/32;++j) {
    printf("%uU%c",two_k_offtable[j]|(two_k_lentable[j]<<16), (j==(CORDIC_MAXSYSEXP+CORDIC_MAXSYSEXP/2)/32-1)? ' ':',');
    if(j>0 && ((j%8)==7)) printf("\n");
}

printf("\n\n};\n\n");



}


// GIVEN COS(X) IN RReg[0], RETURN COS(10x) IN RReg[0]
// USES RReg[0] THROUGH RReg[3]

// COS(10X)=512*Z^10-1280*Z^8+1120*Z^6-400*Z^4+50*Z^2-1
// WITH Z=COS(X)

void cos10x()
{
    mulReal(&RReg[1],&RReg[0],&RReg[0]);   // X^2

    RReg[2].flags=0;
    RReg[2].data[0]=512;
    RReg[2].exp=0;
    RReg[2].len=1;          // CONSTANTS FOR POLYNOMIAL

    mulReal(&RReg[0],&RReg[1],&RReg[2]);
    RReg[2].data[0]=1280;
    subReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[0],&RReg[1],&RReg[3]);
    RReg[2].data[0]=1120;
    addReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[0],&RReg[1],&RReg[3]);
    RReg[2].data[0]=400;
    subReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[0],&RReg[1],&RReg[3]);
    RReg[2].data[0]=50;
    addReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[0],&RReg[1],&RReg[3]);
    RReg[2].data[0]=1;
    subReal(&RReg[0],&RReg[0],&RReg[2]);

}

// SAME BUT RECEIVES COS(X)-1 FOR INCREASED PRECISION
void cos10x_prec()
{
    mulReal(&RReg[1],&RReg[0],&RReg[0]);   // X^2
    add_real_mul(&RReg[1],&RReg[1],&RReg[0],2); // 2*X+X^2

    RReg[2].flags=0;
    RReg[2].data[0]=512;
    RReg[2].exp=0;
    RReg[2].len=1;          // CONSTANTS FOR POLYNOMIAL

    mulReal(&RReg[0],&RReg[1],&RReg[2]);
    RReg[2].data[0]=768;
    subReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[4],&RReg[1],&RReg[3]);
    addReal(&RReg[0],&RReg[4],&RReg[3]);
    RReg[2].data[0]=1120;
    addReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[4],&RReg[1],&RReg[3]);
    addReal(&RReg[0],&RReg[4],&RReg[3]);
    RReg[2].data[0]=400;
    subReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[4],&RReg[1],&RReg[3]);
    addReal(&RReg[0],&RReg[4],&RReg[3]);
    RReg[2].data[0]=50;
    addReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[4],&RReg[1],&RReg[3]);
    addReal(&RReg[0],&RReg[4],&RReg[3]);
    RReg[2].data[0]=1;
    subReal(&RReg[0],&RReg[0],&RReg[2]);

}




// COMPUTE COS(X) FOR X<1E-5
// GIVEN X IN RReg[0]
// RETURNS COS(X) IN RReg[0]

void cospower()
{
int k;
int orgexp,digits;
int needdigits=Context.precdigits;
int correction;
int chebyshev_iter=0;

while( (16*chebyshev_iter*chebyshev_iter) < needdigits) chebyshev_iter+=2;

orgexp=RReg[0].exp;
RReg[0].exp=0;
digits=intdigitsReal(&RReg[0]);

correction=orgexp+digits+chebyshev_iter;

if(correction>0) {
    Context.precdigits=needdigits+((3*chebyshev_iter+15)&~7); // GET NECESSARY DIGITS TO PRESERVE VALUE
    RReg[0].exp=-digits-chebyshev_iter;
    if(Context.precdigits<needdigits) Context.precdigits=needdigits+8;
}
else {
    RReg[0].exp=orgexp;
    Context.precdigits+=16;
}


RReg[2].flags=F_NEGATIVE;
RReg[2].exp=0;
RReg[2].len=1;          // FACTORIAL


mulReal(&RReg[3],&RReg[0],&RReg[0]);   // X^2
RReg[0].flags=0;
RReg[0].exp=0;
RReg[0].len=1;
RReg[0].data[0]=1;      // FIRST TERM

RReg[4].flags=0;
RReg[4].exp=0;
RReg[4].len=1;
RReg[4].data[0]=0;      // ACCUMULATOR

// DO AS MANY TERMS AS NEEDED
for(k=2;k<=REAL_PRECISION_MAX/2;k+=2)
{
    mulReal(&RReg[1],&RReg[0],&RReg[3]); // TERM*X^2
    //normalize(&RReg[1]);
    RReg[2].data[0]=k*(k-1);
    divReal(&RReg[0],&RReg[1],&RReg[2]); // NEWTERM= TERM*X^2/(k*(k-1)) = X^K/K!
    // HERE WE HAVE THE NEW TERM OF THE SERIES IN RReg[0]
    addReal(&RReg[5],&RReg[4],&RReg[0]);

    if(eqReal(&RReg[4],&RReg[5])) break;
    swapReal(&RReg[4],&RReg[5]);
}


//printf("COS: total iterations=%d, correction=%d, need_prec=%d, actual prec=%d\n",k,correction,(-RReg[4].exp+7)&~7,Context.precdigits);

if(correction>0) {


RReg[2].data[0]=1;
RReg[2].flags=0;

addReal(&RReg[0],&RReg[4],&RReg[2]);

for(k=0;k<correction;++k) cos10x();
}
else {
    RReg[2].data[0]=1;
    RReg[2].flags=0;

    addReal(&RReg[0],&RReg[4],&RReg[2]);
}


Context.precdigits=needdigits;

}


void trig_cospower(REAL *angle, BINT angmode)
{
    int negsin,negcos,startexp;
    REAL pi,pi2,pi4;
    BINT savedprec;

    negcos=negsin=0;

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
            MACROOneToRReg(0);
            if(isoddReal(&RReg[1])) RReg[0].flags|=F_NEGATIVE;
            // RESTORE PREVIOUS PRECISION
            Context.precdigits=savedprec;

            return;
        }
        RReg[2].data[0]>>=1; // 90 OR 100 DEGREES
        RReg[2].flags|=RReg[0].flags&F_NEGATIVE;

        if(eqReal(&RReg[0],&RReg[2])) {
            // EXACT PI/2 OR 3/2PI, RETURN EXACT VALUES
            MACROZeroToRReg(0);
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
        MACROOneToRReg(0);
        if(isoddReal(&RReg[1])) RReg[0].flags|=F_NEGATIVE;
        // RESTORE PREVIOUS PRECISION
        Context.precdigits=savedprec;

        return;
    }

    // IF THE RESULT OF THE DIVISION IS ODD, THEN WE ARE IN THE OTHER HALF OF THE CIRCLE
    if(isoddReal(&RReg[1])) { negcos=negsin=1; }

    if(RReg[0].flags&F_NEGATIVE) { negsin^=1; RReg[0].flags&=~F_NEGATIVE; }

    if(gtReal(&RReg[0],&pi2)) {
        negcos^=1;
        sub_real(&RReg[0],&pi,&RReg[0]);
    }
    /*
    if(gtReal(&RReg[0],&pi4)) {
        swap^=1;
        sub_real(&RReg[0],&pi2,&RReg[0]);
    }
    */
    normalize(&RReg[0]);

    Context.precdigits=savedprec;

    cospower();


    if(negcos) RReg[0].flags|=F_NEGATIVE;

}


// GIVEN SIN(X) IN RReg[0], RETURN SIN(5x) IN RReg[0]
// USES RReg[0] THROUGH RReg[3]

// SIN(5X)=16*Z^5-20*Z^3+5*Z
// Z=SIN(X)


void sin5x()
{
    mulReal(&RReg[1],&RReg[0],&RReg[0]);   // X^2

    RReg[2].flags=0;
    RReg[2].data[0]=16;
    RReg[2].exp=0;
    RReg[2].len=1;          // CONSTANTS FOR POLYNOMIAL

    mulReal(&RReg[4],&RReg[1],&RReg[2]);
    RReg[2].data[0]=20;
    subReal(&RReg[3],&RReg[4],&RReg[2]);

    mulReal(&RReg[4],&RReg[1],&RReg[3]);
    RReg[2].data[0]=5;
    addReal(&RReg[3],&RReg[4],&RReg[2]);

    mulReal(&RReg[4],&RReg[0],&RReg[3]);

    swapReal(&RReg[0],&RReg[4]);

}


// COMPUTE SIN(X)
// GIVEN X IN RReg[0]
// RETURNS SIN(X) IN RReg[0]

void sinpower()
{
int k;
int orgexp,digits;
int needdigits=Context.precdigits;
int correction;
int chebyshev_iter=0;

orgexp=RReg[0].exp;

while( (16*chebyshev_iter*chebyshev_iter) < needdigits) chebyshev_iter+=2;

RReg[0].exp=0;
digits=intdigitsReal(&RReg[0]);

correction=orgexp+digits+chebyshev_iter;

if(correction>0) {
    Context.precdigits=needdigits+((3*chebyshev_iter+15)&~7); // GET NECESSARY DIGITS TO PRESERVE VALUE
    RReg[0].exp=-digits-chebyshev_iter;

    if(Context.precdigits<needdigits) Context.precdigits=needdigits+8;

    // MULTIPLY BY 2^correction
    newRealFromBINT(&RReg[1],1<<correction,0);
    mulReal(&RReg[0],&RReg[0],&RReg[1]);

}
else {
    RReg[0].exp=orgexp;
    Context.precdigits+=16;
}


RReg[2].flags=F_NEGATIVE;
RReg[2].exp=0;
RReg[2].len=1;          // FACTORIAL


mulReal(&RReg[3],&RReg[0],&RReg[0]);   // X^2

//  FIRST TERM IN RReg[0] IS X

copyReal(&RReg[4],&RReg[0]); // ACCUMULATOR

// DO AS MANY TERMS AS NEEDED
for(k=3;k<=REAL_PRECISION_MAX/2;k+=2)
{
    mulReal(&RReg[1],&RReg[0],&RReg[3]); // TERM*X^2
    //normalize(&RReg[1]);
    RReg[2].data[0]=k*(k-1);
    divReal(&RReg[0],&RReg[1],&RReg[2]); // NEWTERM= TERM*X^2/(k*(k-1)) = X^K/K!
    // HERE WE HAVE THE NEW TERM OF THE SERIES IN RReg[0]
    addReal(&RReg[5],&RReg[4],&RReg[0]);

    if(eqReal(&RReg[4],&RReg[5])) break;
    swapReal(&RReg[4],&RReg[5]);
}


//printf("SIN: total iterations=%d, correction=%d, need_prec=%d, actual prec=%d\n",k,correction,(-RReg[4].exp+7)&~7,Context.precdigits);

swapReal(&RReg[0],&RReg[4]);
if(correction>0) {
for(k=0;k<correction;++k) sin5x();
}

Context.precdigits=needdigits;

}


void trig_sinpower(REAL *angle, BINT angmode)
{
    int negsin,negcos,swap,startexp;
    REAL pi,pi2,pi4;
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
            MACROZeroToRReg(0);
            // RESTORE PREVIOUS PRECISION
            Context.precdigits=savedprec;

            return;
        }
        RReg[2].data[0]>>=1; // 90 OR 100 DEGREES
        RReg[2].flags|=RReg[0].flags&F_NEGATIVE;

        if(eqReal(&RReg[0],&RReg[2])) {
            // EXACT PI/2 OR 3/2PI, RETURN EXACT VALUES
            MACROOneToRReg(0);
            if(isoddReal(&RReg[1])) RReg[0].flags|=F_NEGATIVE;
            // RESTORE PREVIOUS PRECISION
            Context.precdigits=savedprec;


            return;
        }





        // CONVERT TO RADIANS
        mulReal(&RReg[0],&RReg[0],&convfactor);

        normalize(&RReg[0]);

    }


    // HERE RReg[0] HAS THE REMAINDER THAT WE NEED TO WORK WITH


    // CHECK FOR SPECIAL CASES

    if(iszeroReal(&RReg[0])) {
        // EXACT MULTIPLE OF PI, IN RADIANS THIS CAN ONLY HAPPEN IF THE ARGUMENT IS ACTUALLY ZERO
        MACROZeroToRReg(0);
        // RESTORE PREVIOUS PRECISION
        Context.precdigits=savedprec;

        return;
    }

    // IF THE RESULT OF THE DIVISION IS ODD, THEN WE ARE IN THE OTHER HALF OF THE CIRCLE
    if(isoddReal(&RReg[1])) { negcos=negsin=1; }

    if(RReg[0].flags&F_NEGATIVE) { negsin^=1; RReg[0].flags&=~F_NEGATIVE; }

    if(gtReal(&RReg[0],&pi2)) {
        sub_real(&RReg[0],&pi,&RReg[0]);
        normalize(&RReg[0]);
    }

    if(gtReal(&RReg[0],&pi4)) {
        swap^=1;
        sub_real(&RReg[0],&pi2,&RReg[0]);
        normalize(&RReg[0]);

    }


    Context.precdigits=savedprec;

    if(swap) cospower(); else sinpower();


    if(negsin) RReg[0].flags|=F_NEGATIVE;

}










int main()
{



    //generate_atantable();
    //generate_Ktable();
    //generate_two_k();


    //return 0;


    // INITIALIZE REGISTERS STORAGE

    int k;
    BINT extranumber[REAL_REGISTER_STORAGE];



    initContext(2000);

    for(k=0;k<REAL_REGISTERS;++k) {
        RReg[k].data=allocRegister();
        newRealFromBINT(&RReg[k],0,0);
    }

    for(k=0;k<TOTAL_BREGISTERS;++k) {
        BReg[k].data=(BINT *)&(BRegData[k*MAX_WORDS*2]);
    }

    REAL constpi180;

    decconst_PI_180(&constpi180);




    clock_t start,end;


    // ************************************************************************************
    // SINE TEST


    start=clock();




#define TEST_DIGITS 100

    Context.precdigits=TEST_DIGITS;

//  TEST SINE THROUGH POWERS
    for(k=0;k<100;++k) {


    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    trig_sinpower(&RReg[0],ANGLERAD);

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[7]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[7],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in sin(x), k=%d\n",k);
    }


    }

    end=clock();

    printf("Done first run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

    start=clock();


//  TEST SINE THROUGH POWERS
    for(k=0;k<100;++k) {


    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    trig_sinpower(&RReg[0],ANGLERAD);

    /*

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],5*k,-1);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }
    */

    }

    end=clock();

    printf("Done second run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);


    start=clock();


//  TEST SINE THROUGH POWERS
    for(k=0;k<100;++k) {

    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    //cospower();
    trig_sincos(&RReg[0],ANGLERAD);

    /*

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],5*k,-1);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }
    */

    }

    end=clock();

    printf("Done trig_sincos in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

// END OF SINE TEST
// ********************************************************************************************
return 0;


    // ************************************************************************************
    // COSINE TEST


    start=clock();




#define TEST_DIGITS 100

    Context.precdigits=TEST_DIGITS;

//  TEST COSINE THROUGH POWERS
    for(k=0;k<100;++k) {


    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    trig_cospower(&RReg[0],ANGLERAD);
    //trig_sincos(&RReg[0],ANGLERAD);

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }


    }

    end=clock();

    printf("Done first run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

    start=clock();


//  TEST COSINE THROUGH POWERS
    for(k=0;k<100;++k) {


    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    cospower();
    //trig_sincos(&RReg[0],ANGLERAD);

    /*

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],5*k,-1);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }
    */

    }

    end=clock();

    printf("Done second run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);


    start=clock();


//  TEST COSINE THROUGH POWERS
    for(k=0;k<100;++k) {

    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    //cospower();
    trig_sincos(&RReg[0],ANGLERAD);

    /*

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],5*k,-1);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }
    */

    }

    end=clock();

    printf("Done trig_sincos in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

// END OF COSINE TEST
// ********************************************************************************************



    return 0;


   return 0;
// TEST OF NEW DROP-IN bintrig_sincos

    for(k=0;k<100;++k) {


    newRealFromBINT(&RReg[0],k*137,-k);     // 0.3

    bintrig_sincos(&RReg[0],ANGLERAD);

    swapReal(&RReg[6],&RReg[8]);
    swapReal(&RReg[7],&RReg[9]);

    finalize(&RReg[8]);
    finalize(&RReg[9]);

    newRealFromBINT(&RReg[0],k*137,-k); // ANGLE 0.3 RADIANS

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[7]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);
    subReal(&RReg[1],&RReg[7],&RReg[9]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }
    if(!iszeroReal(&RReg[1])) {
        printf("Error in sin(x), k=%d\n",k);
    }

    }
    return 0;



    return 0;

// Original tests

    REAL one,two_6720;

    two_6720.data=extranumber;

    decconst_One(&one);

    newRealFromBINT(&RReg[0],6720,0);
    newRealFromBINT(&RReg[1],2,0);

    Context.precdigits=2024;

    // COMPUTE THE CONSTANT 2^6720
    powReal(&two_6720,&RReg[1],&RReg[0]);


    Context.precdigits=2016;

    // TEST OF A SIMPLE CORDIC LOOP

    newRealFromBINT(&RReg[0],3,-500); // ANGLE 0.3 RADIANS

    mulReal(&RReg[1],&RReg[0],&two_6720);
    roundReal(&RReg[1],&RReg[1],0);
    ipReal(&RReg[1],&RReg[1],1);    // TAKE INTEGER PART AND JUSTIFY THE DIGITS

    bIntegerfromReal(&BReg[0],&RReg[1]);   // CONVERT ANGLE TO BINARY FORM


    bIntegerMul(&BReg[2],&BReg[0],&BReg[0],6720);


    RealfrombInteger(&RReg[0],&BReg[2]);

    divReal(&RReg[0],&RReg[0],&two_6720);


    REAL *ang,*x,*xn,*y,*yn,*tmp;

    REAL table_const;

    ang=&BReg[0];
    x=&BReg[1];
    y=&BReg[2];
    xn=&BReg[3];
    yn=&BReg[4];

    table_const.exp=0;
    table_const.len=CORDIC_TABLEWORDS;
    table_const.flags=0;

    int exp;

    // INITIAL VALUES:
    //X=1 , Y=0

    x->len=CORDIC_TABLEWORDS+1;
    y->len=1;
    x->flags=0;
    y->flags=0;
    for(k=0;k<CORDIC_TABLEWORDS;++k) x->data[k]=y->data[k]=0;
    x->data[k]=1;

    Context.precdigits=2016;

    for(exp=0;exp<CORDIC_TABLESIZE;++exp)
    {
        // Xn=X (-/+) Y>>exp
        // YN=Y (+/-) X>>exp


        if(!(ang->flags&F_NEGATIVE)) {
            y->flags^=F_NEGATIVE;
            bIntegerAddShift(xn,x,y,exp);
            y->flags^=F_NEGATIVE;
            bIntegerAddShift(yn,y,x,exp);
        }
        else {
            bIntegerAddShift(xn,x,y,exp);
            x->flags^=F_NEGATIVE;
            bIntegerAddShift(yn,y,x,exp);
            x->flags^=F_NEGATIVE;
        }

        // READ THE ANGLE FROM THE TABLE

        table_const.data=atan_binary+exp*CORDIC_TABLEWORDS;
        table_const.len=CORDIC_TABLEWORDS;
        while(!table_const.data[table_const.len-1]) --table_const.len;


        if(!(ang->flags&F_NEGATIVE)) {
            table_const.flags^=F_NEGATIVE;
            bIntegerAdd(ang,ang,&table_const);
            table_const.flags^=F_NEGATIVE;
        } else bIntegerAdd(ang,ang,&table_const);

        RealfrombInteger(&RReg[5],ang);
        RealfrombInteger(&RReg[6],&table_const);

        tmp=x; x=xn; xn=tmp;
        tmp=y; y=yn; yn=tmp;

    }

    // FINAL ROTATION BY RESIDUAL ANGLE
    // Xn=X-Y*tan(Ang)=X-Y*Ang
    // Yn=Y+X*tan(Ang)=Y-X*Ang

    bIntegerMul(yn,y,ang,CORDIC_TABLEWORDS*32);
    bIntegerMul(xn,x,ang,CORDIC_TABLEWORDS*32);
    yn->flags^=F_NEGATIVE;
    bIntegerAdd(x,x,yn);
    bIntegerAdd(y,y,xn);



    // RESULTS ARE IN BReg[1] AND BReg[2] FOR x AND y RESPECTIVELY
    // DONE, APPLY CORRECTION FACTOR K= PRODUCT(1/SQRT(1+2^-2K))


    table_const.data=K_binary+(exp-1)*CORDIC_TABLEWORDS;
    table_const.len=CORDIC_TABLEWORDS;
    table_const.flags=0;
    table_const.exp=0;

    bIntegerMul(xn,x,&table_const,CORDIC_TABLEWORDS*32);
    bIntegerMul(yn,y,&table_const,CORDIC_TABLEWORDS*32);

    // FINISHED! RESULTS ARE IN BReg[3] and BReg[4]

    RealfrombInteger(&RReg[3],&BReg[3]);    // COS(ANGLE)
    RealfrombInteger(&RReg[4],&BReg[4]);    // SIN(ANGLE)
    RealfrombInteger(&RReg[5],ang);

    divReal(&RReg[8],&RReg[3],&two_6720);
    divReal(&RReg[9],&RReg[4],&two_6720);

      newRealFromBINT(&RReg[0],3,-500); // ANGLE 0.3 RADIANS

      trig_sincos(&RReg[0],ANGLERAD);

      finalize(&RReg[6]);
      finalize(&RReg[7]);

      subReal(&RReg[0],&RReg[6],&RReg[8]);
      subReal(&RReg[1],&RReg[7],&RReg[9]);
    return 0;
}
