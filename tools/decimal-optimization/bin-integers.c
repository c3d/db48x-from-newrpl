#include "newrpl.h"

// ADD WITH CARRY PROPAGATION
// res=a+b
WORD bbintadd(WORD *res,WORD *a,WORD *b,int nwords)
{
    UBINT64 rr=0;
while(nwords) {
    rr+=(UBINT64)*a+(UBINT64)*b;
    *res=rr;
    ++res;
    ++a;
    ++b;
    --nwords;
    rr>>=32;
}
return rr;
}

// ADD 2 BINARY INTEGERS

void bIntegerAdd(REAL *res,REAL *a,REAL *b)
{

    BINT resflags=a->flags&F_NEGATIVE;

    if((a->flags&F_NEGATIVE)!=(b->flags&F_NEGATIVE))
    {
        // MAKE SURE WE SUBTRACT ALWAYS FROM LARGER NUMBER TO SMALLER NUMBER
        REAL *largest;

        if(a->len>b->len) largest=a; else {

            if(a->len==b->len) {
                int k=a->len-1;
               while((k>0) && (a->data[k]==b->data[k])) --k;

               if(((WORD)a->data[k])>=((WORD)b->data[k])) largest=a; else largest=b;
            }
            else largest=b;
        }


        if(largest!=a) {
            resflags^=F_NEGATIVE;
            b=a;
            a=largest;
        }

        // HERE IS GUARANTEED THAT a IS LARGER THAN b
        BINT xwords=a->len-b->len,bwords=b->len;
        WORDPTR aptr=(WORDPTR)a->data,bptr=(WORDPTR)b->data,resptr=(WORDPTR)res->data;

        BINT64 rr=0;
        while(bwords>0) {
            rr+=(BINT64)((WORD)*aptr)-(BINT64)((WORD)*bptr);
            *resptr=(WORD)rr;
            ++resptr;
            ++aptr;
            ++bptr;
            --bwords;
            rr>>=32;
        }
        while(xwords) {
            rr+=(BINT64)((WORD)*aptr);
            *resptr=(WORD)rr;
            ++resptr;
            ++aptr;
            --xwords;
            rr>>=32;
        }
        if(rr) *resptr++=(WORD)rr;
        res->len=(BINT *)resptr-res->data;

        while(res->len && (!res->data[res->len-1])) --res->len;

        res->flags=resflags;
        return;
    }



    // SAME SIGN ADDITION
    if(b->len>a->len) {
        REAL *tmp=b;
        b=a;
        a=tmp;
    }

    // HERE IS GUARANTEED THAT a IS LARGER THAN b
    BINT xwords=a->len-b->len,bwords=b->len;
    WORDPTR aptr=(WORDPTR)a->data,bptr=(WORDPTR)b->data,resptr=(WORDPTR)res->data;

    BINT64 rr=0;
    while(bwords) {
        rr+=(BINT64)((WORD)*aptr)+(BINT64)((WORD)*bptr);
        *resptr=(WORD)rr;
        ++resptr;
        ++aptr;
        ++bptr;
        --bwords;
        rr>>=32;
    }
    while(xwords) {
        rr+=(BINT64)((WORD)*aptr);
        *resptr=(WORD)rr;
        ++resptr;
        ++aptr;
        --xwords;
        rr>>=32;
    }
    if(rr) *resptr++=(WORD)rr;
    res->len=(BINT *)resptr-res->data;
    res->flags=resflags;

}



// ADD 2 BINARY INTEGERS: res=a+(b>>bshift)

