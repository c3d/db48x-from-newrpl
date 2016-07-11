/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"




// CREATE A NEW FILE

int FSCreate(char *name,int attr,FS_FILE **fileptr)
{
unsigned char *nptr;
FS_FILE *dir,*file;
FS_FILECREATE *cr;
FS_VOLUME *fs;
unsigned char *path=NULL;
unsigned char tname[257];
int error,k,g;
unsigned int csize;
int ntype;

*fileptr=NULL;

error=FSInit();
if(error!=FS_OK) return error;

error=FSVolumePresent(FSystem.Volumes[FSystem.CurrentVolume]);
if(error!=FS_OK) { return error; }
//printf("Create init\n");
ntype=FSGetNameType(name);
//printf("ntype=%02X\n",ntype);

if(ntype<0) return FS_BADNAME;

if((ntype&FSNAME_ENDSLASH) || (ntype&FSNAME_EMPTY)) return FS_BADNAME;

nptr=(unsigned char *)name;
// OBTAIN DIRECTORY
if(ntype&(FSNAME_HASPATH|FSNAME_HASVOL)) {
nptr=(unsigned char *)__fsfindcharrev(name,NULL,(char *)"/\\:");

// nptr CAN'T BE NULL HERE, SO NO NEED TO CHECK
path=simpmallocb((int)(nptr-(unsigned char *)name)+2);
if(!path) return FS_ERROR;
// EXTRACT DRIVE/PATH
memmoveb(path,name,(int) (nptr-(unsigned char *)name+1));
path[(int)(nptr-(unsigned char *)name)+1]=0;
++nptr;
//printf("path=%s\n",path);
dir=(FS_FILE *)simpmallocb(sizeof(FS_FILE));
if(!dir) { simpfree(path); return FS_ERROR; }

error=FSFindFile((char *)path,dir,TRUE);
if(error!=FS_OK)
{
if(error==FS_OPENDIR) {
// GET DIRECTORY ADDRESS
file=dir;
dir=dir->Dir;
simpfree(file);
}
else {
simpfree(dir);
simpfree(path);
return error;
}
}
//printf("dir=%s\n",(dir->Name)? dir->Name: "\\");
simpfree(path);
}
else {
// GET CURRENT DIRECTORY
dir=FSystem.Volumes[FSystem.CurrentVolume]->CurrentDir;
}


if(!(dir->Attr&FSATTR_DIR)) {
// GIVEN PATH IS A FILE, NOT A DIR
while(dir!=NULL) dir=FSFreeFile(dir);
return FS_BADNAME; 
}
// dir HERE IS THE DIRECTORY WHERE FILE WILL BE CREATED
// CHECK IF DISK HAS AVAILABLE FILES TO OPEN
fs=FSystem.Volumes[dir->Volume];


for(error=0;error<FS_MAXOPENFILES;++error)
{
if(fs->Files[error]==NULL) break;
}

if(error==FS_MAXOPENFILES) {
//printf("no more file handles\n");
while(dir!=NULL) dir=FSFreeFile(dir);
//printf("free returned ok\n");
return FS_MAXFILES; 
}

//printf("Files available\n");




cr=(FS_FILECREATE *)simpmallocb(sizeof(FS_FILECREATE));
if(!cr) {
while(dir!=NULL) dir=FSFreeFile(dir);
return FS_ERROR;
}




//printf("find entry\n");
error=FSFindForCreation((char *)nptr,cr,dir);

if(error!=FS_OK) {
simpfree(cr);
while(dir!=NULL) dir=FSFreeFile(dir);
return FS_ERROR;
}

if(cr->FileExists) {
FSReleaseEntry(cr->Entry);
simpfree(cr->Entry);
simpfree(cr);
//printf("free ok\n");
while(dir!=NULL) dir=FSFreeFile(dir);
//printf("dir free ok\n");
return FS_EXIST;
}

// FILE DOESN'T EXIST, CREATE ENTRY SUGGESTED BY FSFindForCreation 

// COMPLETE MISSING FIELDS IN ENTRY STRUCTURE
file=cr->Entry;

file->Mode=FSMODE_WRITE;
file->Attr=attr;
if(!(cr->NameFlags&2)) file->NTRes=cr->NameFlags&(~3);		// SAVE RESERVED CASE INFORMATION

// DETERMINE A VALID NON-CONFLICTING SHORT NAME ENTRY

stringcpy((char *)tname,file->Name);
FSConvert2ShortEntry((char *)tname,(cr->NameFlags&1)? cr->ShortNum:0);


// WRITE DIRECTORY INFORMATION
int f;

k=1;
if(cr->NameFlags&2) k+=(utf8len((char *)file->Name)+12)/13;

file->DirEntryNum=k;

//printf("k=%d\n",k);

nptr=simpmallocb((k+1)<<5);		// ALLOCATE DIRECTORY ENTRIES
if(!nptr) {
simpfree(cr);
FSFreeFile(file);
while(dir!=NULL) dir=FSFreeFile(dir);
return FS_ERROR;
}

// RESET ALL ENTRIES
memsetb(nptr,0,(k+1)<<5);

// CREATE MAIN ENTRY
path=nptr+((k-1)<<5);

//printf("nentries=%d\n",k);
// COPY SHORT NAME

memsetb(path,32,11);

for(f=0;(tname[f]!='.') && (tname[f]!=0) ;++f)
{
path[f]=tname[f];
}
if(tname[f]=='.') {
++f;
for(g=0;tname[f+g]!=0 ;++g)
{
path[g+8]=tname[f+g];
}
}
// SET FILE PROPERTIES

//printf("short=\"%s\"\n",path);

path[11]=file->Attr;
path[12]=file->NTRes;
path[13]=file->CrtTmTenth;
WriteInt32(path+14,file->CreatTimeDate);
WriteInt16(path+18,file->LastAccDate);
WriteInt16(path+20,file->FirstCluster>>16);
WriteInt32(path+22,file->WriteTimeDate);
WriteInt16(path+26,file->FirstCluster);
if(!(file->Attr&FSATTR_DIR)) WriteInt32(path+28,file->FileSize);

// CREATE LONG FILE NAME ENTRIES

if(cr->NameFlags&2) {

	// CALCULATE NAME CHECKSUM
	int checksum=0;
	for(f=0;f<11;++f)
	{
	checksum= (((checksum<<7)&0x80) | ((checksum>>1)&0x7f)) + path[f];
	}
	
	g=k-1;
    char *nameptr=file->Name;
	//printf("long entries=%d\n",g);
	while(g--) {
	path-=32;
    nameptr=FSUnpackName(nameptr,(char *)path);
	path[13]=checksum;
	path[11]=FSATTR_LONGNAME;
	path[0]=f;
	}
	//printf("All entries processed\n");
	path[0]|=0x40;

}


// WRITE THE NEW DIRECTORY ENTRY
//printf("Trying to fseek\n");
FSSeek(dir,file->DirEntryOffset,FSSEEK_SET);
//printf("Trying to write\n");


csize=FSGetChainSize(&dir->Chain);
//printf("Ok chainsize\n");

f=FSWriteLL(nptr,k<<5,dir,fs);
//printf("Ok write\n");
simpfree(nptr);

if(f!=(k<<5)) {
	// ERROR

		while(file!=NULL) file=FSFreeFile(file);
		return (f<0)? f:FS_ERROR;
}

dir->FileSize=FSGetChainSize(&dir->Chain);

if(csize!=dir->FileSize) {
// DIRECTORY IS NOW LARGER
// CLEAN THE NEW CLUSTER
nptr=simpmallocb(512);
if(nptr) {
// DON'T CARE IF ERROR, ONLY MINOR PROBLEMS TO THE O.S.
memsetb(nptr,0,512);
FSWriteLL(nptr,512-(dir->CurrentOffset&511),dir,fs);
while(dir->CurrentOffset<dir->FileSize-511) {
if(FSWriteLL(nptr,512,dir,fs)!=512) break;
}
simpfree(nptr);

}

}


// INSERT FILE IN OPEN FILE LIST

for(k=0;k<FS_MAXOPENFILES;++k)
{
if(fs->Files[k]==NULL) {
fs->Files[k]=file;
break;
}
}

if(FSystem.CaseMode==FSCASE_SENSHP) FSStripSemi(file->Name);

*fileptr=file;
simpfree(cr);
//printf("Ok create\n");

return FS_OK;

}






