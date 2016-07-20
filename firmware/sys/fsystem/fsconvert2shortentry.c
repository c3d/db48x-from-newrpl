/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


static int stringchr(char *string,char a)
{
    while(*string) { if(*string==a) return 1; ++string; }
    return 0;
}


extern const int cp850toUnicode[128];

// CONVERT A LONG NAME INTO SHORT NAME
//        BIT 0=1 --> tname INCLUDES TRAILING ~1
// ERROR= BIT 1=1 --> tname IS A LONG ENTRY
//        BITS 3&4 = NT CASE FLAGS IF NAME IS NOT LONG ENTRY

int FSConvert2ShortEntry(char *name,int minnum)
{
int ncase,ecase,noext,flags;
int nlen,nchars;
unsigned char *ext,*tmp,*orgname=(unsigned char *)name;

ncase=ecase=flags=0;

nlen=(int)stringlen((char *)name)+1;


// 1ST STAGE, ANALYZ123E NAME
tmp=(unsigned char *)name;

while((*tmp=='.') || (*tmp==' ')) ++tmp;

if(*tmp==0) return 0;   // INVALID NAME IS EITHER ALL DOTS OR ALL SPACES
// FIND EXTENSION
ext=(unsigned char *)__fsfindcharrev((char *)tmp,NULL,(char *)".");

if(!ext) { noext=1; ext=(unsigned char *)name+nlen-1; }		// POINT TO END-OF-STRING
else noext=0;

if(tmp!=(unsigned char *)name) {
// STRIP LEADING DOTS OR SPACES
flags|=3;	// MARK NAME WAS CONVERTED
memmoveb(name,tmp,nlen-(int)(tmp-(unsigned char *)name));
ext-=(int)(tmp-(unsigned char *)name);
nlen-=(int)(tmp-(unsigned char *)name);
tmp=(unsigned char *)name;
}

nchars=8;
do {

do {
if(*tmp<=127) {		// >127 IS PERMITTED
	if(*tmp<'A' || *tmp>'Z') {		// UPPERCASE LETTERS ARE PERMITTED
        if(!stringchr("0123456789$%'-_@~`!(){}^#&",*tmp)) {	// NUMBERS AND SYMBOLS PERMITTED
			// IT HAS TO BE AN ILLEGAL CHARACTER
			if(*tmp==' ' || *tmp=='.') {
				// REMOVE SPACES AND PERIODS
				flags|=3; 
                memmoveb(tmp,tmp+1,nlen-(int)(tmp-(unsigned char *)name));
				ext--;
				nlen--;
				continue;
			}
			if(*tmp>='a' && *tmp<='z') {
				// LOWERCASE LETTERS NEED CONVERSION
				*tmp-=32;
				if(ecase!=1) ecase=2;		// MARK NAME IS LOWERCASE
				else flags|=2;
			}
			else {
				// ANY OTHER CHARACTER IS INVALID
				*tmp='_';
				flags|=3;
			}
		}
	}
	else {
		if(ecase!=2) ecase=1;				// MARK NAME IS UPPERCASE
		else flags|=2;
	}
}
else {
    // DECODE A UTF-8 CHARACTER AND CONVERT TO CP850
    int cp=utf82cp((char *)tmp,(char *)tmp+4);
    unsigned char *skiptmp=(unsigned char *)utf8skip((char *)tmp,(char *)tmp+4);

    int k;
    for(k=0;k<128;++k) { if(cp850toUnicode[k]==cp) { cp=128+k; break; } }

    if(cp>0xff) *tmp='_';   // REPLACE ANY UNICODE CHARACTER OUTSIDE RANGE
    else if((cp==0xe5) && (tmp==(unsigned char *)name)) *tmp=0x5;
    else *tmp=cp;

    memmoveb(tmp+1,skiptmp,nlen-(int)(skiptmp-(unsigned char *)name));
    ext-=skiptmp-(tmp+1);
    nlen-=skiptmp-(tmp+1);
}
++tmp;
} while( (tmp!=ext) && (tmp<(unsigned char *)name+nchars));

// NAME WAS PROCESSED
if(tmp!=ext) {
// NAME IS LONGER THAN EXPECTED
flags|=3;

// COPY EXTENSION AFTER NAME
memmoveb(tmp,ext,nlen-(int)(ext-(unsigned char *)name));
nlen-=(int)(ext-tmp);
ext-=(int)(ext-tmp);
}
if(*tmp=='.') {
ext=(unsigned char *)name+nlen-1;
tmp++;
name=(char *)tmp;	// SKIP DOT
// PREPARE TO PROCESS EXTENSION
nchars=3;
ncase=ecase;
ecase=0;
continue;
}


} while(tmp<ext);

if(noext) {
    ncase=ecase;
    ecase=0;
}

if(flags&1 || minnum>0) {
// ADD TAIL IF REQUIRED
// FIND EXTENSION
ext=(unsigned char *)__fsfindcharrev((char *)orgname,NULL,(char *)".");
if(!ext) ext=orgname+nlen-1;


if(minnum<=0) minnum=1;

int digits=1;
if(minnum>9999) digits=5;
else {
if(minnum>999) digits=4;
else {
if(minnum>99) digits=3;
else {
	if(minnum>9) digits=2;
}
}
}

// EXPAND STRING
if(ext<orgname+7-digits) tmp=ext;
else tmp=orgname+7-digits;
memmoveb(tmp+digits+1,ext,nlen-(int)(ext-orgname));

*tmp++='~';

memsetb(tmp,48,digits);

while(minnum>9999)
{
*tmp+=1;
minnum-=10000;
}
if(digits>4) ++tmp;


while(minnum>999)
{
*tmp+=1;
minnum-=1000;
}
if(digits>3) ++tmp;

while(minnum>99)
{
*tmp+=1;
minnum-=100;
}
if(digits>2) ++tmp;

while(minnum>9)
{
*tmp+=1;
minnum-=10;
}
if(digits>1) ++tmp;

*tmp=minnum+'0';
flags|=1;

}

if(flags) return flags;
flags=(ncase&2)<<2;
flags|=(ecase&2)<<3;
return flags;
}