void bIntegerAddShift(REAL *res,REAL *a,REAL *b,int bshift)
{
    REAL bs;
    BINT resflag;
    bs.data=b->data+(bshift>>5);
    bs.len=b->len-(bshift>>5);
    bs.flags=b->flags;
    bs.exp=0;

    bshift&=31;

    resflag=a->flags&F_NEGATIVE;

    if((a->flags&F_NEGATIVE)!=(b->flags&F_NEGATIVE))
    {
        // MAKE SURE WE SUBTRACT ALWAYS FROM LARGER NUMBER TO SMALLER NUMBER
        REAL *largest;
        BINT bslen=bs.len-((bs.data[bs.len-1]>>bshift)? 0:1);

        if(a->len>bslen) largest=a; else {

            if(a->len==bslen) {
                int k=a->len-1;
                WORD bl,br;

                if(bslen!=bs.len) br=(bs.data[bslen]<<(32-bshift));
                else br=0;

               while(k>0) {
                   bl=(((WORD)bs.data[k])>>bshift)|br;
                   if((WORD)a->data[k]!=bl) break;
                   br=(bs.data[k]<<(32-bshift));
                   --k;
               }

               if(((WORD)a->data[k])>=bl) largest=a; else largest=&bs;
            }
            else largest=&bs;
        }


        if(largest!=a) {
            resflag^=F_NEGATIVE;

            // res=-((bs>>bshift)-a)

            // HERE IS GUARANTEED THAT bs IS LARGER THAN a
            BINT xwords=bs.len-a->len,bwords=a->len;
            WORDPTR aptr=(WORDPTR)a->data,bptr=(WORDPTR)bs.data,resptr=(WORDPTR)res->data;

            BINT64 rr=0;
            while(bwords>0) {
                rr+=(BINT64)(*bptr>>bshift)-(BINT64)*aptr+(BINT64)((((UBINT64)bptr[1])<<(32-bshift))&0xffffffff);
                *resptr=(WORD)rr;
                ++resptr;
                ++aptr;
                ++bptr;
                rr>>=32;
                --bwords;
            }
            while(xwords>0) {
                rr+=(BINT64)(*bptr>>bshift)+(BINT64)((((UBINT64)bptr[1])<<(32-bshift))&0xffffffff);
                *resptr=(WORD)rr;
                ++resptr;
                ++bptr;
                --xwords;
                rr>>=32;

            }
            rr&=(1LL<<(32-bshift))-1;
            if(rr) *resptr++=(WORD)rr;

            res->len=(BINT *)resptr-res->data;

            while(res->len && (!res->data[res->len-1])) --res->len;

            res->flags=resflag;
            return;
        }

        // HERE IS GUARANTEED THAT a IS LARGER THAN bs
        BINT xwords=a->len-bs.len,bwords=bs.len-1;
        WORDPTR aptr=(WORDPTR)a->data,bptr=(WORDPTR)bs.data,resptr=(WORDPTR)res->data;


        BINT64 rr=0;
        while(bwords>0) {
            rr+=(BINT64)*aptr-(BINT64)(*bptr>>bshift)-(BINT64)((((UBINT64)bptr[1])<<(32-bshift))&0xffffffff);
            *resptr=(WORD)rr;
            ++resptr;
            ++aptr;
            ++bptr;
            --bwords;
            rr>>=32;
        }
        // LAST LOOP IS UNROLLED TO AVOID BRINGING BITS FROM THE TOP OF bs
        if(xwords<0) rr-=(BINT64)(*bptr>>bshift);
            else rr+=(BINT64)*aptr-(BINT64)(*bptr>>bshift);
        *resptr=(WORD)rr;
        ++resptr;
        ++aptr;
        ++bptr;
        rr>>=32;

        while(xwords>0) {
            rr+=(BINT64)*aptr;
            *resptr=(WORD)rr;
            ++resptr;
            ++aptr;
            --xwords;
            rr>>=32;
        }
        if(rr) *resptr++=(WORD)rr;
        res->len=(BINT *)resptr-res->data;

        while(res->len && (!res->data[res->len-1])) --res->len;
        res->flags=resflag;
        return;
    }


    // SAME SIGN ADDITION
    // MAKE SURE WE SHIFT THE RIGHT NUMBER

    if(a->len<bs.len) {

        // HERE IS GUARANTEED THAT bs IS LARGER THAN a
        BINT xwords=bs.len-a->len,bwords=a->len;
        WORDPTR aptr=(WORDPTR)a->data,bptr=(WORDPTR)bs.data,resptr=(WORDPTR)res->data;

        UBINT64 rr=0;
        while(bwords>0) {
            rr+=(BINT64)(*bptr>>bshift)+(BINT64)*aptr+(BINT64)((((UBINT64)bptr[1])<<(32-bshift))&0xffffffff);
            *resptr=(WORD)rr;
            ++resptr;
            ++aptr;
            ++bptr;
            rr>>=32;
            --bwords;
        }
        while(xwords>1) {
            rr+=(BINT64)(*bptr>>bshift)+(BINT64)((((UBINT64)bptr[1])<<(32-bshift))&0xffffffff);
            *resptr=(WORD)rr;
            ++resptr;
            ++bptr;
            --xwords;
            rr>>=32;
        }
        if(xwords>0) {
            rr+=(BINT64)(*bptr>>bshift);
            if(rr) {
            *resptr=(WORD)rr;
            ++resptr;
            rr>>=32;
            }
        }

        rr&=(1LL<<(32-bshift))-1;
        if(rr) *resptr++=(WORD)rr;

        res->len=(BINT *)resptr-res->data;
        res->flags=resflag;
        return;
    }

    // HERE IS GUARANTEED THAT a IS LARGER THAN bs
    BINT xwords=a->len-bs.len,bwords=bs.len-1;
    WORDPTR aptr=(WORDPTR)a->data,bptr=(WORDPTR)bs.data,resptr=(WORDPTR)res->data;

    BINT64 rr=0;
    while(bwords>0) {
        rr+=(BINT64)*aptr+(BINT64)(*bptr>>bshift)+(BINT64)((((UBINT64)bptr[1])<<(32-bshift))&0xffffffff);
        *resptr=(WORD)rr;
        ++resptr;
        ++aptr;
        ++bptr;
        --bwords;
        rr>>=32;
    }
    // LAST WORD DONE MANUALLY TO AVOID BITS ENTERING FROM THE TOP ON bs
    rr+=(BINT64)*aptr+(BINT64)(*bptr>>bshift);
    *resptr=(WORD)rr;
    ++resptr;
    ++aptr;
    ++bptr;
    rr>>=32;

    while(xwords>0) {
        rr+=(BINT64)*aptr;
        *resptr=(WORD)rr;
        ++resptr;
        ++aptr;
        --xwords;
        rr>>=32;
    }
    if(rr) *resptr++=(WORD)rr;
    res->len=(BINT *)resptr-res->data;
    res->flags=resflag;
    return;
}

























