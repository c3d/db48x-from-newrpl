/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


// RETURN TYPE OF PATH
// 1 == Name include drive
// 2 == Name include path
// 4 == Path is absolute
// 8 == File ends in slash
// 16== Drive is HP style
// 32== Name is empty
// BITS 16-31 = DEPTH OF PATH (0=CURRENT DIR OR ROOT)
// NEGATIVE RESULT ==> INVALID FILENAME 

#define SEPARATORS ":\\/"
#define ILLEGAL "\"*<>?|"



int FSGetNameType(char *name)
{
int flags=0;
int iflags=0;
int depth=0;

char *ptr,*partial;

partial=name;
if(__fsfindchar(partial,NULL,ILLEGAL)) return -1;	// NAME CANNOT CONTAIN ILLEGAL CHARACTERS
while(*partial) if(*partial<32) return -1; else ++partial;	// CHARACTERS <32 ARE ILLEGAL
partial=name;
while((ptr=__fsfindchar(partial,NULL,SEPARATORS))) {

// FILE NAME INCLUDES DRIVE/PATH
if(*ptr==':') {
// CHECK IF VALID DRIVE SPECIFICATION
switch(ptr-name)
{
case 0:
// FIRST CHARACTER == ':' --> HP TYPE SPECIFICATION ":3:"
iflags=1;
break;
case 1:
// SECOND CHARACTER == ':' --> DOS-LIKE DRIVE "C:"
if(iflags==1) return -1;
iflags=2;
flags|=1;
break;
case 2:
if(iflags==1) { flags|=17; iflags=3; }
else return -1;
break;
default:
return -1;			// ':' IS ILLEGAL ANYWHERE ELSE
}
}
else {
// PATH SEPARATOR FOUND
if(iflags==1) return -1; 		// INVALID DRIVE SPECIFICATION

flags|=2;
if( ((int)(ptr-name)) == iflags ) flags|=4;		// PATH IS ABSOLUTE
else {
if(ptr==partial) return -1;	// TWO SLASHES TOGETHER
++depth;
}
if(ptr[1]==0) flags|=8;		// NAME ENDS WITH SLASH
}
partial=ptr+1;
}

if(*partial==0) flags|=32;		// NAME IS EMPTY
return flags | (depth<<16);

}
