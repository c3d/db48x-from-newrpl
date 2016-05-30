/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"






// CONVERT A LONG NAME INTO SHORT NAME
//        BIT 0=1 --> tname INCLUDES TRAILING ~1
// ERROR= BIT 1=1 --> tname IS A LONG ENTRY
//        BITS 3&4 = NT CASE FLAGS IF NAME IS NOT LONG ENTRY

int FSConvert2ShortEntry(char *name,int minnum)
{
int ncase,ecase,flags;
int nlen,nchars;
char *ext,*tmp,*orgname=name;

ncase=ecase=flags=0;

nlen=(int)strlen((char *)name)+1;

// FIND EXTENSION
ext=(char *)__fsfindcharrev(name,NULL,(char *)".");

if(!ext) ext=name+nlen-1;		// POINT TO END-OF-STRING

// 1ST STAGE, ANALYZE NAME
tmp=name;

while(*tmp=='.' || *tmp==' ') ++tmp;
if(tmp!=name) {
// STRIP LEADING DOTS OR SPACES
flags|=3;	// MARK NAME WAS CONVERTED
memmove(name,tmp,nlen-(int)(tmp-name));
ext-=(int)(tmp-name);
nlen-=(int)(tmp-name);
tmp=name;
}

nchars=8;
do {

do {
if(*tmp<=127) {		// >127 IS PERMITTED
	if(*tmp<'A' || *tmp>'Z') {		// UPPERCASE LETTERS ARE PERMITTED
		if(!strchr("0123456789$%'-_@~`!(){}^#&",*tmp)) {	// NUMBERS AND SYMBOLS PERMITTED
			// IT HAS TO BE AN ILLEGAL CHARACTER
			if(*tmp==' ' || *tmp=='.') {
				// REMOVE SPACES AND PERIODS
				flags|=3; 
				memmove(tmp,tmp+1,nlen-(int)(tmp-name));
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
if(*tmp==0xe5 && tmp==name) *tmp=0x5;

}
++tmp;
} while( (tmp!=ext) && (tmp<name+nchars));

// NAME WAS PROCESSED
if(tmp!=ext) {
// NAME IS LONGER THAN EXPECTED
flags|=3;

// COPY EXTENSION AFTER NAME
memmove(tmp,ext,nlen-(int)(ext-name));
nlen-=(int)(ext-tmp);
ext-=(int)(ext-tmp);
}
if(*tmp=='.') {
ext=name+nlen-1;
tmp=name=tmp+1;	// SKIP DOT
// PREPARE TO PROCESS EXTENSION
nchars=3;
ncase=ecase;
ecase=0;
continue;
}


} while(tmp<ext);

if(flags&1 || minnum>0) {
// ADD TAIL IF REQUIRED
// FIND EXTENSION
ext=(char *)__fsfindcharrev(orgname,NULL,(char *)".");
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
memmove(tmp+digits+1,ext,nlen-(int)(ext-orgname));

*tmp++='~';

memset(tmp,48,digits);

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