// ADDS THE IMPLICIT 1>>shift

WORD bbintacc2n(WORD *res,int shift,int nwords)
{
    int awords=nwords-(shift>>5)-1;
    int xwords=nwords-awords;
    res+=awords;
    shift&=31;

    UBINT64 rr=0x100000000LL>>shift;
// CONTINUE A POSSIBLE CARRY
while(xwords && (rr!=0)) {
    rr+=(UBINT64)*res;
    *res=rr;
    ++res;
    --xwords;
    rr>>=32;
}

return rr;
}

// SUBTRACT THE IMPLICIT 1>>shift

WORD bbintaccminus2n(WORD *res,int shift,int nwords)
{
    int awords=nwords-(shift>>5)-1;
    int xwords=nwords-awords;
    res+=awords;
    shift&=31;

    UBINT64 rr=0x100000000LL>>shift;
// CONTINUE A POSSIBLE CARRY
while(xwords && (rr!=0)) {
    rr=(UBINT64)*res-rr;
    *res=rr;
    ++res;
    --xwords;
    rr>>=32;
    if(rr&0x80000000) rr|=0xffffffff00000000LL;
}

return rr;
}


WORD bbintaddshort(WORD *res,WORD *a,WORD b,int nwords)
{
    UBINT64 rr=b;
while(nwords) {
    rr+=(UBINT64)*a;
    *res=rr;
    ++res;
    ++a;
    --nwords;
    rr>>=32;
}
return rr;
}


