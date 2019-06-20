/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


#ifndef CONFIG_NO_FSYSTEM


// WRITE DATA TO A FILE

int FSWrite(unsigned char *buffer,int nbytes,FS_FILE *file)
{


if(!FSystem.Init) return FS_ERROR;

if(!file) return FS_ERROR;

if(!(file->Mode&FSMODE_WRITE)) return FS_ERROR;	// WRITE NOT ALLOWED
if(file->Attr&FSATTR_RDONLY) return FS_ERROR;

if(file->Mode&FSMODE_APPEND) file->CurrentOffset=file->FileSize+file->WrBuffer.Used;		// ONLY AT THE END OF FILE

int totalwritten=0;

while( (file->Mode&FSMODE_WRITEBUFFERS)&& !(file->Mode&FSMODE_MODIFY)) {
    // BUFFERED WRITES
    if(!file->WrBuffer.Data) {
        file->WrBuffer.Data=simpmallocb(512);
        if(!file->WrBuffer.Data) {
            // DISABLE BUFFERED WRITES
            file->Mode&=~FSMODE_WRITEBUFFERS;
            continue;
        }
        file->WrBuffer.Used=0;
    }

    if(file->WrBuffer.Used) {
        if( (file->CurrentOffset<file->WrBuffer.Offset)||(file->CurrentOffset>file->WrBuffer.Offset+512) || (file->CurrentOffset>file->WrBuffer.Offset+file->WrBuffer.Used)) {
            // BUFFER IS NOT CONTIGUOUS, NEEDS TO BE FLUSHED
            unsigned int oldoffset=file->CurrentOffset;
            file->CurrentOffset=file->WrBuffer.Offset;
            int written=FSWriteLL(file->WrBuffer.Data,file->WrBuffer.Used,file,FSystem.Volumes[file->Volume]);
            if(!(file->Mode&FSMODE_MODIFY)) {
            file->FileSize=file->CurrentOffset;		// TRUNCATE FILE
            }
            if(written!=file->WrBuffer.Used) return FS_ERROR;
            file->CurrentOffset=oldoffset;
            file->WrBuffer.Used=0;
        }
        else {
          if(file->CurrentOffset<file->WrBuffer.Offset+file->WrBuffer.Used) {
              // OVERWRITE A FEW BYTES IN THE BUFFER, IT'S FINE
              file->WrBuffer.Used=file->CurrentOffset-file->WrBuffer.Offset;
          }

        // APPEND DATA TO THE CURRENT BUFFER
        int maxbytes=512-file->WrBuffer.Used;

        if(nbytes>maxbytes) {
            memmoveb(file->WrBuffer.Data+file->WrBuffer.Used,buffer,maxbytes);
            file->WrBuffer.Used=512;
            // FLUSH THE BUFFER
            file->CurrentOffset=file->WrBuffer.Offset;
            int written=FSWriteLL(file->WrBuffer.Data,file->WrBuffer.Used,file,FSystem.Volumes[file->Volume]);
            if(!(file->Mode&FSMODE_MODIFY)) {
            file->FileSize=file->CurrentOffset;		// TRUNCATE FILE
            }
            totalwritten+=maxbytes;
            if(written!=file->WrBuffer.Used) return totalwritten;
            file->WrBuffer.Used=0;
            nbytes-=maxbytes;
            buffer+=maxbytes;
        }
        else {
         // THE NEW DATA FITS WITHIN THE BUFFER
            memmoveb(file->WrBuffer.Data+file->WrBuffer.Used,buffer,nbytes);
            file->WrBuffer.Used+=nbytes;
            file->CurrentOffset+=nbytes;

            return nbytes;
        }



        }
    }

        // HERE IT IS GUARANTEED THAT ANY OLD BUFFERS WERE FLUSHED


            // WRITE LARGE BLOCKS DIRECTLY
            int remainder=(file->CurrentOffset+nbytes)&511;
            if(nbytes>remainder) {
            int written=FSWriteLL(buffer,nbytes-remainder,file,FSystem.Volumes[file->Volume]);
            if(!(file->Mode&FSMODE_MODIFY)) {
            file->FileSize=file->CurrentOffset;		// TRUNCATE FILE
            }
            totalwritten+=written;
            if(written!=nbytes-remainder) return totalwritten;
            buffer+=nbytes-remainder;
            nbytes=remainder;
            }

        if(nbytes) {
         // AND BUFFER THE REST
            memmoveb(file->WrBuffer.Data,buffer,nbytes);
            file->WrBuffer.Offset=file->CurrentOffset;
            file->WrBuffer.Used=nbytes;
            totalwritten+=nbytes;
            file->CurrentOffset+=nbytes;


        }
        return totalwritten;
    }


// UNBUFFERED WRITES HERE


// CHANGE THIS WITH BUFFERED VERSION WHEN AVAILABLE
nbytes=FSWriteLL(buffer,nbytes,file,FSystem.Volumes[file->Volume]);

if(!(file->Mode&FSMODE_MODIFY)) {
file->FileSize=file->CurrentOffset;		// TRUNCATE FILE
}

return nbytes;
}
#endif
