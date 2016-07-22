/*
* Copyright (c) 2014-2016, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#ifndef _FSYSTEM_H

#define _FSYSTEM_H

// FILE SYSTEM PUBLIC HEADER FILE
/*!
 * \file fsystem.h
 * \brief FileSystem main header
 */
/*!
 * \defgroup fsystemmainpage FSystem - SD Card Access Library
 *
 * \section overview Overview
 *
 * The FSystem library provides access to the SD card, providing long name
 * support, case sensitivity and full directory support.
 * Using the standard C file I/O routines will automatically use the FSystem
 * library, but it can be used directly to take advantage of its powerful
 * features.
 *
 * For a detailed reference of each function see \ref fsystem.h
 */



// CONSTANTS

// FILE ATTRIBUTES
/*!
 * \brief Read only file attribute
 */
#define FSATTR_RDONLY 1
/*!
 * \brief Hidden file attribute
 */
#define FSATTR_HIDDEN 2
/*!
 * \brief System file attribute
 */
#define FSATTR_SYSTEM 4
/*!
 * \brief Volume file attribute
 */
#define FSATTR_VOLUME 8
/*!
 * \brief Directory file attribute
 */
#define FSATTR_DIR    16
/*!
 * \brief Archive file attribute
 */
#define FSATTR_ARCHIVE 32
/*!
 * \brief Special attribute value for long filename entries
 */
#define FSATTR_LONGNAME 0xf
/*!
 * \brief Long filename attribute mask
 */
#define FSATTR_LONGMASK 0x3f

// OPEN MODES
/*!
 * \brief Read only mode
 *
 * Write operations won't be allowed on this file.
 */
#define FSMODE_READ      0    // READ PERMISSION (DEFAULT)
/*!
 * \brief Write mode
 *
 * Write operations truncate the file after the last written byte.
 */
#define FSMODE_WRITE     2    // WRITE PERMISSION
/*!
 * \brief Append mode
 *
 * Write operations occur only at the end of the file.
 */
#define FSMODE_APPEND    4    // ONLY ALLOW WRITING AFTER EOF
/*!
 * \brief Modify mode
 *
 * Write operations occur anywhere in the file. File preserves its
 * original size.
 */
#define FSMODE_MODIFY    8    // WRITE TO ANY POSITION WITHIN FILE
/*!
 * \brief No-growth mode
 *
 * Write operations that require to increase the file size won't
 * be allowed.
 */
#define FSMODE_NOGROW   16    // DON'T ALLOW FILE TO GROW
/*!
 * \brief Don't create file
 *
 * This flag is for the FSOpen function only. If write permission was
 * requested and the file doesn't exist, don't create a new one.
 */
#define FSMODE_NOCREATE	32    // DON'T CREATE IF FILE DOESN'T EXIST

// SIMILAR TO STANDARD SEEK_XXX CONSTANTS
#define FSSEEK_SET  0
#define FSSEEK_CUR  1
#define FSSEEK_END  2






// ERRORS RETURNED BY USER-LEVEL FUNCTIONS
/*!
 * \brief Error code: Returned by all functions when there are no errors.
 */
#define FS_OK		  1
/*!
 * \brief Error code: Indicates a hardware error or an unknown error.
 */
#define FS_ERROR      0		    // UNKNOWN ERROR (OR FUNCTION DOESN'T CARE)
/*!
 * \brief Error code: Indicates end-of-file was reached
 */
#define FS_EOF	     -1			// END OF FILE
/*!
 * \brief Error code: Indicates an invalid file name was given
 */
#define FS_BADNAME   -2			// INVALID FILE NAME
/*!
 * \brief Error code: Returned when the volume requested doesn't exist
 */
#define FS_BADVOLUME -3			// INVALID DRIVE
/*!
 * \brief Error code: File not found.
 */
#define FS_NOTFOUND  -4			// FILE NOT FOUND
/*!
 * \brief Error code: Write operation failed or not permitted.
 */
#define FS_CANTWRITE -5			// WRITE FAILED
/*!
 * \brief Error code: No card is currently inserted.
 */
#define FS_NOCARD    -6			// NO CARD INSERTED
/*!
 * \brief Error code: User has changed the card without unmounting the volumes.
 */
#define FS_CHANGED   -7 		// CARD HAS CHANGED
/*!
 * \brief Error code: Max. number of open files has been reached.
 */