// ACCUMULATE

WORD bbintacc(WORD *res,WORD *a,int nwords)
{
    UBINT64 rr=0;
while(nwords) {
    rr+=(UBINT64)*res+*a;
    *res=rr;
    ++res;
    ++a;
    --nwords;
    rr>>=32;
}
return rr;
}




// ADD AND SHIFT RIGHT IN ONE OPERATION

WORD bbintaccshiftright(WORD *res,WORD *a,int shift,int nwords)
{
    int awords=nwords-(shift>>5);
    int xwords=nwords-awords;
    a+=shift>>5;
    shift&=31;

    UBINT64 rr=0;
while(awords) {
    rr+=(UBINT64)*res+(((*((UBINT64 *)a))>>shift)&0xffffffffLL);
    *res=rr;
    ++res;
    ++a;
    --awords;
    rr>>=32;
}
// CONTINUE A POSSIBLE CARRY
while(xwords && (rr!=0)) {
    rr+=(UBINT64)*res;
    *res=rr;
    ++res;
    --xwords;
    rr>>=32;
}

return rr;
}

// MULTIPLICATION LONG*SHORT
// res+=a*b

WORD bbintmulshortacc(WORD *res,WORD *a,WORD b,int nwords)
{
UBINT64 rr;
UBINT64 carry=0;

while(nwords) {
    rr=((UBINT64)(*a))*b;
    carry+=*res;
    *res=rr+carry;
    carry=((rr&0xffffffff)+carry)>>32;
    carry+=rr>>32;
    ++res;
    ++a;
    --nwords;
}
*res=carry;
return carry>>32;
}


// MULTIPLICATION LONG*SHORT IN-PLACE
// res*=b

WORD bbintmulshortinplace(WORD *res,WORD b,int nwords)
{
UBINT64 rr;
UBINT64 carry=0;

while(nwords) {
    rr=((UBINT64)(*res))*b;
    *res=rr+carry;
    carry=((rr&0xffffffff)+carry)>>32;
    carry+=rr>>32;
    ++res;
    --nwords;
}
*res=carry;
return carry>>32;
}



WORD bbintmullong(WORD *res,WORD *a,WORD *b,int nwords)
{
    int k;

    // ZERO OUT THE ENTIRE RESULT
    for(k=0;k<2*nwords;++k) res[k]=0;

    k=nwords;
    while(k--)
    {
        res[nwords+1]+=bbintmulshortacc(res,a,*b,nwords);
        ++res;
        b;
    }
}



// NEGATE A LONG BINARY INTEGER (TWO'S COMPLEMENT)
void bbintneg(WORD*res,WORD *a,int nwords)
{
    int k;
    WORD carry=1;
for(k=0;k<nwords;++k)
{
    res[k]=(~a[k])+carry;
    carry=(res[k]==0)? 1:0;
}
}

// CONVERT DECIMAL TO BINARY NUMBER, RETURN NUMBER OF WORDS NEEDED
void bIntegerfromReal(REAL *res,REAL *number)
{
 int nwords=((WORD)((number->len<<3)+number->exp)*217706U)>>21;   // (len*8+exp)*ln(10)/ln(2)/32;
 WORDPTR resdata=res->data;
 int k;

 if(resdata==number->data) resdata=allocRegister();

 ++nwords;  // ONE EXTRA JUST IN CASE

 // NO NEED TO INITIALIZE, USING 0 WORDS WRITES A 0 IN THE FIRST WORD

 // HORNER'S RULE

 for(k=0;k<number->len;++k) {

     // NO ACCUMULATE!
     bbintmulshortinplace(resdata,100000000,k);
     bbintaddshort(resdata,resdata,number->data[number->len-(k+1)],k+1);

 }

 for(k=0;k<number->exp;++k) {
     bbintmulshortinplace(resdata,10,nwords);
 }

 while((resdata[nwords-1]==0)&&(nwords>1)) --nwords;

 if(resdata!=res->data) {
     freeRegister(res->data);
     res->data=resdata;
 }

 res->len=nwords;
 res->exp=0;
 res->flags=0;

}


