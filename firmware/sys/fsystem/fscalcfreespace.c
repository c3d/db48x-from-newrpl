/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"


// CALCULATE FREE SPACE AND CACHE BIG EMPTY AREAS

int FSCalcFreeSpace(FS_VOLUME *fs)
{
unsigned char *buffer;
unsigned int fataddr;
int value;
unsigned int maxaddr=0;
unsigned int freecount,freestart,freesize;
int limit,f;

buffer=simpmallocb(1536);
if(!buffer) return FS_ERROR;

//if(!SDDSetBlockLen(fs->Disk,7)) { simpfree(buffer); return FS_ERROR; }

fataddr=0;
switch(fs->FATType)
{
case 1:
maxaddr=(( ((fs->TotalClusters)&(~1))+2)*3)/2;
if(fs->TotalClusters&1) maxaddr--;
break;
case 2:
maxaddr=(fs->TotalClusters+2)<<1;
break;
case 3:
maxaddr=(fs->TotalClusters+2)<<2;
break;
}
maxaddr+=fataddr;


freecount=0;
freestart=0;
freesize=0;
fs->FreeAreaSize=0;

do {

if(SDDRead((((uint64_t)fs->FirstFATAddr)<<9)+fataddr,1536,buffer,fs->Disk)!=1536) {
simpfree(buffer);
return FS_ERROR;
}

FSPatchFATBlock(buffer,1536,fataddr,fs,FALSE);		// GET UPDATED FAT INFORMATION

// SCAN FAT BLOCK
if(maxaddr-fataddr<1536) limit=maxaddr-fataddr;
else limit=1536;


switch(fs->FATType)
{
case 1:
// FAT 12 SCANNING
	for(f=0;f<limit;f+=3)
	{
		value=buffer[f]+(buffer[f+1]<<8)+(buffer[f+2]<<16);
		if(!value) {
			// PROCESS 2 CLUSTERS AT A TIME
			freecount+=2;
			if(!freestart) {
				freestart=FSFATEntry2Addr(fataddr+f,fs);
				freesize=2;
			}
			else freesize+=2;
		}
		else {
			// PROCESS EACH CLUSTER INDIVIDUALLY
			if(value&0xfff) {
				if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }
				freestart=0;
			}
			else {
				++freecount;
				if(!freestart) {
					freestart=FSFATEntry2Addr(fataddr+f,fs);
					freesize=1;
				}
				else freesize++;
			}

			if(value>>12) {
				if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }
				freestart=0;
			}
			else {
				++freecount;
				if(!freestart) {
					freestart=FSFATEntry2Addr(fataddr+f+1,fs);
					freesize=1;
				}
				else freesize++;
			}
		}

	}

	if(f!=limit) {
		// FAT HAS ODD-NUMBER OF CLUSTERS

		value=buffer[f]+((buffer[f+1]&0xf)<<8);
			// PROCESS EACH CLUSTER INDIVIDUALLY
			if(value&0xfff) {
				if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }
				freestart=0;
			}
			else {
				++freecount;
				if(!freestart) {
					freestart=FSFATEntry2Addr(fataddr+f,fs);
					freesize=1;
				}
				else freesize++;
			}

		




	}


break;

case 2:
// FAT 16 SCANNING

for(f=0;f<limit;f+=2)
{
value=*(unsigned short int *)(buffer+f);
if(!value) {
// PROCESS 2 CLUSTERS AT A TIME
freecount++;
if(!freestart) {
freestart=FSFATEntry2Addr(fataddr+f,fs);
freesize=1;
}
else freesize++;
}
else {
if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }
freestart=0;
}
}
break;

case 3:

// FAT 32 SCANNING

for(f=0;f<limit;f+=4)
{
value=*(unsigned int *)(buffer+f);
if(!value) {
// PROCESS 2 CLUSTERS AT A TIME
freecount++;
if(!freestart) {
freestart=FSFATEntry2Addr(fataddr+f,fs);
freesize=1;
}
else freesize++;
}
else {
if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }
freestart=0;
}

}
break;

}

fataddr+=1536;

} while(limit==1536);

if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }

simpfree(buffer);
fs->FreeAreaSize<<=fs->ClusterSize-9;
fs->FreeSpace=freecount<<(fs->ClusterSize-9);
fs->InitFlags|=VOLFLAG_FREESPACEVALID;
return FS_OK;

}



// LIMITED SCAN ON THE FAT (3 sectors), LOOKING FOR FREE SPACE