#define FS_MAXFILES  -8			// MAXIMUM NUMBER OF FILES OPEN WAS EXCEEDED
/*!
 * \brief Error code: The name given correspond to an open directory.
 */
#define FS_OPENDIR   -9			// NAME IS AN ALREADY OPEN DIRECTORY
/*!
 * \brief Error code: The name given corresponds to an open file.
 */
#define FS_OPENFILE  -9			// NAME IS AN ALREADY OPEN FILE
/*!
 * \brief Error code: File/directory is open/referenced/locked.
 */
#define FS_USED      -9			// FILE/DIRECTORY IS BEING USED
/*!
 * \brief Error code: Disk is full.
 */
#define FS_DISKFULL  -10		// DISK IS FULL
/*!
 * \brief Error code: File already exists.
 */
#define FS_EXIST     -11		// FILE ALREADY EXISTS
/*!
 * \brief Error code: Invalid Handle.
 */
#define FS_INVHANDLE -12		// HANDLE IS NOT VALID




// CASE SENSITIVITY MODES
/*!
 * \brief Case sensitivity mode: RAW case sensitive.
 *
 * In this mode, names are case-sensitive:
 * "testfile.dat" and "TestFile.dat" are considered different files.
 * However, the FAT specification indicates that two names that differ
 * only in case cannot coexist in the disk.
 * For example, given a disk with only one file called "TestFile.dat":
 * Trying to open "testfile.dat" will fail (file not found).
 * Trying to create "testfile.dat" will fail (conflicting file name).
 * 
 * 
 */
#define FSCASE_SENS   0 		// CASE SENSITIVE (RAW NAMES)
/*!
 * \brief Case sensitivity mode: HP compatible case sensitive.
 *
 * In this mode, names are case-sensitive:
 * "testfile.dat" and "TestFile.dat" are considered different files.
 * However, the FAT specification indicates that two names that differ
 * only in case cannot coexist in the disk. The file system will add
 * trailing semicolons to the names of conflicting files. Trailing
 * semicolons are ignored in name comparisons and handled transparently.
 * The only consideration that the user must keep in mind is that names
 * having trailing semicolons will be visible on the disk when the disk
 * is either read by another OS, or read in a different case-sensitivity
 * mode.
 * For example, given a disk with only one file called "TestFile.dat":
 * Trying to open "TestFile.dat;" wil open the file "TestFile.dat"
 * Trying to open "TestFile.dat;;;;" will open the file "TestFile.dat"
 * Trying to create "TestFile.dat;" will fail (file already exists)
 * Trying to create "testfile.dat" will create "testfile.dat;"
 * Assuming that "testfile.dat;" exists, trying to open "testfile.dat"
 * will open "testfile.dat;"
 * All directory access functions in this mode will report that the files
 * are "TestFile.dat" and "testfile.dat". All trailing semicolons will be
 * stripped.
 * In case of multiple conflicting files, the system will add multiple
 * trailing semicolons.
 * For example:
 * Create "hello world", OK
 * Create "HELLO world" creates "HELLO world;"
 * Create "HELLO WORLD" creates "HELLO WORLD;;"
 * 
 */
#define FSCASE_SENSHP 1			// CASE SENSITIVE (W/SEMICOLON STRIPPING)
/*!
 * \brief Case sensitivity mode: Case insensitive.
 *
 * In this mode, names are case-insensitive but case-preserving.
 * This mode follows the original FAT specification. The behavior is
 * therefore the usual.
 *
 */
#define FSCASE_INSENS 2			// CASE INSENSITIVE
/*!
 * \brief Case sensitivity mode: HP compatible case sensitive.
 *
 * This mode provides case-sensitivity with trailing semicolon
 * handling similar to the mode described in FSCASE_SENSHP.
 * The only difference is that file names returned by directory
 * functions will be "true" names. Trailing semicolons will not
 * be stripped, and the name returned matches exactly the name
 * stored in the disk.
 * 
 */
#define FSCASE_SENSHPTRUE 3		// CASE SENSITIVE W/SEMICOLONS BUT RETURNS TRUE NAMES

// FILENAME ANALYSIS RESULT FLAGS (FSGetNameType())
/*!
 * \brief Name constant: Name includes volume specification
 *
 * Examples: "C:", ":3:name", "3:\"
 */