// CONVERT DECIMAL TO BINARY NUMBER, RETURN NUMBER OF WORDS NEEDED
void RealfrombInteger(REAL *res,REAL *integer)
{
 int nwords=(integer->len*2525223)>>21;   // (len*32)*ln(2)/ln(10)/8;

 int k;

 ++nwords;  // ONE EXTRA JUST IN CASE

 // NO NEED TO INITIALIZE, USING 0 WORDS WRITES A 0 IN THE FIRST WORD

 // HORNER'S RULE

 res->data[0]=0;
 res->len=1;
 res->exp=0;
 res->flags=0;
 REAL tmp,two32;
 BINT tmpstorage[4],two32storage[2]={94967296,42};

 two32.flags=0;
 two32.data=&two32storage;
 two32.len=2;
 two32.exp=0;

 tmp.data=tmpstorage;

 for(k=0;k<integer->len;++k) {

     newRealFromBINT64(&tmp,(WORD)integer->data[integer->len-1-k],0);
     mulReal(res,res,&two32);
     addReal(res,res,&tmp);
 }


}









// MULTIPLY TWO BINARY INTEGERS.
// OPTIONALLY SHIFT THE RESULT RIGHT (FOR FIXED POINT OPERATIONS)

void bIntegerMul(REAL *res,REAL *a,REAL *b,int FPShift)
{
REAL c,*result=res;

if( (result->data==a->data)||(result->data==b->data)) {
    // STORE RESULT INTO ALTERNATIVE LOCATION TO PREVENT OVERWRITE
    result=&c;
    result->data=allocRegister();
}

result->flags=(a->flags^b->flags)&F_NEGATIVE;
result->exp=0;
result->len=a->len+b->len;

zero_words(result->data,result->len);

// MAKE SURE b IS THE SHORTEST NUMBER
if(a->len<b->len) {
    REAL *tmp=a;
    a=b;
    b=tmp;
}

int i,j;

i=0;

UBINT64 rr=0;
BINT carry=0;
while(i<b->len) {
    j=0;
    while(j<a->len) {
        if(__builtin_uaddll_overflow(rr,((UBINT64)(WORD)a->data[j])*((UBINT64)(WORD)b->data[i]),(unsigned long long *)&rr)) ++carry;
        if(__builtin_uadd_overflow((WORD)rr,(WORD)result->data[i+j],(WORDPTR)result->data+i+j)) {
            rr>>=32;
            rr++;
        }
        else rr>>=32;
        if(carry) { rr+=0x100000000LL; carry=0; }

        j++;
    }
    while(rr) {
        if(__builtin_uadd_overflow((WORD)rr,(WORD)result->data[i+j],(WORDPTR)result->data+i+j)) {
            rr>>=32;
            rr++;
        }
        else rr>>=32;
        ++j;
    }


    i++;
}


if(result!=res) {
    // COPY THE RESULT TO THE ORIGINALLY REQUESTED LOCATION
    freeRegister(res->data);
    res->data=result->data;
    res->exp=result->exp;
    res->flags=result->flags;
    res->len=result->len;
}

// TODO: IMPLEMENT THE OPTIONAL SHIFT

if(FPShift) {
    int shword=FPShift>>5;
    int shbits=FPShift&31;

    if(!shbits) {
        copy_words(res->data,res->data+shword,res->len-shword);
        res->len-=shword;
    }
    else {
        WORD carry=0;
        for(i=shword;i<res->len;++i)
            res->data[i-shword]=(((WORD)res->data[i])>>shbits) | ((WORD)(((UBINT64)res->data[i+1])<<(32-shbits)));

        res->len-=shword;
        res->data[res->len-1]&=(1LL<<(32-shbits))-1;


    }
}


}
