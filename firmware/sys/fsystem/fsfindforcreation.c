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

int FSFindForCreation(char *name,FS_FILECREATE *cr,FS_FILE *dir)
{
FS_VOLUME *fs;
int order,diroffset;
int namelen;
int nentries;
unsigned char checksum;
int nbytesread;
unsigned char buffer[32],*morebuff;
unsigned char *ptr;
unsigned char shortn[35];
unsigned char *shname;
unsigned char *newname;
unsigned char *entryname;

if(!FSystem.Init) return FS_ERROR;
if(!dir || !cr) return FS_ERROR;

fs=FSystem.Volumes[dir->Volume];

if(!fs) return FS_ERROR;

newname=simpmallocb(257);
if(!newname) return FS_ERROR;
shname=simpmallocb(257);
if(!shname) {simpfree(newname); return FS_ERROR; }
entryname=simpmallocb(257);
if(!entryname) { simpfree(newname); simpfree(shname); return FS_ERROR; }

// INITIALIZE COUNTERS

memsetb((void *)cr,0,sizeof(FS_FILECREATE));

if(dir->Mode&FSMODE_NOGROW) cr->DirMaxEntries=dir->FileSize>>5;
else cr->DirMaxEntries=65536;

// DETERMINE LENGTH OF STRING
namelen=(int)stringlen((char *)name);

// SAFE OBTAIN NAME OF NEW FILE
if(namelen>255) {
	memmoveb(newname,name,255);
	newname[255]=0;		// TRUNCATE FILE NAME IF >255 CHARS
}
else stringcpy((char *)newname,(char *)name);

// STRIP SEMICOLONS IF PRESENT
if( FSystem.CaseMode==FSCASE_SENSHP || (FSystem.CaseMode==FSCASE_SENSHPTRUE)) FSStripSemi((char *)newname);

namelen=stringlen((char *)newname);

// EXTRACT ROOT NAME TO CHECK FOR CONFLICTS

stringcpy((char *)shname,(char *)newname);

cr->NameFlags=FSConvert2ShortEntry((char *)shname,0);

if(cr->NameFlags&1) cr->ShortNum=1;			// MINIMUM NUMBER NEEDED =1



// READY TO BEGIN DIRECTORY SCAN

//printf("Search... %s\n",name);
//printf("dir size=%d\n",dir->FileSize);
//keyb_getkeyM(1);
FSSeek(dir,0,FSSEEK_SET);

while((nbytesread=FSReadLL(buffer,32,dir,fs))==32)
{
//printf("x");
if(buffer[0]==0) break;
++cr->DirUsedEntries;
if(buffer[0]==0xe5) continue;	// DELETED ENTRY, USE NEXT ENTRY
if( (buffer[11]&FSATTR_LONGMASK) == FSATTR_LONGNAME) {

//	printf("LFN entry found\n");
	// TREAT AS LONG FILENAME
	if(!(buffer[0]&0X40)) continue;		// ORPHAN ENTRY, SKIP
	diroffset=dir->CurrentOffset-32;
//	printf("last entry\n");
	// FOUND LAST ENTRY OF A NAME
	nentries=buffer[0]&0x3f;
    morebuff=simpmallocb(32*nentries);
	if(morebuff==NULL) {
		simpfree(newname);
		simpfree(shname);
		simpfree(entryname);
		if(cr->Entry) simpfree(cr->Entry); 
		return FS_ERROR;
	}
    if( (nbytesread=FSReadLL(morebuff,32*nentries,dir,fs))!=32*nentries) {
		simpfree(morebuff);
		if(nbytesread>0) cr->DirUsedEntries+=nbytesread>>5;

		if(FSEof(dir)) break;
		simpfree(newname);
		simpfree(shname);
		simpfree(entryname);
		if(cr->Entry) simpfree(cr->Entry); 
		return FS_ERROR;
	}
	
	cr->DirUsedEntries+=nentries;

//	printf("Read %d entries\n",nentries);
//	keyb_getkeyM(1);

//	printf("Checking validity\n");
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
		cr->DirUsedEntries-=order+1;
		simpfree(morebuff);
		continue;
	}
	// VERIFY THAT SHORT ENTRY FOLLOWS LONG NAME
	
	if( ((ptr[11]&FSATTR_LONGMASK) == FSATTR_LONGNAME) || (*ptr==0) || (*ptr==0xe5)) {
//	printf("no valid shortname follows\n");
//	keyb_getkeyM(1);

	// VALID SHORT ENTRY NOT FOUND
	simpfree(morebuff);
	if(*ptr==0) { cr->DirUsedEntries--; break; }
	if(*ptr!=0xe5) { FSSeek(dir,-32,FSSEEK_CUR);		// REWIND LAST ENTRY
					cr->DirUsedEntries--;
	}
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
	simpfree(morebuff);
	FSSeek(dir,-32,FSSEEK_CUR);		// REWIND LAST ENTRY
	--cr->DirUsedEntries;
	continue;
	}
//	printf("All valid!!!\n");
	
	// VALID ENTRY FOUND, FILL STRUCTURE AND RETURN

	cr->DirValidEntries+=nentries+1;

	ptr-=11;
	// REPACK LONG NAME
    unsigned char *entrynameptr=entryname;
	for(order=1;order<nentries;++order)
	{
    entrynameptr=(unsigned char *)FSPackName((char *)entrynameptr,(char *)ptr-(order<<5));
	}
    entrynameptr=(unsigned char *)FSPackName((char *)entrynameptr,(char *)buffer);
    *entrynameptr=0;		// FORCE NULL-TERMINATED STRING

	memmoveb(buffer,ptr,32);		// COPY MAIN (SHORT) ENTRY TO buffer
	simpfree(morebuff);
	


	// OBTAINED ENTRY, NOW COMPARE WITH NEW ENTRY
        FSPackShortName((char *)shortn,(char *)buffer);


	// FIRST CHECK IF ENTRY IS THE FILE WE SEARCH FOR
	
	
		// CHECK IF LONG NAME MATCHES
        if(FSNameCompare((char *)entryname,(char *)newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSystem.CaseMode) ||
		// CHECK IF SHORT NAME MATCHES
        FSNameCompare((char *)shortn,(char *)newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSystem.CaseMode))

		{		// FILE EXISTS


		if(!cr->FileExists) {		// DON'T CHECK FOR DUPLICATED FILES
		cr->FileExists=TRUE;
		cr->Entry=(FS_FILE *)simpmallocb(sizeof(FS_FILE));

		if(!cr->Entry) {
		simpfree(newname);
		simpfree(shname);
		simpfree(entryname);
		return FS_ERROR;
		}

        cr->Entry->Name=(char *)simpmallocb(stringlen((char *)entryname)+1);

		if(!cr->Entry->Name) {
		simpfree(newname);
		simpfree(shname);
		simpfree(entryname);
		return FS_ERROR;
		}

        stringcpy(cr->Entry->Name,(char *)entryname);
		if(FSystem.CaseMode==FSCASE_SENSHP) FSStripSemi(cr->Entry->Name);

		cr->Entry->Mode=0;
		cr->Entry->Volume=fs->VolNumber;
		cr->Entry->Attr=buffer[11];
		cr->Entry->NTRes=buffer[12];
		cr->Entry->CrtTmTenth=buffer[13];
		cr->Entry->LastAccDate=ReadInt16(buffer+18);
		cr->Entry->CreatTimeDate=ReadInt32(buffer+14);
		cr->Entry->WriteTimeDate=ReadInt32(buffer+22);
		cr->Entry->FirstCluster=buffer[26]+(buffer[27]<<8)+(buffer[20]<<16)+(buffer[21]<<24);
		cr->Entry->FileSize=ReadInt32(buffer+28);
		cr->Entry->CurrentOffset=0;
		cr->Entry->DirEntryOffset=diroffset;
		cr->Entry->DirEntryNum=nentries+1;
		cr->Entry->Dir=dir;
		memsetb((void *)&(cr->Entry->Chain),0,sizeof(FS_FRAGMENT));
		memsetb((void *)&(cr->Entry->RdBuffer),0,sizeof(FS_BUFFER));
		memsetb((void *)&(cr->Entry->WrBuffer),0,sizeof(FS_BUFFER));
		}

		}
		else {		// IF NOT EXACT MATCH, CHECK FOR APPROXIMATE MATCHES

	// CHECK FOR POSSIBLE CASE CONFLICT

			if(FSystem.CaseMode!=FSCASE_INSENS) {
                if(FSNameCompare((char *)entryname,(char *)newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSCASE_INSENSHP)) {
					// CASE CONFLICT - DETERMINE HOW MANY SEMICOLONS TO ADD
                    unsigned char *tmp=entryname;
					int semis=0;

					while(*tmp) ++tmp;
					--tmp;
					while( (*tmp==';') && (tmp>=entryname)) { tmp--; ++semis; }

					if(cr->NeedSemi<=semis) cr->NeedSemi=semis+1;

				}
			}


	// CHECK FOR POSSIBLE CASE CONFLICT (SHORT)
			if(FSystem.CaseMode!=FSCASE_INSENS) {
                if(FSNameCompare((char *)shortn,(char *)newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSCASE_INSENSHP)) {
					// CASE CONFLICT - DETERMINE HOW MANY SEMICOLONS TO ADD
					if(!cr->NeedSemi) cr->NeedSemi=1;

				}
			}


	// CHECK FOR POSSIBLE ROOT/NUMBER CONFLICT
			{
                int rootn=FSNameCompareRoot((char *)shortn,(char *)shname);
				if(cr->ShortNum<=rootn)
					cr->ShortNum=rootn+1;
			}



		}
		}
	
	else {
        FSPackShortName((char *)shortn,(char *)buffer);
		
		diroffset=dir->CurrentOffset-32;

		++cr->DirValidEntries;


	// FIRST CHECK IF ENTRY IS THE FILE WE SEARCH FOR
	
	
		// CHECK IF SHORT NAME MATCHES
            if(FSNameCompare((char *)shortn,(char *)newname,FSystem.CaseMode))

		{		// FILE EXISTS


		if(!cr->FileExists) {		// DON'T CHECK FOR DUPLICATED FILES
		cr->FileExists=TRUE;
		cr->Entry=(FS_FILE *)simpmallocb(sizeof(FS_FILE));

		if(!cr->Entry) {
		simpfree(newname);
		simpfree(shname);
		simpfree(entryname);
		return FS_ERROR;
		}

        cr->Entry->Name=(char *)simpmallocb(stringlen((char *)shname)+1);

		if(!cr->Entry->Name) {
		simpfree(newname);
		simpfree(shname);
		simpfree(entryname);
		return FS_ERROR;
		}

        stringcpy(cr->Entry->Name,(char *)shname);
		if(FSystem.CaseMode==FSCASE_SENSHP) FSStripSemi(cr->Entry->Name);

		cr->Entry->Mode=0;
		cr->Entry->Volume=fs->VolNumber;
		cr->Entry->Attr=buffer[11];
		cr->Entry->NTRes=buffer[12];
		cr->Entry->CrtTmTenth=buffer[13];
		cr->Entry->LastAccDate=ReadInt16(buffer+18);
		cr->Entry->CreatTimeDate=ReadInt32(buffer+14);
		cr->Entry->WriteTimeDate=ReadInt32(buffer+22);
		cr->Entry->FirstCluster=buffer[26]+(buffer[27]<<8)+(buffer[20]<<16)+(buffer[21]<<24);
		cr->Entry->FileSize=ReadInt32(buffer+28);
		cr->Entry->CurrentOffset=0;
		cr->Entry->DirEntryOffset=diroffset;
		cr->Entry->DirEntryNum=1;
		cr->Entry->Dir=dir;
		memsetb((void *)&(cr->Entry->Chain),0,sizeof(FS_FRAGMENT));
		memsetb((void *)&(cr->Entry->RdBuffer),0,sizeof(FS_BUFFER));
		memsetb((void *)&(cr->Entry->WrBuffer),0,sizeof(FS_BUFFER));
		}

		}
		else {		// IF NOT EXACT MATCH, CHECK FOR APPROXIMATE MATCHES

	// CHECK FOR POSSIBLE CASE CONFLICT

			if(FSystem.CaseMode!=FSCASE_INSENS) {
                if(FSNameCompare((char *)shortn,(char *)newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSCASE_INSENSHP)) {
					// CASE CONFLICT - DETERMINE HOW MANY SEMICOLONS TO ADD
                    unsigned char *tmp=entryname;
					int semis=0;

					while(*tmp) ++tmp;
					--tmp;
					while( (*tmp==';') && (tmp>=entryname)) { tmp--; ++semis; }

					if(cr->NeedSemi<=semis) cr->NeedSemi=semis+1;

				}
			}


	// CHECK FOR POSSIBLE CASE CONFLICT (SHORT)
			if(FSystem.CaseMode!=FSCASE_INSENS) {
                if(FSNameCompare((char *)shortn,(char *)newname,(FSystem.CaseMode==FSCASE_SENS)? FSCASE_INSENS:FSCASE_INSENSHP)) {
					// CASE CONFLICT - DETERMINE HOW MANY SEMICOLONS TO ADD
					if(!cr->NeedSemi) cr->NeedSemi=1;

				}
			}


	// CHECK FOR POSSIBLE ROOT/NUMBER CONFLICT
			{
                int rootn=FSNameCompareRoot((char *)shortn,(char *)shname);
				if(cr->ShortNum<rootn) cr->ShortNum=rootn+1;
			}

		}
	}
	
}