#define FSNAME_HASVOL    1		// 1 == Name include drive
/*!
 * \brief Name constant: Name includes path information
 *
 * Examples: "mydir/name", "../mydir/subdir/name"
 *
 */
#define FSNAME_HASPATH   2		// 2 == Name include path
/*!
 * \brief Name constant: Name has an absolute path
 *
 * Examples: "\mydir"
 *
 */
#define FSNAME_ABSPATH   4		// 4 == Path is absolute
/*!
 * \brief Name constant: Name is a directory ended in slash
 *
 * Examples: "\mydir\subdir\", "mydir/subdir/"
 *
 */
#define FSNAME_ENDSLASH  8		// 8 == name ends in slash
/*!
 * \brief Name constant: Drive specification is HP style (:x:)
 *
 * Examples: ":C:name"
 *
 */
#define FSNAME_VOLHP    16		// 16== Drive is HP style (:x:)
/*!
 * \brief Name constant: Name/path does not have a file name
 *
 * Examples: "C:", "C:\mydir\", "\"
 *
 */
#define FSNAME_EMPTY    32		// 32== Name is empty

/*!
 * \brief Name constant: File name is the single dot
 *
 * Examples: "C:\mydir\."
 *
 */
#define FSNAME_DOT     64

/*!
 * \brief Name constant: File name is the double dot
 *
 * Examples: "C:\mydir\.."
 *
 */
#define FSNAME_DOTDOT     128


/*!
 * \brief Name constant: Invalid filename
 *
 * Examples: "C:\mydir\\file", "\dir\c:",
 *
 */
#define FSNAME_INVALID  -1

// BITS 16-31 = DEPTH OF PATH (0=CURRENT DIR OR ROOT)
// NEGATIVE RESULT ==> INVALID FILENAME 


// CASE INSENSITIVITY MACRO
#define __ICASE(a) ( ((a>96)&&(a<123))? (a&0xdf):a)
#define __UPPER(a) ( ((a>96)&&(a<123))? (a&0xdf):a)
#define __LOWER(a) ( ((a>64)&&(a<91))? (a|0x20):a)


// TYPE DEFINITIONS

struct __frag;
typedef struct __frag FS_FRAGMENT;
struct __buffer;
typedef struct __buffer FS_BUFFER;
struct __file;
typedef struct __file FS_FILE;


struct __frag {
unsigned int StartAddr,EndAddr;
FS_FRAGMENT *NextFragment;
};

struct __buffer {
unsigned char *Data;
unsigned int Offset;
int Used;
};


// MAIN FILE STRUCTURE
/*!
 * \brief Main structure FS_FILE
 *
 * This structure encapsulates all the information needed to handle a file.
 * 
 */


struct __file {
/*!
 * File name, including period and file extension, NULL terminated
 */
char *Name;
/*!
 * Volume that contains this file (0-3)
 */
unsigned Volume:8,
/*!
 * Any combination of the FSMODE_XXX constants
 */
          Mode:8,
/*!
 * File Attribute
 */
          Attr:8;
/*!
 * Reserved
 */
unsigned NTRes:8,
/*!
 * Create time fraction, hundredths of second (0-199)
 */
          CrtTmTenth:8,
/*!
 * Last Access Date
 */
		  LastAccDate:16;
/*!
 * Create time and date (DOS format)
 */
unsigned int CreatTimeDate;
/*!
 * Last write time and date (DOS format)
 */
unsigned int WriteTimeDate;
/*!
 * First cluster allocated to the file
 */
int FirstCluster;
/*!
 * File size in bytes
 */
unsigned int FileSize;
/*!
 * Current file offset pointer
 */
unsigned int CurrentOffset;
/*!
 * Offset of file entry within its parent directory
 */
int DirEntryOffset,
/*!
 * Number of directory entries this file uses
 */
    DirEntryNum;
/*!
 * Pointer to parent directory
 */
FS_FILE *Dir;
/*!
 * **Internal**: Cluster chain
 */
FS_FRAGMENT Chain;
/*!
 * **Internal**: Read/Write buffers
 */
FS_BUFFER RdBuffer,WrBuffer;
};



// INITIALIZATION FUNCTIONS

/*!
 * \brief Shutdown file system, close all open files and unmount all volumes.
 *
 * This function may be called when the program exits or when the file system will
 * no longer be used (recomended to preserve batteries). If the program exits
 * normally by exit() or by returning from main(), this function will be called
 * automatically.
 *
 * \return An FS_XXX error code (see error code constants)
 * \sa FSRestart
 */
