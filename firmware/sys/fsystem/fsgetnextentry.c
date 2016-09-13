/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"



// FILLS IN A FS_FILE STRUCTURE WITH THE NEXT DIRECTORY ENTRY
// STARTS AT CurrentOffset

int FSGetNextEntry(FS_FILE *entry,FS_FILE *dir)
{
int order,diroffset;
int nentries;
unsigned char checksum;
unsigned char buffer[32],*morebuff,*ptr;
FS_VOLUME *fs;

if(!FSystem.Init) return FS_ERROR;
if(!dir || !entry) return FS_ERROR;

fs=FSystem.Volumes[dir->Volume];

if(!fs) return FS_ERROR;

while(FSReadLL(buffer,32,dir,fs)==32)
{
if(buffer[0]==0) return FS_EOF;
if(buffer[0]==0xe5) continue;	// DELETED ENTRY, USE NEXT ENTRY
//
if((buffer[11]&FSATTR_LONGMASK) == FSATTR_LONGNAME)
{
  if((fs->FATType!=3)&&(((buffer[0x1a]|buffer[0x1b])!=0)||((buffer[0x1c]|buffer[0x1d]|buffer[0x1e]|buffer[0x1f])==0)))
   {
    // THIS IS NOT A VALID LFN ENTRY, POSSIBLY USED BY OTHER OS'S
    // JUST IGNORE IT
      continue;
    }



	// TREAT AS LONG FILENAME
	if(!(buffer[0]&0X40)) continue;		// ORPHAN ENTRY, SKIP
	diroffset=dir->CurrentOffset-32;
	// FOUND LAST ENTRY OF A NAME
	nentries=buffer[0]&0x3f;
    morebuff=simpmallocb(32*nentries);
	if(morebuff==NULL) return FS_ERROR;
    if(FSReadLL(morebuff,32*nentries,dir,fs)!=32*nentries) { simpfree(morebuff); return FS_ERROR; }
	
	// VERIFY THAT ENTRIES ARE VALID
	ptr=morebuff;
	for(order=nentries-1;(order!=0)&& ((*ptr&0x3f)==order) ;--order,ptr+=32)
	{
		if(ptr[13]!=buffer[13]) break;		// VERIFY CHECKSUM
	}
	if(order) {
		// ENTRIES ARE ORPHANS, DISCARD AND CONTINUE SEARCHING
		FSSeek(dir,-32*(order+1),FSSEEK_CUR);		// REWIND TO NEXT UNKNOWN ENTRY
		simpfree(morebuff);
		continue;
	}
	// VERIFY THAT SHORT ENTRY FOLLOWS LONG NAME
	
	if( ((ptr[11]&FSATTR_LONGMASK) == FSATTR_LONGNAME) || (*ptr==0) || (*ptr==0xe5)) {

	// VALID SHORT ENTRY NOT FOUND
	simpfree(morebuff);
	if(*ptr==0) return FS_EOF;
	if(*ptr!=0xe5) FSSeek(dir,-32,FSSEEK_CUR);		// REWIND LAST ENTRY
	continue;
	}
	
	// CALCULATE CHECKSUM
	checksum=0;
	for(order=0;order<11;++order,++ptr)
	{
	checksum= (((checksum<<7)&0x80) | ((checksum>>1)&0x7f)) + *ptr;
	}
	
	if(checksum!=buffer[13]) {
	// FAILED CHECKSUM, SKIP ORPHANS AND CONTINUE
	simpfree(morebuff);
	FSSeek(dir,-32,FSSEEK_CUR);		// REWIND LAST ENTRY
	continue;
	}
	// VALID ENTRY FOUND, FILL STRUCTURE AND RETURN
    entry->Name=(char *)simpmallocb(nentries*13*3+1);
	if(entry->Name==NULL) {
	simpfree(morebuff);
	return FS_ERROR;
	}
	
	ptr-=11;
	// REPACK LONG NAME
    char *nameptr=entry->Name;
	for(order=1;order<nentries;++order)
	{
    nameptr=FSPackName(nameptr,(char *)ptr-(order<<5));
	}
    nameptr=FSPackName(nameptr,(char *)buffer);
    *nameptr=0;		// FORCE NULL-TERMINATED STRING

	memmoveb(buffer,ptr,32);
	simpfree(morebuff);
	
	}
	
	else {
	// IT'S A SHORT NAME ENTRY
	diroffset=dir->CurrentOffset-32;
	nentries=0;
    entry->Name=(char *)simpmallocb(35);
	if(entry->Name==NULL) return FS_ERROR;
    FSPackShortName(entry->Name,(char *)buffer);
	}
	
	if(FSystem.CaseMode==FSCASE_SENSHP) FSStripSemi(entry->Name);
	// NOW FILL THE COMMON FIELDS
	entry->Volume=fs->VolNumber;
	entry->Mode=0;
	entry->Attr=buffer[11];
	entry->NTRes=buffer[12];
	entry->CrtTmTenth=buffer[13];
	entry->LastAccDate=ReadInt16(buffer+18);
	entry->CreatTimeDate=ReadInt32(buffer+14);
	entry->WriteTimeDate=ReadInt32(buffer+22);
    entry->FirstCluster=buffer[26]+(buffer[27]<<8);
    if(fs->FATType==3) entry->FirstCluster|=(buffer[20]<<16)+(buffer[21]<<24);
    entry->FileSize=ReadInt32(buffer+28);
	entry->CurrentOffset=0;
	entry->DirEntryOffset=diroffset;
	entry->DirEntryNum=nentries+1;
	entry->Dir=dir;
	memsetb((void *)&(entry->Chain),0,sizeof(FS_FRAGMENT)+sizeof(FS_BUFFER));
	return FS_OK;
}
	
if(dir->CurrentOffset>=dir->FileSize) return FS_EOF;
else return FS_ERROR;
}

