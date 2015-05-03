#ifndef UTF8LIB_H
#define UTF8LIB_H

#define CCLASS(a) ((a)&0x3f)
#define NFC_QC(a) ((a)&0x40)
#define NFC_EX(a) ((a)&0x80)

#define MAX_NCHARS       0x3ff
#define NCHARS(value) (((value)>>8)&0x3ff)
#define LONG_NCHARS(value) (((value)>>8)&0xffffff)
#define CCBYTE(value) ((value)&0xff)
#define TOFFSET(value) (((value)>>18)&0x3fff)
#define MK_PACKDATA(nchars,ccbyte,toffset) ( ((toffset)<<18)|(((ccbyte)&0xff))|(((nchars)&0x3ff)<<8))
#define MK_LONGPACKDATA(nchars,ccbyte) ( (((ccbyte)&0xff))|(((nchars)&0xffffff)<<8))

#define SING_OFFSET(val) (((val)&0xFFF))
#define SING_LEN(val) (((val)>>12)&0xfffff)

#define MK_SINGRANGE(start,end,offset) ((((end)-(start)+1)<<12)|((offset)&0xfff))
#define MK_SINGGAP(start,end) MK_SINGRANGE(start,end,0xfff)


#define MAX_UNICODE_CHARACTER_LEN 32





// ********************   USER ACCESSIBLE API FOR UTF-8 STRING MANIPULATION *****************************



// ALL STRINGS ARE PROVIDED AS A POINTER AND A BYTE COUNT
// STRINGS MAY CONTAIN THE NULL CHARACTER AND DON'T NEED TO BE NULL-TERMINATED

// DECODE A SINGLE CODE POINT FROM THE GIVEN STRING TO A 32-BIT INTEGER
int utf82Char(char * ptr,int len);

// SKIP A SINGLE CODE POINT IN A UTF-8 ENCODED STRING
// RETURNS THE INCREASED POINTER, OR NULL AT THE END OF STRING
char *utf8Skip(char *ptr,int len);

// ENCODE A UNICODE CODE POINT INTO A LITTLE ENDIAN SEQUENCE OF 4 BYTES, PACKED IN AN INT
// LSB IS ALWAYS USED, AND CONTAINS THE FIRST BYTE
// HIGHER ORDER BYTES ONLY CONTAIN VALID DATA WHEN NON-ZERO
// RETURNS -1 ON AN INVALID CODEPOINT THAT CANNOT BE ENCODED
unsigned int Char2utf8(unsigned int codepoint);


// ********************   INTERNAL API FOR UNICODE NORMALIZATION  *****************************

// INTERNAL BUFFER USED FOR NORMALIZATION
extern unsigned int unicodeBuffer[MAX_UNICODE_CHARACTER_LEN];

// OPTIMIZED VERSION RETURNS NFC QUICK CHECK, COMBINING CLASS AND COMPOSITION EXCLUSION PROPERTIES
// GIVEN A SINGLE CODE POINT
// EXTRACT THE PROPERTIES WITH THE CCLASS(), NFC_QC() AND NFC_EX() MACROS
unsigned char getCPInfo(unsigned int cp);


// STORE THE DECOMPOSITION OF A SINGLE CODE POINT INTO UP TO 3 CODE POINTS
// *dec1==-1 --> NO DECOMPOSITION AVAILABLE
// *dec2 or *dec3 == -1 --> CODEPOINT NOT USED/NEEDED
void quickDecomp(unsigned int cp,unsigned int *dec1,unsigned int *dec2,unsigned int *dec3);

// FULLY (RECURSIVE) DECOMPOSE AND APPEND CHARACTER TO THE NORMALIZATION BUFFER
// BUFFER POSITION SPECIFIED BY lastchar
// RETURNS THE UPDATED POSITION
int appendDecomp(unsigned int cp,int lastchar);

// SORT COMBINING CHARACTERS BASED ON COMBINING CLASS
// WORKS INSIDE THE BUFFER
// lastch IS THE NUMBER OF CODEPOINTS IN THE BUFFER
void bubbleSort(int lastch);

// FULLY COMPOSES A CHARACTER IN THE BUFFER
// lastch IS THE NUMBER OF CODEPOINTS IN THE BUFFER
int quickCompose(int lastch);


// READ A UNICODE CHARACTER (POSSIBLY MULTIPLE CODEPOINTS), CONVERT TO NFC AND LEAVE AT THE BUFFER.
// RETURNS THE NUMBER OF BYTES CONSUMED FROM STRING.
int utf82NFC(char *string,int len);















#endif // UTF8LIB_H