extern int FSShutdown();

/*!
 * \brief Shutdown and reinitialize the file system.
 *
 * This function forces to close all files, unmount all volumes and remount if a
 * card is inserted. It is usually called whenever a card change is detected.
 * However, if any files are open and the card was changed, data will be lost, so
 * use it with caution.
 *
 * \return The FS_XXX error code (see error code constants)
 * \sa FSShutdown
 */
extern int FSRestart();

/*!
 * \brief Stops the clock to the SD card to preserve power
 *
 * If the file system will not be used for a while, use this function to preserve
 * batteries. The clock needs to be restarted using FSWakeUp.
 *
 * \sa FSWakeUp
 */
extern void FSSleep();

/*!
 * \brief Restarts the clock to the SD card
 *
 * Use this function to recover when FSSleep was used
 *
 * \sa FSSleep
 */
extern void FSWakeUp();

// VOLUME FUNCTIONS

/*!
 * \brief Checks if a specified volume is currently mounted
 *
 * This function verifies that the volume number given as a parameter
 * is a valid mounted volume.
 *
 * \param VolNumber    Volume number (from 0 to 3), being volume zero the
 *                     first existing volume in the card
 * \return TRUE if the volume exists and is mounted, FALSE otherwise
 * \sa FSVolumeInserted
 */
extern int FSVolumeMounted(int VolNumber);


/*!
 * \brief Checks if a specified volume is mounted and inserted
 *
 * This function verifies that the volume number given as a parameter
 * is a valid mounted volume and the SD card that contains the volume
 * is currently inserted.
 *
 * \param VolNumber    Volume number (from 0 to 3), being volume zero the
 *               first existing volume in the card
 * \return One of the following error codes: FS_OK (volume is inserted),
 * FS_NOCARD (no card inserted), FS_CHANGED (user inserted a different card),
 * FS_ERROR (unknown error - hardware/memory)
 * \sa FSVolumeMounted
 */
extern int FSVolumeInserted(int VolNumber);

/*!
 * \brief Set the current working volume
 *
 * When using multiple volumes on a card, set the volume used by default
 * on all operations.
 *
 * \param VolNumber    Volume number (from 0 to 3), being volume zero the
 *               first existing volume in the card
 * \return One of the following error codes: FS_OK (current volume was changed),
 * FS_ERROR (volume doesn't exist or other errors)
 * \sa FSGetCurrentVolume
 */
extern int FSSetCurrentVolume(int VolNumber);

/*!
 * \brief Get the current working volume
 *
 * When using multiple volumes on a card, it returns the volume used by default
 * on all operations.
 *
 * \return Volume number (from 0 to 3), being volume zero the first existing volume
 * in the card
 * \sa FSSetCurrentVolume
 */
extern int FSGetCurrentVolume();

/*!
 * \brief Get the total size of a volume
 *
 * Get volume total size, in 512 byte sectors.
 * 
 * \param Volnumber Number of the volume (0-3)
 * \return Total size of the volume in bytes. If error, one of the FS_XXX constants (<=0).
 * \sa FSGetVolumeFree
 */
extern int FSGetVolumeSize(int Volnumber);
/*!
 * \brief Get the free space on a volume
 *
 * Get volume total free space, in bytes.
 * 
 * \param Volnumber Number of the volume (0-3)
 * \return Free space of the volume in 512 byte sectors, If error, one of the FS_XXX constants (<=0).
 * \sa FSGetVolumeSize
 */
extern int FSGetVolumeFree(int Volnumber);

// DIRECTORY ACCESS FUNCTIONS

/*!
 * \brief Creates a directory
 *
 * Creates a directory. See the documentation on FSGetNameType for details
 * about naming conventions.
 *
 * \param name         Name for the new directory
 * \return An FS_XXX error code (see error code constants)
 * \sa FSChdir FSRmdir FSGetNameType
 */
extern int FSMkdir(char *name);

/*!
 * \brief Delete a directory
 *
 * Delete an empty directory, it will fail if the directory contains any files,
 * or if the directory is the current directory. See the documentation on
 * FSGetNameType for details about naming conventions.
 *
 * \param name         Name for the new directory
 * \return An FS_XXX error code (see error code constants)
 * \sa FSChdir FSMkdir FSGetNameType
 */
extern int FSRmdir(char *name);

