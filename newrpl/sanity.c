/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"
#include "newrpl.h"
#include "sysvars.h"
#include "hal_api.h"

// GENERAL SYSTEM SANITY CHECK IMPLEMENTATION

// DETECT DATA CORRUPTION AND PREVENT BAD OBJECTS FROM BEING IMPORTED

// TAKE AN OBJECT AND CALL THE LIBRARY TO VALIDATE ITS DATA PAYLOAD
// RETURN TRUE IF OBJECT IS VALID, ZERO IF NOT

// DOES NOT CHECK FOR VALIDITY OF THE OBJECT POINTER

int32_t rplVerifyObject(word_p obj)
{
    int libnum = LIBNUM(*obj);
    LIBHANDLER han = rplGetLibHandler(libnum);
    if(!han)
        return 0;
    // EXECUTE THE LIBRARY DIRECTLY
    int32_t SavedOpcode = CurOpcode;

    CurOpcode = MK_OPCODE(libnum, OPCODE_CHECKOBJ);

    RetNum = ERR_INVALID;
    ObjectPTR = obj;

    (*han) ();

    CurOpcode = SavedOpcode;

    if(RetNum != OK_CONTINUE)
        return 0;

    return 1;
}

int32_t rplIsTempObPointer(word_p ptr)
{
// CHECK IF POINTER IS WITHIN TEMPOB
    if((ptr >= TempOb) && (ptr < TempObEnd))
        return 1;
    return 0;
}

// VERIFY IF A POINTER TO AN OBJECT IS VALID
// OBJECT POINTERS HAVE TO EITHER:
// A) POINT WITHIN TEMPOB
// B) POINT TO AN OBJECT IN ROM

int32_t rplVerifyObjPointer(word_p ptr)
{
// CHECK IF POINTER IS WITHIN TEMPOB
    if((ptr >= TempOb) && (ptr < TempObEnd))
        return 1;

// POINTER IS NOT, CHECK IF IT'S A VALID ROM POINTER
// BY ASKING LIBRARIES TO RECOGNIZE IT
    int32_t libnum = MAXLIBNUMBER;
    int32_t SavedOpcode;
    LIBHANDLER han;

    SavedOpcode = CurOpcode;
    CurOpcode = MK_OPCODE(libnum, OPCODE_GETROMID);

    while(libnum >= 0) {
        han = rplGetLibHandler(libnum);
        if(han) {
            RetNum = ERR_INVALID;
            ObjectPTR = ptr;

            (*han) ();

            if(RetNum == OK_CONTINUE)
                return 1;
        }
        libnum = rplGetNextLib(libnum);
    }
    CurOpcode = SavedOpcode;

// BAD POINTER
    return 0;

}

// CHECK DATA STACK INTEGRITY
// DATA STACK CAN ONLY CONTAIN VALID OBJECT POINTERS

// WILL REPLACE INVALID POINTERS WITH zero_bint IF fix IS TRUE
// OTHERWISE LEAVES THE INVALID DATA AS-IS
// RETURNS 0 IF INVALID POINTERS FOUND (EVEN IF PATCHED)
// RETURNS 1 IF ALL POINTERS ON STACK ARE VALID AND POINT TO VALID OBJECTS

int32_t rplVerifyDStack(int32_t fix)
{
    int32_t errors = 0;
    word_p *stkptr = DSTop - 1;
    word_p *bottom = DStkBottom;

    do {

        while(stkptr >= bottom) {
            if(!rplVerifyObjPointer(*stkptr)) {
                if(!fix)
                    return 0;
                // FIX THE BAD POINTER
                *stkptr = (word_p) zero_bint;
                ++errors;
            }
            else {
                // POINTER IS GOOD, CHECK OBJECT
                if(!rplVerifyObject(*stkptr)) {
                    if(!fix)
                        return 0;
                    *stkptr = (word_p) zero_bint;
                    ++errors;
                }
            }
            --stkptr;
        }

// FINISHED CURRENT STACK, DO THE NEXT SNAPSHOT
        if(bottom > DStk) {
            --bottom;
            int32_t levels = (intptr_t) * bottom;
            if((levels <= 0) || ((bottom - levels) < DStk)
                    || ((bottom - levels) >= DSTop)) {
                // INVALID SNAPSHOT!!
                if(!fix)
                    return 0;
                *bottom = (word_p) (bottom - DStk);
                ++errors;
            }
        }
        else
            break;
    }
    while(bottom >= DStk);

    if(errors)
        return 0;
    return 1;
}

