/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"







// FILLS IN A FS_FILE STRUCTURE WITH THE DIRECTORY ENTRY
// name MUST BE A VALID NAME (NO ABSOLUTE PATH ALLOWED, NO VALIDITY CHECK)
// caseflag== 0 --> CASE-SENSITIVE NAME COMPARISON
// 	       == 1 --> CASE-SENSITIVE W/SEMICOLON STRIPPING (CALCULATOR-FRIENDLY)
//         == 2 --> CASE-INSENSITIVE NAME COMPARISON
// returns FS_EOF if not found, FS_OK if found or FS_ERROR if there's a problem

int FSPackDir(FS_FILE *dir)
{
FS_VOLUME *fs;
int order,diroffset;
int nentries;
char checksum;
unsigned char *buffer,*morebuff,*ptr;
unsigned char *packbuf;
unsigned int bufoffset,bufsize;

if(!FSystem.Init) return FS_ERROR;

fs=FSystem.Volumes[dir->Volume];

if(!fs) return FS_ERROR;

for(bufsize=32768;bufsize>=1024;bufsize>>=1)
{
packbuf=simpmallocb(bufsize);
if(packbuf) break;
}

if(!packbuf) return FS_ERROR;

bufoffset=0;
buffer=packbuf;



FSSeek(dir,0,FSSEEK_SET);

while((checksum=FSReadLL(buffer,32,dir,fs))==32)
{

if(buffer[0]==0) break;
if(buffer[0]==0xe5) continue;	// DELETED ENTRY, USE NEXT ENTRY
if( (buffer[11]&FSATTR_LONGMASK) == FSATTR_LONGNAME) {

//	printf("LFN entry found\n");
	// TREAT AS LONG FILENAME
	if(!(buffer[0]&0X40)) continue;		// ORPHAN ENTRY, SKIP
	diroffset=dir->CurrentOffset-32;
//	printf("last entry\n");
	// FOUND LAST ENTRY OF A NAME
	nentries=buffer[0]&0x3f;
	morebuff=buffer+32;
	if(morebuff+32*nentries>packbuf+bufsize) { 

	// SAVE PARTIAL BUFFER TO DISK
	int savesize=bufsize-512;
	int olddiroff=dir->CurrentOffset;
	FSSeek(dir,bufoffset,FSSEEK_SET);
    if(FSWriteLL(packbuf,savesize,dir,fs)!=savesize) {
	// THIS BETTER NOT HAPPEN, SEVERE DAMAGE TO DIRECTORY STRUCTURES
	simpfree(packbuf);
	return FS_ERROR;
	
	}
	bufoffset+=savesize;
	memmoveb(packbuf,packbuf+savesize,512);
	buffer-=savesize;
	morebuff-=savesize;
	FSSeek(dir,olddiroff,FSSEEK_SET);

	}
    if(FSReadLL(morebuff,32*nentries,dir,fs)!=32*nentries) break; // ASSUME END-OF-FILE
	
	// VERIFY THAT ENTRIES ARE VALID
	ptr=morebuff;
	for(order=nentries-1;(order!=0)&& ((*ptr&0x3f)==order) ;--order,ptr+=32)
	{
		if(ptr[13]!=buffer[13]) break;		// VERIFY CHECKSUM
	}
//	printf("entries valid\n");
	if(order) {
//		printf("failed entries checksum test\n");
//		keyb_getkeyM(1);
		// ENTRIES ARE ORPHANS, DISCARD AND CONTINUE SEARCHING
		FSSeek(dir,-32*(order+1),FSSEEK_CUR);		// REWIND TO NEXT UNKNOWN ENTRY
		continue;
	}
	// VERIFY THAT SHORT ENTRY FOLLOWS LONG NAME
	
	if( ((ptr[11]&FSATTR_LONGMASK) == FSATTR_LONGNAME) || (*ptr==0) || (*ptr==0xe5)) {
//	printf("no valid shortname follows\n");
//	keyb_getkeyM(1);

	// VALID SHORT ENTRY NOT FOUND
	if(*ptr==0) break;
	if(*ptr!=0xe5) FSSeek(dir,-32,FSSEEK_CUR);		// REWIND LAST ENTRY
	continue;
	}
	
//	printf("calculating checksum\n");
	// CALCULATE CHECKSUM
	checksum=0;
	for(order=0;order<11;++order,++ptr)
	{
//	printf("%c",*ptr);
	checksum= (((checksum<<7)&0x80) | ((checksum>>1)&0x7f)) + *ptr;
	}
	
//	printf("Calc. checksum=%02X\n",checksum);
	if(checksum!=buffer[13]) {
	// FAILED CHECKSUM, SKIP ORPHANS AND CONTINUE
//	printf("failed checksum\n");
//	keyb_getkeyM(1);
	FSSeek(dir,-32,FSSEEK_CUR);		// REWIND LAST ENTRY
	continue;
	}
//	printf("All valid!!!\n");
	
	// VALID ENTRY FOUND, UPDATE BUFFER POINTERS AND CONTINUE
	
	FSMovedOpenFiles(dir,diroffset,bufoffset+(int)(buffer-packbuf),fs);

	buffer=morebuff+32*nentries;
	
	}
	
	else {
	// IT'S A SHORT NAME ENTRY
	diroffset=dir->CurrentOffset-32;
	FSMovedOpenFiles(dir,diroffset,bufoffset+(int)(buffer-packbuf),fs);
	
	buffer+=32;
	
	}

// SAVE PARTIAL BUFFER IF FULL
if( (unsigned int)(buffer-packbuf)>bufsize-32) {

	int savesize=bufsize-512;
	int olddiroff=dir->CurrentOffset;
	FSSeek(dir,bufoffset,FSSEEK_SET);
    if(FSWriteLL(packbuf,savesize,dir,fs)!=savesize) {
	// THIS BETTER NOT HAPPEN, SEVERE DAMAGE TO DIRECTORY STRUCTURES
	simpfree(packbuf);
	return FS_ERROR;
	
	}
	bufoffset+=savesize;
	memmoveb(packbuf,packbuf+savesize,512);
	buffer-=savesize;
	FSSeek(dir,olddiroff,FSSEEK_SET);

}


	
} // END MAIN WHILE LOOP



	
	// SHRINK DIRECTORY SIZE
	if( !((dir==&fs->RootDir) && (fs->FATType!=3)) ) {
	// DON'T SHRINK ROOT DIR ON FAT12/FAT16
	if(FSTruncateChain(dir,bufoffset+(int)(buffer-packbuf))==FS_OK)
	{
	dir->FileSize=FSGetChainSize(&dir->Chain);
	if(dir->FileSize<65536*32) dir->Mode&=~FSMODE_NOGROW;  // ALLOW DIRECTORY TO GROW
	}
	else {
		simpfree(packbuf);
		return FS_ERROR;
	}
	
	}




if(buffer>packbuf) {

// SAVE FINAL SECTORS

// CLEAN UNUSED ENTRIES
memsetb(buffer,0,bufsize-(int)(buffer-packbuf));

	int savesize=bufsize;
	int written;
	
	if(dir->FileSize<bufoffset+bufsize) savesize=dir->FileSize-bufoffset;
	
	FSSeek(dir,bufoffset,FSSEEK_SET);
    if((written=FSWriteLL(packbuf,savesize,dir,fs))!=savesize) {
	// THIS BETTER NOT HAPPEN, SEVERE DAMAGE TO DIRECTORY STRUCTURES
	simpfree(packbuf);
	return (written<0)? written:FS_ERROR;
	
	}
	bufoffset+=savesize;
}
	
	
	
	// CLEAN ALL ENTRIES AFTER LAST ONE
	memsetb(packbuf,0,bufsize);
	
    unsigned int savesize=dir->FileSize-bufoffset;
	int written;
	
	while(savesize>bufsize) {
    written=FSWriteLL(packbuf,bufsize,dir,fs);
    if((unsigned int)written!=bufsize) {
		// SEVERE DAMAGE TO FILE SYSTEM
		simpfree(packbuf);
		return (written<0)? written : FS_ERROR;
	}
	savesize-=bufsize;
	}
	
	if(savesize) {
        if((written=FSWriteLL(packbuf,savesize,dir,fs))!=(int)savesize) {
			// BIG PROBLEM SEVERE DAMAGE TO THE FILE SYSTEM CAN HAPPEN
			simpfree(packbuf);
			return (written<0)? written:FS_ERROR;

		}
	}
	
	simpfree(packbuf);

	// FLUSH FAT BUFFERS TO ENSURE DIR IS MODIFIED ON DISK

return	FSFlushFATCache(fs);


}