/*!
 * \brief Changes current working directory
 *
 * Changes the current directory. If the name includes a volume, it changes
 * the current directory for that volume but it does not affect the current
 * volume. See FSGetNameType for details about naming conventions.
 *
 * \param name         Name of the directory to change to
 * \return An FS_XXX error code (see error code constants)
 * \sa FSMkdir FSRmdir FSGetNameType
 */
extern int FSChdir(char *name);

/*!
 * \brief Returns the current work directory for the specified volume.
 *
 * Allocates a string and returns the current work directory 
 * for the specified volume as a text string. The string won't
 * have an ending slash. Use FSGetCurrentVolume() to get the
 * current directory of the current volume.
 *
 * \param Volume       Number of the volume to use (0-3)
 * \return A string containing the directory, NULL if error.
 * \sa FSGetCurrentVolume FSChdir
 */
extern char *FSGetcwd(int Volume);


/*!
 * \brief Open a directory for entry scanning
 *
 * Opens a directory for read, to be scanned using FSGetNextEntry.
 * Use FSClose to close an open directory. See FSGetNameType for details 
 * about naming conventions. 
 *
 * \param name         Name of the directory
 * \param fileptr      Address of a (FS_FILE *) pointer to be filled with
 *               the address of a newly created FS_FILE structure
 * \return An FS_XXX error code (see error code constants)
 * \sa FSOpen FSClose FSGetNameType FS_FILE
 */
extern int FSOpenDir(char *name,FS_FILE **fileptr);

/*!
 * \brief Get the next entry from a directory
 *
 * This function returns the next valid directory entry at the current
 * offset within the directory. It fills a FS_FILE structure allocated by
 * the user and passed as a parameter. All members in the FS_FILE structure
 * will be filled, including date/time, attributes, file length, etc.
 * The FS_FILE structure contains dynamically allocated members that need 
 * to be freed using FSReleaseEntry. For every call to this function there
 * should be a corresponding call to FSReleaseEntry. Failure to do so will
 * result in memory leaks.
 * This function returns FS_EOF when there are no more entries in the
 * directory. The FS_FILE structure will contain valid data ONLY when the
 * function returns FS_OK.
 *
 * \param entry        Pointer to a FS_FILE structure to fill. The caller
 *               is responsible for allocating the FS_FILE structure.
 * \param dir          Pointer to a FS_FILE structure obtained from FSOpenDir
 * \return An FS_XXX error code (see error code constants)
 * \sa FSOpenDir FSReleaseEntry FS_FILE
 */
extern int FSGetNextEntry(FS_FILE *entry,FS_FILE *dir);


/*!
 * \brief Release dynamically allocated memory on a FS_FILE structure
 *
 * This function releases any memory allocated within the FS_FILE structure.
 * The FS_FILE structure itself is NOT freed. Data in the structure should
 * be considered invalid after calling this function.
 *
 * \param file         Pointer to a FS_FILE structure to release.

 * \sa FSGetNextEntry FS_FILE
 */
extern void FSReleaseEntry(FS_FILE *file);

// FILE ACCESS FUNCTIONS
/*!
 * \brief Create a file
 *
 * This function creates a file with the given attribute.
 * It allocates a FS_FILE structure and passes a pointer using the
 * fileptr parameter. ONLY when the function returns FS_OK, *fileptr
 * contains a valid FS_FILE pointer. If the file already exists, returns
 * an error and the file won't be overwritten. File is created in write
 * mode. See FSGetNameType for details about naming conventions.
 *
 * \param name        Name of the file to create. Paths allowed.
 * \param attr        File attribute
 * \param fileptr     Address of a (FS_FILE *) pointer to be filled.
 *
 * \return An FS_XXX error code (see error code constants)
 * \sa FSOpen FSClose FS_FILE FSGetNameType
 */
