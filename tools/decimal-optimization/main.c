#include <stdio.h>
#include <newrpl.h>

#define MAX_WORDS 256
#define TOTAL_BREGISTERS 10

void bIntegerAdd(REAL *res,REAL *a,REAL *b);
void bIntegerAddShift(REAL *res,REAL *a,REAL *b,int bshift);
void bIntegerMul(REAL *res,REAL *a,REAL *b,int FPShift);
void bIntegerfromReal(REAL *res,REAL *number);
void RealfrombInteger(REAL *res,REAL *integer);


WORD bbintadd(WORD *res,WORD *a,WORD *b,int nwords);
WORD bbintacc(WORD *res,WORD *a,int nwords);
WORD bbintmulshortacc(WORD *res,WORD *a,WORD b,int nwords);
WORD bbintmullong(WORD *res,WORD *a,WORD *b,int nwords);
WORD bbintaccshiftright(WORD *res,WORD *a,int shift,int nwords);
void bbintneg(WORD*res,WORD *a,int nwords);
WORD bbintacc2n(WORD *res,int shift,int nwords);


WORD BRegData[MAX_WORDS*2*TOTAL_BREGISTERS];

REAL BReg[TOTAL_BREGISTERS];


#define CORDIC_TABLESIZE    6720/2
#define CORDIC_TABLEWORDS    210

WORD atanbin_table[MAX_WORDS*CORDIC_TABLESIZE];


extern const uint32_t const atan_binary[3360*210];
extern const uint32_t const K_binary[3360*210];


int generate_atantable(void)
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



    printf("#include <stdint.h>\n\n\n// Start of atan(2^-k) table generator!\n\n\n");


    printf("const uint32_t const atan_binary[%d*%d]= {\n",CORDIC_TABLESIZE,CORDIC_TABLEWORDS);


    // START FROM 2^0 = 1
    newRealFromBINT(&RReg[8],1,0);



    for(k=0;k<CORDIC_TABLESIZE;++k)
    {
        trig_atan2(&RReg[8],&one,ANGLERAD);

        // HERE RREG[0] HAS THE RESULT WITH EXTRA DIGITS
        normalize(&RReg[0]);

        Context.precdigits=2024;
        // CONVERT TO AN INTEGER
        mulReal(&RReg[1],&RReg[0],&two_6720);
        roundReal(&RReg[1],&RReg[1],0);
        ipReal(&RReg[2],&RReg[1],1);    // TAKE INTEGER PART AND JUSTIFY THE DIGITS

        Context.precdigits=2016;

        // THEN CONVERT TO BINARY FORM

        bIntegerfromReal(&BReg[0],&RReg[2]);

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




        // HALF THE ANGLE AND GO AGAIN
        newRealFromBINT(&RReg[1],5,-1);
        mulReal(&RReg[8],&RReg[8],&RReg[1]);

    }

    // BINARY CORDIC TABLE READY
    printf("\n\n};\n\n");

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

    // 1/2^(2*K) FOR K=0
    newRealFromBINT(&RReg[9],1,0);


    for(k=0;k<CORDIC_TABLESIZE;++k)
    {

        Context.precdigits=2024;

        mulReal(&RReg[3],&RReg[8],&RReg[9]);    // Prod*2^(-2k)
        addReal(&RReg[8],&RReg[8],&RReg[3]);    // Prod+Prod*2^(-2k)= Prod*(1+2^(-2k))

        newRealFromBINT(&RReg[0],25,-2);    // 0.25 = 2^(-2)
        mulReal(&RReg[9],&RReg[9],&RReg[0]);    // (1/2)^(2*k)*(1/2)^(-2) = (1/2)^(2*(k+1))


        Context.precdigits=2016;

        hyp_sqrt(&RReg[8]);

        // HERE RREG[0] HAS THE RESULT WITH EXTRA DIGITS
        Context.precdigits=2024;
        finalize(&RReg[0]);

        // CONVERT TO AN INTEGER
        divReal(&RReg[1],&two_6720,&RReg[0]);
        roundReal(&RReg[1],&RReg[1],0);
        ipReal(&RReg[2],&RReg[1],1);    // TAKE INTEGER PART AND JUSTIFY THE DIGITS

        Context.precdigits=2016;

        // THEN CONVERT TO BINARY FORM

        bIntegerfromReal(&BReg[0],&RReg[2]);

        while(BReg[0].data[BReg[0].len-1]==0) --BReg[0].len;

        if(BReg[0].len>CORDIC_TABLEWORDS) {
            printf("Bad number of words!");
            return 0;
        }

        // AND STORE ALL 210 WORDS

        printf("/* Product(1/sqrt(1+2^-2k)) with k=1...%d */\n",k);

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





int main()
{



    //generate_atantable();
    //generate_Ktable();
    //return;


    // INITIALIZE REGISTERS STORAGE

    int k;
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

    // TEST OF A SIMPLE CORDIC LOOP

    newRealFromBINT(&RReg[0],3,-1); // ANGLE 0.3 RADIANS

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

      newRealFromBINT(&RReg[0],3,-1); // ANGLE 0.3 RADIANS

      trig_sincos(&RReg[0],ANGLERAD);

      finalize(&RReg[6]);
      finalize(&RReg[7]);

      subReal(&RReg[0],&RReg[6],&RReg[8]);
      subReal(&RReg[1],&RReg[7],&RReg[9]);
    return 0;
}