if(cr->NeedSemi) cr->NameFlags|=3; // FORCE LONG NAME W/SHORT NAME TAIL


simpfree(entryname);
simpfree(shname);

// ANALIZE WHETHER DIRECTORY NEEDS REPACKING

if(cr->FileExists) {
simpfree(newname);
	return FS_OK;	// DON'T REPACK IF FILE EXISTS, NO CREATION WILL BE DONE
}
// REPACK IF MORE THAN 100 ENTRIES TOTAL AND (BAD_ENTRIES>=VALID_ENTRIES)
if( ((cr->DirUsedEntries-cr->DirValidEntries>cr->DirValidEntries>>1) && (cr->DirUsedEntries>100))
   // ALSO REPACK IF BAD_ENTRIES>500
   || (cr->DirUsedEntries-cr->DirValidEntries>500)
   // ALSO REPACK IF LESS THAN 100 ENTRIES LEFT TO USE IN DIRECTORY
   || ((cr->DirUsedEntries-cr->DirValidEntries>0) && (cr->DirMaxEntries-cr->DirUsedEntries<100)))
{
int error=FSPackDir(dir);
if(error!=FS_OK) {
simpfree(newname);
return error;
}
// DONE WITH PACKING

cr->DirUsedEntries=cr->DirValidEntries;
}


