/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

// PRIVATE HEADER FILE FOR LIBRARY BUILD

#ifndef __FS_PRIV_H
#define __FS_PRIV_H

#include <newrpl.h>
#include <ui.h>


#include <fsystem.h>

#include "../sddriver.h"

// MAXIMUM NUMBER OF FILES OPENED AT THE SAME TIME PER VOLUME
#define FS_MAXOPENFILES 10
#define FS_MAXFATCACHE  10


#define FSCASE_INSENSHP 4			// CASE INSENSITIVE



struct __chainbuffer;
typedef struct __chainbuffer FS_CHAINBUFFER;

struct __chainbuffer {
struct {
unsigned int Cluster;
unsigned int EntryValue;
} Entries[16];
int Used;
FS_CHAINBUFFER *Next;
};





typedef struct {
SD_CARD *Disk;
unsigned InitFlags:16,VolNumber:16;
unsigned int VolumeAddr;
int SectorSize;
unsigned int TotalSectors;
unsigned int TotalClusters;
int ClusterSize;
int NumFATS;
unsigned int FirstFATAddr;
int FATSize;
int FATType;
unsigned int Cluster0Addr;
unsigned int NextFreeCluster,FreeAreaSize;
unsigned int FreeSpace;
FS_FILE RootDir;
FS_FILE *CurrentDir;
FS_FILE *Files[FS_MAXOPENFILES];
FS_CHAINBUFFER *FATCache;
int NumCache;
} FS_VOLUME;


typedef struct {
FS_VOLUME *Volumes[4];		// ALL MOUNTED VOLUMES
unsigned CurrentVolume:16,CaseMode:8,Init:8;			// CURRENTLY SELECTED VOLUME, Init=1 if FS was initialized
} FS_PUBLIC;


// PRIVATE STRUCTURE FOR FILE CREATION
typedef struct {
// STATISTICS ABOUT DIRECTORY
int DirMaxEntries;  // MAX. NUMBER OF ENTRIES IN DIR
int DirUsedEntries; // USED ENTRIES INCLUDING INVALID AND DELETED
int DirValidEntries;// USED ENTRIES NOT INCLUDING INVALID AND DELETED
// STATS FOR FILE CREATION
int FileExists; // TRUE/FALSE
int NeedSemi;  // NUMBER OF SEMICOLONS TO ADD (0=NOT NEEDED)
int ShortNum;  // SHORT NAME NUMBER TO ADD
int	NameFlags;	// INDICATE IF LONG NAME, ETC.
FS_FILE *Entry;
} FS_FILECREATE;



// FSINIT MADE PRIVATE, AUTOMATIC MOUNTING
/*!
 * \brief Initialize the file system, mount any volumes in the current SD card.
 *
 * This function must be called at startup. When the user changes the card,
 * it is necessary to shutdown the file system and call this function again
 * to mount the volumes in the new card.
 *
 * \return An FS_XXX error code (see error code constants)
 * \sa FSShutdown FSRestart
 */
extern int FSInit();
extern int FSIsInit();
extern int FSCardInserted();

extern void FSHardReset();

extern unsigned int ReadInt32(unsigned char *ptr);
extern void WriteInt32(unsigned char *ptr,unsigned int value);
extern unsigned int ReadInt16(unsigned char *ptr);
extern void WriteInt16(unsigned char *ptr,unsigned int value);

// ADD-ON MEMORY ALLOCATOR
extern void init_simpalloc();
extern unsigned int *simpmalloc(int words);
extern unsigned char *simpmallocb(int bytes);
extern void simpfree(void *voidptr);






// DOS-TIME PROCESSING FUNCTION
extern void FSGetDateTime(unsigned int *datetime,unsigned int *hundredths);


// VOLUME FUNCTIONS
extern int FSMountVolume(SD_CARD *Disk,FS_VOLUME *fs, int VolNumber);
extern int FSVolumePresent(FS_VOLUME *fs);
extern int FSCalcFreeSpace(FS_VOLUME *fs);


// DIRECTORY FUNCTIONS
extern int FSFindEntry(char *name,int caseflags,FS_FILE *entry,FS_FILE *dir);
extern int FSUpdateDirEntry(FS_FILE *file);
extern int FSDeleteDirEntry(FS_FILE *file);
extern void FSMovedOpenFiles(FS_FILE *dir,int entryoffset,int newoffset,FS_VOLUME *fs);
extern int FSPackDir(FS_FILE *dir);
extern int FSFindForCreation(char *name,FS_FILECREATE *cr,FS_FILE *dir);


// FILE FUNCTIONS
extern int FSReadLL(unsigned char *buffer,int nbytes,FS_FILE *file,FS_VOLUME *fs);
extern int FSWriteLL(unsigned char *buffer,int nbytes,FS_FILE *file,FS_VOLUME *fs);
extern int FSFileIsReferenced(FS_FILE *file,FS_VOLUME *fs);
extern int FSFileIsOpen(FS_FILE *file,FS_VOLUME *fs);
extern FS_FILE *FSFreeFile(FS_FILE *file);
extern int FSFindFile(char *name,FS_FILE *entry, int dirsvalid);
extern int FSFlushBuffers(FS_FILE *file);



// FAT CHAIN FUNCTIONS
extern unsigned int FSAddr2Cluster(unsigned int addr,FS_VOLUME *fs);
extern unsigned int FSCluster2FATEntry(unsigned int cluster,FS_VOLUME *fs);
extern unsigned int FSAddr2FATEntry(unsigned int addr,FS_VOLUME *fs);
extern unsigned int FSCluster2Addr(unsigned int cluster,FS_VOLUME *fs);
extern unsigned int FSFATEntry2Cluster(unsigned int addr,FS_VOLUME *fs);
extern unsigned int FSFATEntry2Addr(unsigned int addr,FS_VOLUME *fs);


extern int FSGetChain(int firstcluster,FS_FRAGMENT *fr,FS_VOLUME *fs);
extern unsigned int FSGetChainSize(FS_FRAGMENT *fr);
extern void FSFreeChain(FS_FILE *file);
extern int FSExpandChain(FS_FILE *file,unsigned int newtotalsize);
extern int FSTruncateChain(FS_FILE *file, unsigned int newsize);
extern int FSScanFreeSpace(FS_VOLUME *fs,unsigned int nextfreecluster);


extern void FSPatchFATBlock(unsigned char *buffer,int size,int addr,FS_VOLUME *fs,int flush);
extern int FSWriteFATEntry(unsigned int cluster, int value, FS_VOLUME *fs);
extern int FSFlushFATCache(FS_VOLUME *fs);


// NAME PROCESSING FUNCTIONS
extern char *FSUnicode2OEM(char *dest,char *origin,int nchars);
extern char *FSOEM2Unicode(char *origin,char *dest,int nchars);
extern char *FSPackName(char *name,char *direntry);
extern char *FSUnpackName(char *name,char *direntry);
extern void FSPackShortName(char *name,char *direntry);
extern int FSConvert2ShortEntry(char *name,int minnum);
extern int FSNameCompareRoot(char *name1,char *name2);

extern char *__fsfindchar(char *strstart,char *strend,char *chars);
extern char *__fsfindcharrev(char *strstart,char *strend,char *chars);

// PUBLIC DATA STRUCTURE
extern FS_PUBLIC FSystem;

#endif



