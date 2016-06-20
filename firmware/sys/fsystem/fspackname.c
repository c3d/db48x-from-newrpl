/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"








char *FSUnicode2OEM(char *dest,char *origin,int nchars)
{
int f;
int val;
unsigned int utf8val;
for(f=0;f<nchars;++f)
{
val=ReadInt16((char *)origin);
utf8val=cp2utf8(val);
// ADD UNICODE -> CALCULATOR CONVERSION HERE
//if(val>0xff) val='_';		// replace UNKNOWN UNICODE CHARACTERS
while(utf8val) { *dest++=utf8val&0xff; utf8val>>=8; }
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
WriteInt16((char *)dest,val);
if(!val) { val=0xffff; origin=NULL; }
}

return origin;
}


// EXTRACT 13 CHARACTERS FROM A LONG DIRECTORY ENTRY
void FSPackName(char *name,char *direntry)
{
name=FSUnicode2OEM(name,direntry+1,5);
name=FSUnicode2OEM(name,direntry+14,6);
FSUnicode2OEM(name,direntry+28,2);
}


// EXTRACT 13 CHARACTERS FROM A LONG DIRECTORY ENTRY
void FSUnpackName(char *name,char *direntry)
{
name=FSOEM2Unicode(name,direntry+1,5);
name=FSOEM2Unicode(name,direntry+14,6);
FSOEM2Unicode(name,direntry+28,2);
}





void FSPackShortName(char *name,char *direntry)
{
int f;
char *ptr;
	f=7;
	while(direntry[f]==0x20) --f;
	ptr=name+f+1;
	if(direntry[12]&0x8) {
	while(f>=0) {
	name[f]=__LOWER(direntry[f]);
	--f;	
	}
	}
	else {
	while(f>=0) {
	name[f]=direntry[f];
	--f;	
	}
	}
	if(direntry[8]!=0x20) {
	*ptr='.';
	++ptr;
	for(f=8;f<11;++f)
	{
	if(direntry[f]!=0x20) { *ptr=(direntry[12]&0x10)?  __LOWER(direntry[f]):direntry[f]; ++ptr; }
	}
	}
	*ptr=0;

}