int FSScanFreeSpace(FS_VOLUME *fs,unsigned int nextfreecluster)
{
unsigned char *buffer;
unsigned int fataddr,startfataddr;
int value;
unsigned int maxaddr=0;
unsigned int freecount,freestart,freesize;
int limit,f;

buffer=simpmallocb(1536);
if(!buffer) return FS_ERROR;

switch(fs->FATType)
{
case 1:
maxaddr=(( ((fs->TotalClusters)&(~1))+2)*3)/2;
fataddr=(( ((nextfreecluster)&(~1)))*3)/2;
if(fataddr&1) fataddr--;
if(fs->TotalClusters&1) maxaddr--;
break;
case 2:
maxaddr=(fs->TotalClusters+2)<<1;
fataddr=nextfreecluster<<1;
break;
case 3:
maxaddr=(fs->TotalClusters+2)<<2;
fataddr=nextfreecluster<<2;
break;
}

fataddr/=1536;
fataddr*=1536;      // ALIGN TO 3 SECTOR GROUP

freecount=0;
freestart=0;
freesize=0;
fs->FreeAreaSize=0;

startfataddr=fataddr;

do {

if(SDDRead((((uint64_t)fs->FirstFATAddr)<<9)+fataddr,1536,buffer,fs->Disk)!=1536) {
simpfree(buffer);
return FS_ERROR;
}

FSPatchFATBlock(buffer,1536,fataddr,fs,FALSE);		// GET UPDATED FAT INFORMATION

// SCAN FAT BLOCK
if(maxaddr-fataddr<1536) limit=maxaddr-fataddr;
else limit=1536;


switch(fs->FATType)
{
case 1:
// FAT 12 SCANNING
    for(f=0;f<limit;f+=3)
    {
        value=buffer[f]+(buffer[f+1]<<8)+(buffer[f+2]<<16);
        if(!value) {
            // PROCESS 2 CLUSTERS AT A TIME
            freecount+=2;
            if(!freestart) {
                freestart=FSFATEntry2Addr(fataddr+f,fs);
                freesize=2;
            }
            else freesize+=2;
        }
        else {
            // PROCESS EACH CLUSTER INDIVIDUALLY
            if(value&0xfff) {
                if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }
                freestart=0;
            }
            else {
                ++freecount;
                if(!freestart) {
                    freestart=FSFATEntry2Addr(fataddr+f,fs);
                    freesize=1;
                }
                else freesize++;
            }

            if(value>>12) {
                if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }
                freestart=0;
            }
            else {
                ++freecount;
                if(!freestart) {
                    freestart=FSFATEntry2Addr(fataddr+f+1,fs);
                    freesize=1;
                }
                else freesize++;
            }
        }

    }

    if(f!=limit) {
        // FAT HAS ODD-NUMBER OF CLUSTERS

        value=buffer[f]+((buffer[f+1]&0xf)<<8);
            // PROCESS EACH CLUSTER INDIVIDUALLY
            if(value&0xfff) {
                if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }
                freestart=0;
            }
            else {
                ++freecount;
                if(!freestart) {
                    freestart=FSFATEntry2Addr(fataddr+f,fs);
                    freesize=1;
                }
                else freesize++;
            }






    }


break;

case 2:
// FAT 16 SCANNING

for(f=0;f<limit;f+=2)
{
value=*(unsigned short int *)(buffer+f);
if(!value) {
// PROCESS 2 CLUSTERS AT A TIME
freecount++;
if(!freestart) {
freestart=FSFATEntry2Addr(fataddr+f,fs);
freesize=1;
}
else freesize++;
}
else {
if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }
freestart=0;
}
}
break;

case 3:

// FAT 32 SCANNING

for(f=0;f<limit;f+=4)
{
value=*(unsigned int *)(buffer+f);
if(!value) {
// PROCESS 2 CLUSTERS AT A TIME
freecount++;
if(!freestart) {
freestart=FSFATEntry2Addr(fataddr+f,fs);
freesize=1;
}
else freesize++;
}
else {
if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }
freestart=0;
}

}
break;

}

fataddr+=limit;

if(fataddr>=maxaddr) fataddr=0;     // WRAP AROUND AND START OVER

} while( (fataddr!=startfataddr) && (freecount==0));

if(fataddr==startfataddr) {
    fs->FreeAreaSize=0;
    simpfree(buffer);
    return FS_DISKFULL;
}

if(freestart && (fs->FreeAreaSize<freesize)) { fs->NextFreeCluster=freestart; fs->FreeAreaSize=freesize; }

simpfree(buffer);
fs->FreeAreaSize<<=fs->ClusterSize-9;
return FS_OK;

}