extern int FSCreate(char *name,int attr,FS_FILE **fileptr);
/*!
 * \brief Open a file. Create if it doesn't exist.
 *
 * This function opens a file in the given mode (see FSMODE_XXX constants)
 * It allocates a FS_FILE structure and passes a pointer using the
 * fileptr parameter. ONLY when the function returns FS_OK, *fileptr
 * contains a valid FS_FILE pointer. If the file is open for writing and
 * it doesn't exist then it creates the file.
 * If the file is open in FSMODE_WRITE mode, the file is truncated to
 * length zero if it exists. In FSMODE_APPEND, all write operations will
 * be forced at the end of the file. In FSMODE_MODIFY, write operations
 * can happen anywhere on the file. The initial position will be set at
 * the beginning of the file, same as in write mode but the file won't be
 * truncated.
 * See FSGetNameType for details about naming conventions.
 *
 * \param name        Name of the file to create. Paths allowed.
 * \param mode        Read/Write mode (one or more FSMODE_XXX constants)
 * \param fileptr     Address of a (FS_FILE *) pointer to be filled.
 *
 * \return An FS_XXX error code (see error code constants)
 * \sa FSCreate FSClose FSGetNameType FS_FILE FSMODE_XXX constants
 */
extern int FSOpen(char *name, int mode, FS_FILE **fileptr);

/*!
 * \brief Close an open file.
 *
 * Use this function to close a file or directory opened with FSCreate,
 * FSOpen or FSOpenDir. All buffers will be flushed when the file is
 * closed.
 * \c Important \c : If FSClose fails to close a file, data may be lost. The
 * user should check for errors and only consider the file closed if FS_OK is
 * returned. On error condition, the user should fix the error and retry.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen
 *
 * \return An FS_XXX error code (see error code constants)
 *
 * \sa FSCreate FSOpen FSOpenDir FSCloseAndDelete
 */
extern int FSClose(FS_FILE *file);

/*!
 * \brief Close an open file and delete it.
 *
 * Same as FSClose but for temporary files.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen
 *
 * \return An FS_XXX error code (see error code constants)
 *
 * \sa FSCreate FSOpen FSOpenDir FSClose
 */
extern int FSCloseAndDelete(FS_FILE *file);

/*!
 * \brief Move the current position pointer for read/write.
 *
 * Change the current position within the file, to a given offset
 * counting from the specified position (one of the SEEK_XXX
 * constants). Setting a position beyond end-of-file is OK.
 * Negative positions will be set to the beginning of the file.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen
 * \param Offset   Number of bytes to move
 * \param position Starting position (one of the SEEK_XXX constants)
 *
 * \return An FS_XXX error code (see error code constants)
 *
 * \sa FSRead FSWrite FSTell
 */
extern int FSSeek(FS_FILE *file,int Offset,int position);

/*!
 * \brief Get the current position pointer for read/write.
 *
 * Obtain the current position within the file. First byte of the file
 * is offset zero.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen
 *
 * \return Current position.
 *
 * \sa FSRead FSWrite FSSeek
 */
extern int FSTell(FS_FILE *file);



/*!
 * \brief Read data from a file.
 *
 * Read the requested number of bytes into the given buffer,
 * starting at the current position. Position is updated.
 * Trying to read beyond end-of-file will result on a number
 * of bytes read that is lower than the requested number or a
 * zero.
 *
 * \param buffer   Byte-aligned buffer to store requested data
 * \param nbytes   Number of bytes requested
 * \param file     (FS_FILE *) pointer obtained from FSOpen
 *
 * \return Number of bytes read, 0 if error or end-of-file.
 *
 * \sa FSSeek FSWrite
 */
extern int FSRead(unsigned char *buffer,int nbytes,FS_FILE *file);
/*!
 * \brief Write data to a file.
 *
 * Write the requested number of bytes into the given file,
 * starting at the current position. Position is updated.
 * In FSMODE_WRITE mode, file length will be truncated after the written
 * block of data. In FSMODE_APPEND the current position will be moved
 * to the end of the file before writing.
 * If the file was open for read-only, it returns zero.
 * 
 * \param buffer   Byte-aligned buffer with data to write
 * \param nbytes   Number of bytes requested
 * \param file     (FS_FILE *) pointer obtained from FSOpen
 *
 * \return Number of bytes written, 0 if error
 *
 * \sa FSSeek FSRead
 */
extern int FSWrite(unsigned char *buffer,int nbytes,FS_FILE *file);
/*!
 * \brief Detect end-of-file condition.
 *
 * Returns TRUE if end-of-file was reached after last operation,
 * it returns FALSE otherwise.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen
 *
 * \return TRUE if end-of-file, FALSE otherwise
 *
 * \sa FSSeek FSWrite
 */
extern int FSEof(FS_FILE *file);
/*!
 * \brief Delete a file
 *
 * This function deletes a file. See FSGetNameType for details about
 * naming conventions.
 *
 * \param name        Name of the file to delete.
 *
 * \return An FS_XXX error code (see error code constants)
 * \sa FSRename FSGetNameType
 */
