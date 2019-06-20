/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


#ifndef CONFIG_NO_FSYSTEM



// COMPARISON OF FILE NAMES
// caseflag== 0 --> CASE-SENSITIVE NAME COMPARISON (PURE FAT32)
// 	       == 1 --> CASE-SENSITIVE W/SEMICOLON STRIPPING (FAT32+HP49 FRIENDLY)
//         == 2 --> CASE-INSENSITIVE NAME COMPARISON

int FSNameCompare(char *name1,char *name2,int caseflags)
{
unsigned char *n1,*n2;

switch(caseflags) {
case FSCASE_SENS:
// CASE SENSITIVE
while(*name1==*name2)
{
if(*name1==0) return TRUE;
++name1;
++name2;
}
return FALSE;
case FSCASE_SENSHP:
case FSCASE_SENSHPTRUE:
// CASE-SENSITIVE W/SEMICOLON STRIPPING
n1=(unsigned char *)name1;
n2=(unsigned char *)name2;
while(*n1!=0) ++n1;
while(*n2!=0) ++n2;
n1--;
n2--;
while( (n1>(unsigned char *)name1)&&(*n1==';') ) --n1;
while( (n2>(unsigned char *)name2)&&(*n2==';') ) --n2;
if( (int)(n2-(unsigned char *)name2)!=(int)(n1-(unsigned char *)name1)) return FALSE;		// FALSE IF STRINGS ARE NOT THE SAME LENGTH

while(*n1==*n2) {
if(n1==(unsigned char *)name1) return TRUE;
--n1;
--n2;
}
return FALSE;

case FSCASE_INSENS:
// CASE-INSENSITIVE
while( __ICASE(*name1)==__ICASE(*name2) )
{
if(*name1==0) return TRUE;
++name1;
++name2;
}
return FALSE;

case FSCASE_INSENSHP:
// CASE-INSENSITIVE W/SEMICOLON STRIPPING
n1=(unsigned char *)name1;
n2=(unsigned char *)name2;
while(*n1!=0) ++n1;
while(*n2!=0) ++n2;
n1--;
n2--;
while( (n1>(unsigned char *)name1)&&(*n1==';') ) --n1;
while( (n2>(unsigned char *)name2)&&(*n2==';') ) --n2;
if( (int)(n2-(unsigned char *)name2)!=(int)(n1-(unsigned char *)name1)) return FALSE;		// FALSE IF STRINGS ARE NOT THE SAME LENGTH

while(__ICASE(*n1)==__ICASE(*n2)) {
if(n1==(unsigned char *)name1) return TRUE;
--n1;
--n2;
}
return FALSE;


default:
return FALSE;
}

}



// COMPARE SHORT FILENAME ROOTS, RETURN -1 IF THEY DIFFER,
// OR THE NUMBER AFTER ~xxx IN shentry IF ROOTS ARE THE SAME

int FSNameCompareRoot(char *shentry,char *newname)
{
char *ext1,*ext2;
int number;
// COMPARE EXTENSIONS FIRST

ext1=shentry;
while(*ext1!='.' && *ext1!=0) ++ext1;

ext2=newname;
while(*ext2!='.' && *ext2!=0) ++ext2;

while( *ext2 && *ext1) {
	if(__ICASE(*ext1) != __ICASE(*ext2)) return -1;		// RETURN IF EXTENSIONS DIFFER
	++ext1;
	++ext2;
}

if(*ext1!=*ext2) return -1;		// RETURN IF EXTENSIONS HAVE DIFFERENT LENGTH

// SAME EXTENSION, NOW CHECK FOR NAME

number=0;

while(*shentry && *newname) {

	if(*newname=='.') {
		if(*shentry!='.') return -1;	// DIFFERENT LENGTH
		return 0;		// SAME ROOT, NO NUMBER
	}
	if(*shentry=='.') {
		if(*newname!='.') return -1;	// DIFFERENT LENGTH
	}


	if(*newname=='~') {
	// ROOTS MATCH, GET NUMBER FROM shentry
	while(*shentry!=0 && *shentry!='.' && *shentry!='~') ++shentry;
	if(*shentry!='~') return 0;		// NO NUMBER ON shentry

	}

	if(*shentry=='~') {

	// ROOTS MATCH, GET NUMBER

	++shentry;
	while(*shentry>='0' && *shentry<='9') {
		number*=10;
		number+=*shentry-48;
		++shentry;
	}

	if(number) return number;
	return 0;

	}

	if(__ICASE(*shentry)!=__ICASE(*newname)) return -1;

++shentry;
++newname;

}

if(*shentry==0 && *newname==0) return 0; // IDENTICAL ROOTS, NO NUMBER

if(*shentry=='~' && *newname==0) {
    // ROOTS MATCH, GET NUMBER

    ++shentry;
    while(*shentry>='0' && *shentry<='9') {
        number*=10;
        number+=*shentry-48;
        ++shentry;
    }

    if(number) return number;
    return 0;

    }





return -1;
}



#endif
