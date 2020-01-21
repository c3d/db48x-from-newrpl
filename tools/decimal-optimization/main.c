#include <newrpl.h>

#include <time.h>

#define TEST_DIGITS 2000

#define MACROZeroToRReg(n) { RReg[n].data[0]=0; RReg[n].exp=0; RReg[n].flags=0; RReg[n].len=1; }
#define MACROOneToRReg(n) { RReg[n].data[0]=1; RReg[n].exp=0; RReg[n].flags=0; RReg[n].len=1; }
#define MACRONANToRReg(n) { RReg[n].data[0]=0; RReg[n].exp=0; RReg[n].flags=F_NOTANUMBER; RReg[n].len=1; }
#define MACROInfToRReg(n) { RReg[n].data[0]=0; RReg[n].exp=0; RReg[n].flags=F_INFINITY; RReg[n].len=1; }

/*
int main()
{

    // INITIALIZE REGISTERS STORAGE

    int k,j;
    BINT extranumber[REAL_REGISTER_STORAGE];

    initContext(TEST_DIGITS);

    for(k=0;k<REAL_REGISTERS;++k) {
        RReg[k].data=allocRegister();
        newRealFromBINT(&RReg[k],0,0);
    }

   clock_t start,end;

    // ************************************************************************************
    // NORMALIZATION TEST

    start=clock();

//  TEST
    for(k=1;k<5000;++k) {
    RReg[0].data[0]=k*1371;
    for(j=1;j<TEST_DIGITS/8;++j) RReg[0].data[j]=(RReg[0].data[j-1]+k);
    RReg[0].len=TEST_DIGITS/8;
    RReg[0].exp=-k/100;
    RReg[0].flags=0;

    normalize(&RReg[0]);
    mulReal(&RReg[1],&RReg[0],&RReg[0]);

    }

    end=clock();

    printf("Done first run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

    return 0;

}
*/
