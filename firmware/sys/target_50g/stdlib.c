/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

#define INT_MAX 0x7fffffff
#define INT_MIN (-INT_MAX -1)


// THESE ARE STUB FUNCTIONS NEEDED BY libgcc TO PROCESS DIV BY ZERO EXCEPTIONS

int errno __SYSTEM_GLOBAL__;

int *__errno() { return &errno; }

// SOME SYSTEMS (BSD) DEFINE errno POINTING TO __error
// THIS IS NEEDED SO STANDARD HEADERS CAN BE USED
int *__error() { return &errno; }


#define	ERANGE 34
#define errno (*__errno())



void abort()
{
    throw_exception("ABORT CALLED",__EX_EXIT | __EX_RESET);
}


void *memcpyb(void *trg, const void *src, int n)
{
    void *r  = trg;
    char *t8 = (char *) trg;
    char *s8 = (char *) src;

    #define ESIZE sizeof(int)
    #define CSIZE (4*ESIZE)

    if (n >= CSIZE &&  ! ( ((unsigned)t8 & (ESIZE-1)) || ((unsigned)s8 & (ESIZE-1)) ) ) {

        // source & target properly aligned
        // use word copy...

        int *T,*S;


        T = (int *) t8;
        S = (int *) s8;

        for (; n >= CSIZE ;) {
            *T++ = *S++;
            *T++ = *S++;
            *T++ = *S++;
            *T++ = *S++;
            n -= CSIZE;
        }

        for (; n >= ESIZE ;) {
            *T++ = *S++;
            n -= ESIZE;
        }

        if ( n > 0 ) {

            t8 = (char *) T;
            s8 = (char *) S;

            for (; n-- ;)
                *t8++ = *s8++;
        }
    }
    else if(n > 0)
        for (; n-- ;)
            *t8++ = *s8++;


    return r;
}


// COPY ASCENDING BY WORDS
void memcpyw(void *dest,const void *source,int nwords)
{
    int *destint = (int *) dest;
    int *sourceint = (int *) source;
    while(nwords) {
        *destint=*sourceint;
        ++destint; ++sourceint; --nwords;
    }

}

// COPY DESCENDING BY WORDS
void memcpywd(void *dest,const void *source,int nwords)
{
    int *destint = ((int *) dest)+nwords;
    int *sourceint = ((int *) source)+nwords;
    while(nwords) {
        *--destint=*--sourceint;
        --nwords;
    }

}

// MOVE MEMORY, HANDLE OVERLAPPING BLOCKS PROPERLY
void memmovew(void *dest,const void *source,int nwords)
{
    int offset=((int *)dest)-((int *)source);
    int *ptr=(int *)source;
    if(offset>0) {
        ptr+=nwords-1;
        while(nwords>0) {
            ptr[offset]=*ptr;
            --ptr;
            --nwords;
        }
    }
    else {
        while(nwords>0) {
            ptr[offset]=*ptr;
            ++ptr;
            --nwords;
        }
    }
}

void *memmoveb(void *_dest, const void *_source, int nbytes)
{
    register char *dest= (char *) _dest;
    register char *source= (char *) _source;
    register int going=(_dest>_source)?(-1):(1);
    if (going==-1)
    {
        dest+=nbytes-1;
        source+=nbytes-1;
    }

    while(nbytes--)
    {
        *dest=*source;
        dest+=going;
        source+=going;
    }
    return _dest;
}

void memsetw(void *dest,int value,int nwords)
{
    int *_dest=(int *)dest;
    while(nwords) { *_dest++=value; --nwords; }
}

void *memsetb(void *_dest,int value, int nbytes)
{
    char *destc = (char *) _dest;

    int val2=value&0xff;
    val2|= (val2<<8) | (val2<<16) | (val2<<24);
    while(nbytes>0 && (((int)destc)&3)) {
        *destc = value;
        ++destc;
        nbytes--;
    }

    int *desti = (int *) destc;

    while(nbytes>=4) {
        *desti = val2;
        ++desti;
        nbytes-=4;
    }

    destc = (char *) desti;

    while(nbytes) {
        *destc=value;
        ++destc;
        nbytes--;
    }
    return (void *) destc;
}





int strncmp ( const char *s1, const char *s2, int num)
{
    if (num > 0)
    {
        while (num > 0)
        {
            if (*s1 != *s2) break;
            if (*s1 == '\0') return 0;

            s1++;
            s2++;
            num--;
        }

        if (num > 0)
        {
            if (*s1 == '\0') return -1;
            if (*s2 == '\0' ) return 1;

            return (((unsigned char ) *s1 ) - ((unsigned char) *s2));
        }
    }

    return 0;
}


int stringlen(const char *s)
{
    int c = 0;
    while (*s++) c++;
    return c;
}

char * strcpy(char *t, const char *s)
{
    char *r = t;
    while(*s)
        *t++ = *s++;
    *t = '\0';
    return r;
}


long strtol(const char *number,char **endptr,int base)
{
    int digit,neg=0,countdigit=0;
    long long result=0;
    char *start=(char *)number;

    // SKIP INITIAL BLANKS
    while((*number==' ') || (*number=='\t')) ++number;

    if(*number=='+') ++number;
    else if(*number=='-') { neg=1; ++number; }


    if(base==16) {
        if(*number=='0') {
            if((number[1]=='x')||(number[1]=='X')) number+=2;
        }
    }




while(*number) {
    digit=-1;
    if((*number>='0')&&(*number<='9')) digit=*number-'0';
    if((*number>='a') &&(*number<='z')) digit=*number-'a'+10;
    if((*number>='A')&&(*number<='Z')) digit=*number-'A'+10;
    if(digit<0 || digit>=base) {

        if(!countdigit) number=start;
        if(neg) result=-result;

        if(result<INT_MIN) {
            errno=ERANGE;
            result=INT_MIN;
        }

            if(result>INT_MAX) {
                errno=ERANGE;
                result=INT_MAX;
            }
    //SET END POINTER AND RETURN
        if((endptr)) *endptr=(char *)number;
        return result;
    }
    ++countdigit;
    ++number;
    result=result*base+digit;
}

if(!countdigit) number=start;

if(neg) result=-result;
if(result<INT_MIN) {
    errno=ERANGE;
    result=INT_MIN;
}

    if(result>INT_MAX) {
        errno=ERANGE;
        result=INT_MAX;
    }
//SET END POINTER AND RETURN
if((endptr)) *endptr=(char *)number;
return result;
}





int raise(int sig)
{
    UNUSED_ARGUMENT(sig);
    return 0;
}