// CHECK RETURN STACK INTEGRITY
// RETURN STACK CAN ONLY CONTAIN VALID POINTERS TO OBJECTS OR OPCODES
// WITH ONE EXCEPTION: THE DATA STACK PROTECTION FEATURE STORES POINTERS
// INTO THE DATA STACK

// LEAVES THE INVALID POINTERS AS-IS, SINCE "FIXING" POINTERS COULD DESTROY THE
// PROGRAM FLOW, THE ONLY FIX IS TO EXIT THE RPL INTERPRETER AND FLUSH THE RETURN STACK

// RETURNS 0 IF INVALID POINTERS FOUND
// RETURNS 1 IF ALL POINTERS ON STACK ARE VALID AND POINT TO VALID OBJECTS

int32_t rplVerifyRStack()
{
    word_p *stkptr = RSTop - 1;

    while(stkptr >= RStk) {
        if((stkptr >= DStk) && (stkptr < DSTop)) {
            // POINTERS INTO THE DATA STACK ARE OK
            --stkptr;
            continue;
        }
        if(!rplVerifyObjPointer(*stkptr))
            return 0;
        else {
            // POINTER IS GOOD, CHECK OBJECT
            if(!rplVerifyObject(*stkptr))
                return 0;
        }
        --stkptr;
    }

    // ALL POINTERS GOOD
    return 1;

}

// VERIFY TEMPBLOCKS LAYOUT IS CORRECT
// TEMPBLOCKS POINTERS GO INTO TEMPOB
// NOT NECESSARILY INTO VALID OBJECTS
// BUT THEY MUST BE MONOTONICALLY INCREASING

// RETURNS TRUS IF TEMPOB LAYOUT IS VALID

int32_t rplVerifyTempOb(int32_t fix)
{
    int32_t errors = 0;
    word_p *tbptr = TempBlocks;
    word_p prevptr = 0;

    while(tbptr < TempBlocksEnd) {
        if((*tbptr < TempOb) || (*tbptr >= TempObEnd) || (*tbptr <= prevptr)) {
            if(!fix)
                return 0;
            // FIX IT BY DELETING THE BLOCK, EFFECTIVELY FUSING IT WITH
            // THE NEXT BLOCK IN THE CHAIN
            memmovew(tbptr, tbptr + 1, TempBlocksEnd - tbptr - 1);
            --TempBlocksEnd;
            ++errors;
            continue;
        }
        prevptr = *tbptr;
        ++tbptr;
    }

    // ALL BLOCKS CHECKED
    if(errors)
        return 0;
    return 1;
}

// AUX FUNCTION TO CREATE A UNIQUE IDENT
// CREATES AN IDENT IN TEMPOB (ALLOCATING MEMORY= POSSIBLE GC)
// USING THE GIVEN NUMBER, MAXIMUM 8 LETTERS

word_p rplCreateHashIdent(int32_t number)
{
    int64_t bignumber =
            ((int64_t) number << 32) ^ ((int64_t) number << 24) ^ ((int64_t) number
            << 16) ^ ((int64_t) number << 8) ^ number;
    bignumber &= 0x0f0f0f0f0f0f0f00LL;
    bignumber += 0x414141414141412ELL;

    word_p obj = rplAllocTempOb(2);
    if(!obj)
        return 0;
    obj[0] = MK_PROLOG(DOIDENT, 2);
    obj[1] = (WORD) bignumber;
    obj[2] = (WORD) (bignumber >> 32);

    return obj;
}

// CHECK DIRECTORY INTEGRITY
// DIRECTORIES CAN ONLY CONTAIN VALID OBJECT POINTERS