// CREATE ENTRY

		cr->Entry=(FS_FILE *)simpmallocb(sizeof(FS_FILE));

		if(!cr->Entry) {
		simpfree(newname);
		return FS_ERROR;
		}

        cr->Entry->Name=(char *)simpmallocb(stringlen((char *)newname)+1+cr->NeedSemi);

		if(!cr->Entry->Name) {
		simpfree(newname);
		return FS_ERROR;
		}

		stringcpy((char *)cr->Entry->Name,(char *)newname);

		simpfree(newname);

		// ADD SEMICOLONS

        entryname=(unsigned char *)cr->Entry->Name;

		while(*entryname) ++entryname;

		int f=cr->NeedSemi;
		while(f) {
		*entryname=';';
		++entryname;
		--f;
		}
		*entryname=0;

		cr->Entry->Volume=dir->Volume;
		cr->Entry->Mode=0;
		cr->Entry->Attr=0;
		cr->Entry->NTRes=0;
		FSGetDateTime(&cr->Entry->CreatTimeDate,&cr->Entry->WriteTimeDate);
		cr->Entry->CrtTmTenth=cr->Entry->WriteTimeDate;
		cr->Entry->LastAccDate=cr->Entry->CreatTimeDate>>16;
		cr->Entry->WriteTimeDate=cr->Entry->CreatTimeDate;
		cr->Entry->FirstCluster=0;
		cr->Entry->FileSize=0;
		cr->Entry->CurrentOffset=0;
		cr->Entry->DirEntryOffset=cr->DirUsedEntries<<5;	// DirEntryNum initialized later
		cr->Entry->Dir=dir;
		memsetb((void *)&(cr->Entry->Chain),0,sizeof(FS_FRAGMENT));
		memsetb((void *)&(cr->Entry->RdBuffer),0,sizeof(FS_BUFFER));
		memsetb((void *)&(cr->Entry->WrBuffer),0,sizeof(FS_BUFFER));

		// OTHERWISE RETURN WITH ALL INFORMATION COLLECTED
		


return FS_OK;

}
