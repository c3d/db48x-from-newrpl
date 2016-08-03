/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"

// CONVERSION TABLE TO TURN CHARACTERS FROM UNICODE TO DOS-OEM CODEPAGE 850
const int const cp850toUnicode[128]={
  0XC7,   0XFC,   0XE9,   0XE2,   0XE4,   0XE0,   0XE5,   0XE7,     0XEA,   0XEB,     0XE8,   0XEF,   0XEE,   0XEC,   0XC4,   0XC5,
  0XC9,   0XE6,   0XC6,   0XF4,   0XF6,   0XF2,   0XFB,   0XF9,     0XFF,   0XD6,     0XDC,   0XF8,   0XA3,   0XD8,   0XD7,  0X192,
  0XE1,   0XED,   0XF3,   0XFA,   0XF1,   0XD1,   0XAA,   0XBA,     0XBF,   0XAE,     0XAC,   0XBD,   0XBC,   0XA1,   0XAB,   0XBB,
0X2591, 0X2592, 0X2593, 0X2502, 0X2524,   0XC1,   0XC2,   0XC0,     0XA9, 0X2563,   0X2551, 0X2557, 0X255D,   0XA2,   0XA5, 0X2510,
0X2514, 0X2534, 0X252C, 0X251C, 0X2500, 0X253C,   0XE3,   0XC3,   0X255A, 0X2554,   0X2569, 0X2566, 0X2560, 0X2550, 0X256C,   0XA4,
  0XF0,   0XD0,   0XCA,   0XCB,   0XC8,  0X131,   0XCD,   0XCE,     0XCF, 0X2518,   0X250C, 0X2588, 0X2584,   0XA6,   0XCC, 0X2580,
  0XD3,   0XDF,   0XD4,   0XD2,   0XF5,   0XD5,   0XB5,   0XFE,     0XDE,   0XDA,     0XDB,   0XD9,   0XFD,   0XDD,   0XAF,   0XB4,
  0XAD,   0XB1, 0X2017,   0XBE,   0XB6,   0XA7,   0XF7,   0XB8,     0XB0,   0XA8,     0XB7,   0XB9,   0XB3,   0XB2, 0X25A0,   0XA0
};





char *FSUnicode2OEM(char *dest,char *origin,int nchars)
{
int f;
int val;
unsigned int utf8val;
for(f=0;f<nchars;++f)
{
val=ReadInt16((unsigned char *)origin);
if(!val) *dest++=0;
else {
utf8val=cp2utf8(val);
// ADD UNICODE -> CALCULATOR CONVERSION HERE
//if(val>0xff) val='_';		// replace UNKNOWN UNICODE CHARACTERS
while(utf8val) { *dest++=utf8val&0xff; utf8val>>=8; }
}
origin+=2;
}
return dest;
}


char *FSOEM2Unicode(char *origin,char *dest,int nchars)
{
int f;
int val;
val=0xffff;
for(f=0;f<nchars;++f,dest+=2)
{
if(origin) {
    val=utf82cp(origin,origin+4);
    origin=utf8skip(origin,origin+4);
    if(val>0xffff) val='_'; // FILTER ALL UNICODE CHARACTERS THAT DON'T FIT IN UCS-2
}
WriteInt16((unsigned char *)dest,val);
if(!val) { val=0xffff; origin=NULL; }
}

return origin;
}


// EXTRACT 13 CHARACTERS FROM A LONG DIRECTORY ENTRY
// RETURN POINTER AFTER THE LAST CHARACTER EXPANDED TO UTF-8
char * FSPackName(char *name,char *direntry)
{
name=FSUnicode2OEM(name,direntry+1,5);
name=FSUnicode2OEM(name,direntry+14,6);
return FSUnicode2OEM(name,direntry+28,2);
}


// PUT 13 CHARACTERS INTO A LONG DIRECTORY ENTRY
// CONVERTING FROM UTF-8 TO UCS-2
char * FSUnpackName(char *name,char *direntry)
{
name=FSOEM2Unicode(name,direntry+1,5);
name=FSOEM2Unicode(name,direntry+14,6);
return FSOEM2Unicode(name,direntry+28,2);
}





void FSPackShortName(char *name,char *direntry)
{
int f,count;
char *ptr;
	f=7;
	while(direntry[f]==0x20) --f;

    ptr=name;
	if(direntry[12]&0x8) {
    for(count=0;count<=f;++count) {
        if(!(direntry[count]&0x80)) { *ptr++=__LOWER(direntry[count]); continue; }
        // INTERPRET AS A CP850 CHARACTER, CONVERT TO UNICODE
        unsigned int cp=cp2utf8(cp850toUnicode[(int)((unsigned char *)direntry)[count]-128]);
        while(cp) { *ptr++=cp&0xff; cp>>=8; }
	}
	}
	else {
        for(count=0;count<=f;++count) {
            if(!(direntry[count]&0x80)) { *ptr++=direntry[count]; continue; }
            // INTERPRET AS A CP850 CHARACTER, CONVERT TO UNICODE
            unsigned int cp=cp2utf8(cp850toUnicode[(int)((unsigned char *)direntry)[count]-128]);
            while(cp) { *ptr++=cp&0xff; cp>>=8; }
        }
    }
	if(direntry[8]!=0x20) {
	*ptr='.';
	++ptr;
	for(f=8;f<11;++f)
	{
    if(direntry[f]!=0x20) {
        if(!(direntry[f]&0x80)) {
        *ptr=(direntry[12]&0x10)?  __LOWER(direntry[f]):direntry[f];
        ++ptr;
        }
        else {
            // INTERPRET AS A CP850 CHARACTER, CONVERT TO UNICODE
            unsigned int cp=cp2utf8(cp850toUnicode[(int)((unsigned char *)direntry)[count]-128]);
            while(cp) { *ptr++=cp&0xff; cp>>=8; }

        }

    }
	}

	}
	*ptr=0;

}