// WILL REPLACE INVALID POINTERS WITH zero_bint IF fix IS TRUE
// OTHERWISE LEAVES THE INVALID DATA AS-IS
// RETURNS 0 IF INVALID POINTERS FOUND (EVEN IF PATCHED)
// RETURNS 1 IF ALL POINTERS ON STACK ARE VALID AND POINT TO VALID OBJECTS

extern const WORD const dir_start_bint[];
extern const WORD const dir_end_bint[];
extern const WORD const dir_parent_bint[];
extern const WORD const root_dir_handle[];

int32_t rplVerifyDirectories(int32_t fix)
{
    int32_t errors = 0;
    word_p *dirptr = Directories, *dirptr2, *dirend;
    word_p handle, parent, name;
    int32_t nitems, dirstate;

// PASS 1 - SCAN DIRECTORY STRUCTURE ONLY

    while(dirptr < DirsTop) {
        if(dirptr[0] != dir_start_bint) {
            // INVALID DIRECTORY START
            if(!fix)
                return 0;
            // FORCE A START MARKER HERE
            dirptr[0] = (word_p) dir_start_bint;
            ++errors;
        }
        if(dirptr[2] != dir_parent_bint) {
            // INVALID PARENT MARKER
            if(!fix)
                return 0;
            // FORCE A PARENT MARKER HERE
            dirptr[2] = (word_p) dir_parent_bint;
            ++errors;
        }

        // FIND THE END OF THE DIRECTORY
        dirend = dirptr + 4;
        while((dirend < DirsTop)) {
            if(!rplVerifyObjPointer(*dirend)) {
                // BAD OBJECT MIGHT BE THE DIRECTORY END?
                if(dirend[1] == dirptr[1]) {
                    // WE FOUND THE END OF THIS DIR, REPAIR IT
                    if(!fix)
                        return 0;
                    dirend[0] = (word_p) dir_end_bint;
                    break;
                }
            }
            else {
                if(**dirend == DIR_END_MARKER)
                    break;
            }
            dirend += 2;
        }

        if(dirend >= DirsTop) {
            // THERE'S NO END IN SIGHT!
            if(!fix)
                return 0;
            dirend[0] = (word_p) dir_end_bint;
            dirend[1] = dirptr[1];
            DirsTop = dirend + 2;
            if(DirSize <= DirsTop - Directories + DIRSLACK)
                growDirs((WORD) (DirsTop - Directories + DIRSLACK + 1024));
            if(Exceptions)
                return 0;
        }

        // HERE WE FOUND/CREATED AN END
        nitems = (dirend - dirptr - 4) >> 1;

        // CHECK IF THE HANDLE IS GOOD

        if(!rplIsTempObPointer(dirptr[1]))
            handle = 0;
        else {
            if(ISDIR(*dirptr[1]))
                handle = dirptr[1];
            else
                handle = 0;
        }

        // IF THE HANDLE WAS BAD, TRY TO RESTORE IT FROM THE END MARKER

        if(!handle) {
            if(!rplIsTempObPointer(dirend[1]))
                handle = 0;
            else {
                if(ISDIR(*dirend[1]))
                    handle = dirend[1];
                else
                    handle = 0;
            }

        }

        if(handle) {
            // WE HAVE A HANDLE, CHECK IF IT WAS USED BEFORE
            dirptr2 = Directories;

            while(dirptr2 < dirptr) {

                if((*dirptr2 == dir_start_bint) || (*dirptr2 == dir_end_bint)) {
                    if(dirptr2[1] == handle) {
                        handle = 0;     // HANDLE WAS USED, CREATE A NEW ONE
                        break;
                    }
                }
                dirptr2 += 2;
            }
        }

        // HERE, WE EITHER HAVE A HANDLE OR WE DON'T

        if(!handle) {
            if(!fix)
                return 0;
            // NEED TO CREATE A HANDLE IN TEMPOB
            handle = rplAllocTempOb(1);
            if(!handle)
                return 0;       // PANIC, CAN'T FIX IT!
            handle[0] = MK_PROLOG(DODIR, 1);
            handle[1] = nitems;
        }

        // HERE WE ARE GUARANTEED TO HAVE A HANDLE

        if(dirptr[1] != handle) {
            // START HANDLE IS BAD, FIX IT
            if(!fix)
                return 0;
            dirptr[1] = handle;
        }
        if(dirend[1] != handle) {
            // THE END HANDLE IS BAD, FIX IT
            if(!fix)
                return 0;
            dirend[1] = handle;
        }

        // CHECK FOR CORRECT NUMBER OF ITEMS IN THE HANDLE

        if(handle[1] != (WORD) nitems) {
            if(!fix)
                return 0;
            handle[1] = nitems;
        }

        // PARENT HANDLE CHECK REQUIRES SCANNING

        parent = dirptr[3];

        if(!rplIsTempObPointer(parent)) {
            if(parent != root_dir_handle) {
                // BAD HANDLE POINTER
                if(!fix)
                    return 0;
                // SCAN FOR A POSSIBLE PARENT DIR ENTRY WITH OUR HANDLE
                parent = (word_p) root_dir_handle;
                word_p *scan = Directories + 1;
                while(scan < DirsTop) {
                    if((scan >= dirptr) && (scan < dirend)) {
                        scan = dirend + 3;
                        continue;
                    }
                    if(*scan == handle)
                        break;
                    scan += 2;
                }

                if((scan < DirsTop) && (*scan == handle)) {
                    // FOUND POSSIBLE PARENT
                    if(rplVerifyObjPointer(*(scan - 1))) {
                        if(ISIDENT(**(scan - 1))) {
                            // THE ENTRY IS GOOD - FIND THE DIRECTORY HANDLE
                            --scan;
                            while(scan >= Directories) {
                                if(rplVerifyObjPointer(*scan)) {
                                    if(**scan == DIR_START_MARKER)
                                        break;
                                }
                                scan -= 2;
                            }
                            if(ISDIR(**(scan + 1))) {
                                // WE HAVE A GOOD HANDLE
                                parent = *(scan + 1);
                            }
                        }

                    }
                }
                // HERE EITHER WE HAVE A NEW PARENT HANDLE OR IT'S ROOT
                dirptr[3] = parent;
            }

        }
        else {
            // PARENT IS AN OBJECT IN TEMPOB
            word_p pcandidate;
            // SCAN FOR A POSSIBLE PARENT DIR ENTRY WITH OUR HANDLE
            pcandidate = (word_p) root_dir_handle;
            word_p *scan = Directories + 1;
            while(scan < DirsTop) {
                if((scan >= dirptr) && (scan < dirend)) {
                    scan = dirend + 3;
                    continue;
                }
                if(*scan == handle)
                    break;
                scan += 2;
            }

            if((scan < DirsTop) && (*scan == handle)) {
                // FOUND POSSIBLE PARENT
                if(rplVerifyObjPointer(*(scan - 1))) {
                    if(ISIDENT(**(scan - 1))) {
                        // THE ENTRY IS GOOD - FIND THE DIRECTORY HANDLE
                        --scan;
                        while(scan >= Directories) {
                            if(rplVerifyObjPointer(*scan)) {
                                if(**scan == DIR_START_MARKER)
                                    break;
                            }
                            scan -= 2;
                        }
                        if(ISDIR(**(scan + 1))) {
                            // WE HAVE A GOOD HANDLE
                            pcandidate = *(scan + 1);
                        }
                    }

                }
            }
            // HERE EITHER WE HAVE A PARENT CANDIDATE HANDLE OR IT'S ROOT
            if(parent != pcandidate) {
                if(!fix)
                    return 0;
                // FIX THE PARENT, USE THE CANDIDATE
                if(ISDIR(*pcandidate))
                    dirptr[3] = pcandidate;
            }
            else if(!ISDIR(*parent)) {
                if(!fix)
                    return 0;
                // MAKE THE PARENT ROOT, EVEN THOUGH IT'S NOT LINKED FROM THERE
                dirptr[3] = (word_p) root_dir_handle;
            }

        }

        dirptr = dirend + 2;

    }

// HERE WE HAVE A PROPER DIRECTORY STRUCTURE, BUT THERE'S NO GUARANTEE THAT
// THE CONTENTS ARE GOOD
// AT LEAST WE CAN SCAN THE DIRECTORIES SAFELY

// PASS 2 - CHECK INTERIOR ITEMS AND DELETE AS NEEDED

    nitems = 0;
    dirptr = Directories;
    dirstate = 0;
    while(dirptr < DirsTop) {
        ++nitems;
        if(*dirptr == dir_start_bint) {
            if(dirstate != 0) {
                // SPURIOUS DIRECTORY START, REMOVE
                if(fix) {
                    dirptr[1] = (word_p) zero_bint;
                    rplPurgeForced(dirptr);
                    dirptr -= 2;
                }
            }
            else
                dirstate = 1;   // DIR START FOUND, NO PARENT YET
            dirptr += 2;
            continue;
        }
        if(*dirptr == dir_parent_bint) {
            if(dirstate != 1) {
                // SPURIOUS PARENT DIR RELATIONSHIP - REMOVE
                if(fix) {
                    dirptr[1] = (word_p) zero_bint;
                    rplPurgeForced(dirptr);
                    dirptr -= 2;
                }
            }
            else
                dirstate = 2;   // DIR START FOUND, PARENT REFERENCE FOUND
            dirptr += 2;
            continue;
        }

        if(*dirptr == dir_end_bint) {
            if(dirstate != 2) {
                // SPURIOUS DIRECTORY END??
                if(fix) {
                    dirptr[1] = (word_p) zero_bint;
                    rplPurgeForced(dirptr);
                    dirptr -= 2;
                }
            }
            else
                dirstate = 0;   // DIR END FOUND, PARENT REFERENCE OK
            dirptr += 2;
            continue;
        }

// CHECK NAME
        int32_t badname = 0;

        if(!rplVerifyObjPointer(dirptr[0])) {
            if(!fix)
                return 0;
            badname = 1;
        }
        else {

            if(!rplVerifyObject(dirptr[0])) {
                // BAD NAME, REPLACE WITH A NEW ONE
                if(!fix)
                    return 0;
                badname = 1;
            }
        }

// CHECK CONTENTS

        if(!rplVerifyObjPointer(dirptr[1])) {
            if(!fix)
                return 0;
            // PURGE BAD VARIABLES
            dirptr[1] = (word_p) zero_bint;
            rplPurgeForced(dirptr);
            continue;

        }
        else {
            if(!rplVerifyObject(dirptr[1])) {
                if(!fix)
                    return 0;
                // PURGE BAD VARIABLES
                dirptr[1] = (word_p) zero_bint;
                rplPurgeForced(dirptr);
                continue;
            }
            else {
                // GOOD OBJECT
                if(badname) {
                    // THE NAME GOT CORRUPTED, REPLACE WITH A NEW NAME
                    name = rplCreateHashIdent(nitems);
                    if(!name)
                        return 0;       // PANIC, CAN'T FIX ANYTHING
                    dirptr[0] = name;
                }
            }
        }

// HERE WE HAVE A VALID OBJECT
// IF CONTENTS ARE A DIRECTORY HANDLE, CHECK IF THE HANDLE IS VALID

        if(ISDIR(*dirptr[1])) {
            if(!rplFindDirbyHandle(dirptr[1])) {
                // THIS IS AN INVALID ENTRY TO A NON-EXISTENT OR CORRUPTED DIRECTORY OBJECT
                // JUST PURGE IT
                if(!fix)
                    return 0;
                dirptr[1] = (word_p) zero_bint;        // CHANGE TO A int32_t SO IT CAN BE PURGED
                rplPurgeForced(dirptr);
                continue;
            }
        }

// DONE, THE ENTRY SHOULD BE GOOD HERE

        dirptr += 2;

    }

    if(errors)
        return 0;
    return 1;
}