extern int FSDelete(char *name);
/*!
 * \brief Rename a file
 *
 * This function moves/renames a file. See FSGetNameType for details
 * about naming conventions. If the target name already exists, returns
 * an error. Files must be in the same volume to be moved.
 *
 * \param name        Name of the file to delete.
 *
 * \return An FS_XXX error code (see error code constants)
 * \sa FSDelete FSGetNameType
 */
extern int FSRename(char *oldname,char *newname);


// NAME PROCESSING FUNCTIONS
/*!
 * \brief Compare two file names.
 *
 * This function compares two names, according to the case sensitivity
 * mode.
 * In FSCASE_SENS mode, it performs a standard case-sensitive comparison.
 * In FSCASE_SENSHP oR FSCASE_SENSHPTRUE mode, it performs a fully
 * case-sensitive comparison, but trailing semicolons are neglected.
 * In FSCASE_INSENS mode, it performs a case-insensitive comparison.
 *
 * \param name1        Name of a file to compare
 * \param name2        Name of a file to compare
 * \param caseflags    One of the FSCASE_XXX constants
 *
 * \return TRUE if names are equal, FALSE otherwise
 * \sa FSStripSemi FSGetNameType FSCASE_XXX constants
 */
extern int FSNameCompare(char *name1,char *name2,int caseflags);

/*!
 * \brief Obtain the file name.
 *
 * This function allocates a string, store a copy of the file name and
 * returns a pointer. The user is responsible for freeing the string.
 * May return NULL if insufficient memory to allocate the string.
 * The user can specify the type of output using FSNAME_XXX constants.
 * If FSNAME_ABSPATH is not specified, the path will be relative to the
 * current directory. If the file is not under the current directory,
 * the path will be absolute.
 *
 * \param file         Pointer to the file's FS_FILE structure
 * \param pathflags    Any combination of FSNAME_XXX constants
 *
 * \return A pointer to the allocated string, NULL if error
 * \sa FSStripSemi FSGetNameType FSNAME_XXX constants
 */
extern char *FSGetFileName(FS_FILE *file,int pathflags);

/*!
 * \brief Get information about a file name.
 *
 * This function analyzes a file name, and return a collection of
 * flags.
 * The file system naming convention is as follows:
 * Volumes (drives): Are specified using either a letter or a port number.
 * The four volumes in a card will be mounted to letters C: D: E: F:, and
 * port numbers 3: 4: 5: 6:. Both DOS syntax (C:) and HP calculator syntax
 * (:3:) are accepted and considered equivalent (3: is the same as :3:)
 * Drive letters are case insensitive (c: and C: are the same).
 * Directories: Both forward slash and back slash are accepted and
 * considered equivalent.
 * Names: File and directory names can have up to 255 characters (including
 * a period and the file extension if any). There's no distinction between
 * long and short names. Valid characters are the same as any FAT file system,
 * with the exception of the semicolon, which receives special treatment
 * when working in HP compatible case-sensitive mode. In such mode, trailing
 * semicolons are ignored. See the FSCASE_XXX constants for more details.
 * Examples of valid filenames:
 * 3:\\mydir\\Hello world.dat
 * :3:/mydir/Hello world.dat
 * :C:mydir\\Hello world.dat
 * C:/mydir\\Hello world.dat
 * ..\\Hello world.dat
 * :c:Hello world.dat
 *
 * \param name         Name to analyze.
 *
 * \return Bits 0-15 contain flags, any combination of FSNAME_XXX constants.
 * Bits 16-30 contain the number of subdirectories in the path if any.
 * If the name is invalid, the function returns a negative number.
 * \sa FSStripSemi FSGetNameType FSCASE_XXX constants FSNAME_XXX constants
 */
extern int FSGetNameType(char *name);

/*!
 * \brief Strip trailing semicolons.
 *
 * This function removes all trailing semicolons in the given string.
 * It modifies the original buffer.
 *
 * \param name        Name of a file to strip
 *
 * \sa FSGetNameType FSCASE_XXX constants
 */
extern void FSStripSemi(char *name);

/*!
 * \brief Get error message.
 *
 * This function returns a pointer to a static string containing
 * an error message.
 *
 * \param errornum    Error number.
 * 
 * \return Pointer to a static string.
 * \sa Error codes (constants)
 */
extern char *FSGetErrorMsg(int errornum);

/*!
 * \brief Change file access mode
 *
 * Change the access mode of a file.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen
 * \param newmode  One or more FSMODE_XXX constants
 *
 * \return An FS_XXX error code (see error code constants)
 * \sa FSOpen FSWrite
 */
extern int FSChMode(FS_FILE *file,int newmode);

/*!
 * \brief Change file attributes
 *
 * Change the attributes of a file. It does not check for
 * validity of the attribute, so any combination can be
 * used. If the attribute FSATTR_RDONLY is set, the file
 * access mode will be forced to FSMODE_READ.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen  or entry
 * obtained from FSGetNextEntry
 * \param newmode  One or more FSATTR_XXX constants
 *
 * \return An FS_XXX error code (see error code constants)
 * \sa FSAttr FSChMode FSOpen FSGetNextEntry
 */
extern int FSChAttr(FS_FILE *file,int newattr);

/*!
 * \brief Get file attributes
 *
 * Return attribute of a file.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen or entry
 * obtained from FSGetNextEntry
 *
 * \return File attributes.
 * \sa FSChAttr FSOpen FSGetNextEntry
 */
extern int FSAttr(FS_FILE *file);

/*!
 * \brief Get file size
 *
 * Return file length in bytes.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen or entry
 * obtained from FSGetNextEntry
 *
 * \return File length in bytes.
 * \sa FSOpen FSGetNextEntry
 */
extern int FSFileLength(FS_FILE *file);

/*!
 * \brief Set the case sensitivity mode.
 *
 * Change the case sensitivity mode for all volumes. See the FSCASE_XXX
 * constants for a detailed explanation of the modes.
 * 
 * \param casemode   New case mode. One of the FSCASE_XXX constants
 * \sa FSCASE_XXX constants
 */
extern void FSSetCaseMode(int casemode);

/*!
 * \brief Get file creation time and date
 *
 * Return time and date in a tm struct allocated by the user.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen or entry
 * obtained from FSGetNextEntry
 * \param timedate Pointer to tm structure to fill in
 *
 * \return Function is void, returns time and date filled in tm structure.
 * \sa FSGetAccessDate FSGetWriteTime
 */
extern void FSGetCreatTime(FS_FILE *file,struct compact_tm *timedate);

/*!
 * \brief Get file write time and date
 *
 * Return time and date in a tm struct allocated by the user. This corresponds
 * to the last time the user opened the file for writing.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen or entry
 * obtained from FSGetNextEntry
 * \param timedate Pointer to tm structure to fill in
 *
 * \return Function is void, returns time and date filled in tm structure.
 * \sa FSGetAccessDate FSGetCreatTime
 */
extern void FSGetWriteTime(FS_FILE *file,struct compact_tm *timedate);


/*!
 * \brief Get file access date (no time)
 *
 * Return time and date in a tm struct allocated by the user. Time fields of
 * the structure will be zero-filled, since only date is returned. The value
 * corresponds to the last time the user opened the file for either read or
 * write.
 *
 * \param file     (FS_FILE *) pointer obtained from FSOpen or entry
 * obtained from FSGetNextEntry
 * \param timedate Pointer to tm structure to fill in
 *
 * \return Function is void, returns time and date filled in tm structure.
 * \sa FSGetCreatTime FSGetWriteTime
 */
extern void FSGetAccessDate(FS_FILE *file,struct compact_tm *timedate);

/*!
 * \brief Get file handle number for given file
 *
 * \param file Pointer to a FS_FILE structure corresponding to an open file.
 *
 * \return Function returns a handle number or FS_INVHANDLE if error.
 * \sa FSGetFileFromHandle
 */
extern int FSGetHandle(FS_FILE *file);

/*!
 * \brief Get a FS_FILE structure that correspond to the given handle.
 *
 * \param handle Handle of a currently open file.
 * \param fileptr Pointer to a (FS_FILE *) that will be filled in with the requested file data.
 *
 * \return Function returns an error code. When the given handle is valid, it returns FS_OK and fills the fileptr
 *         parameter with an open (FS_FILE *).
 * \sa FSGetFileFromHandle
 */
extern int FSGetFileFromHandle(int handle,FS_FILE **fileptr);

extern int FSIsInit();
extern int FSCardInserted();
extern int FSCardIsSDHC();

#endif
