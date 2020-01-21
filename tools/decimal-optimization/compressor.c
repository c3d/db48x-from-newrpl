#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <newrpl.h>
#include <decimal.h>

#define ORG_DIGITS  2016
#define ORG_WORDS  (ORG_DIGITS/8)

#define COMP_DIGITS 520
#define COMP_WORDS  (COMP_DIGITS/8)
#define COMP_WORDS_1 (COMP_WORDS+1)
#define COMP_DIGITS_2 (COMP_DIGITS/2)
#define COMP_DIGITS_2_2 (COMP_DIGITS_2+2)

extern void atan_1_table(int exponent, REAL * real);
extern void atan_2_table(int exponent, REAL * real);
extern void atan_5_table(int exponent, REAL * real);
extern void atanh_1_table(int exponent, REAL * real);
extern void atanh_2_table(int exponent, REAL * real);
extern void atanh_5_table(int exponent, REAL * real);

// FOR Kh TABLE:
//#define ATAN_TABLE cvt_Kh_2016
//#define ORG_ATAN_TABLE Kh_2016
//#define FILE_NAME  "cordic_Kh_8"
//#define ONE_EXTRA_VALUE 0
//#define EXPONENT_FIXED -2015

// FOR K TABLE:
//#define ATAN_TABLE cvt_K_2016
//#define ORG_ATAN_TABLE K_2016
//#define FILE_NAME  "cordic_K_8"
//#define ONE_EXTRA_VALUE 1
//#define EXPONENT_FIXED -2016

// FOR TABLE 1:
//#define ATAN_TABLE cvt_atan_x_2016_1
//#define ORG_ATAN_TABLE atan_1_table
//#define FILE_NAME  "atan_1_8"
//#define ONE_EXTRA_VALUE 0
//#define INDEX_OFFSET    0
// DON'T DEFINE THIS: #define EXPONENT_FIXED -2016

// FOR TABLE 1 hyp:
//#define ATAN_TABLE cvt_atanh_x_2016_1
//#define ORG_ATAN_TABLE atanh_x_2016_1
//#define FILE_NAME  "atanh_1_8"
//#define ONE_EXTRA_VALUE 0
// DON'T DEFINE THIS: #define EXPONENT_FIXED -2016

// FOR TABLE 2:
//#define ATAN_TABLE cvt_atan_x_2016_2
//#define ORG_ATAN_TABLE atan_2_table
//#define FILE_NAME  "atan_2_8"
//#define ONE_EXTRA_VALUE 0
//#define INDEX_OFFSET 1
// DON'T DEFINE THIS: #define EXPONENT_FIXED -2016

// FOR TABLE 2 hyp:
//#define ATAN_TABLE cvt_atanh_x_2016_2
//#define ORG_ATAN_TABLE atanh_x_2016_2
//#define FILE_NAME  "atanh_2_8"
//#define ONE_EXTRA_VALUE 0
// DON'T DEFINE THIS: #define EXPONENT_FIXED -2016

// FOR TABLE 5:
#define ATAN_TABLE cvt_atan_x_2016_5
#define ORG_ATAN_TABLE atan_5_table
#define FILE_NAME  "atan_5_8"
#define ONE_EXTRA_VALUE 0
#define INDEX_OFFSET 1

// FOR TABLE 5 hyp:
//#define ATAN_TABLE cvt_atanh_x_2016_5
//#define ORG_ATAN_TABLE atanh_x_2016_5
//#define FILE_NAME  "atanh_5_8"
//#define ONE_EXTRA_VALUE 0

//extern uint32_t ORG_ATAN_TABLE[];

// NEW CONVERTED TABLE
int32_t ATAN_TABLE[COMP_WORDS_1 * COMP_DIGITS_2_2];

//extern "C" void convert(BINT *dest,BINT *ptr,BINT nwords);

// THIS IS 224 FOR 9-DIGIT VERSION
// CHANGED TO COMP_WORDS FOR 8-DIGIT RADIX VERSION
uint32_t frequency[COMP_WORDS * COMP_DIGITS_2_2];
uint32_t index0[COMP_WORDS * COMP_DIGITS_2_2];
int lastindex;
int numrepeats;

// GET INDEX FOR A GIVEN VALUE
int find(uint32_t value)
{
    int idx;
    for(idx = 0; idx < lastindex; ++idx) {
        if(index0[idx] == value)
            return idx;
    }
    return -1;
}

void sortbyfrequency()
{
    int i, j;
    numrepeats = 0;
    uint32_t value, freqval;
    for(i = 0; i < lastindex; ++i) {
        value = frequency[i];
        for(j = i + 1; j < lastindex; ++j) {
            if(frequency[j] > frequency[i]) {
                value = index0[i];
                freqval = frequency[i];
                index0[i] = index0[j];
                frequency[i] = frequency[j];
                index0[j] = value;
                frequency[j] = freqval;
            }

        }
        if(frequency[i] < 3 && !numrepeats) {
            numrepeats = i;
        }
    }
}

// COMPRESS THE INFORMATION BY USING 16-BIT INDEX INSTEAD OF WORDS

uint16_t comp_offsets[COMP_DIGITS_2_2];
uint8_t compressed_table[COMP_WORDS * COMP_DIGITS_2_2];
int offset;

void compresstable()
{
    // THE EXPONENT IS -COMP_DIGITS-EXPONENT, SO NO NEED TO STORE IT

// FIRST BYTE IS:
// LOW 5-BITS = INDEX 0-31
// HIGH 3-BITS = REPEAT COUNT 0-6
// IF INDEX == 31 THEN 2 FOLLOWING BYTES ARE THE 16-BIT INDEX
// IF REPEAT == 7 THEN FOLLOWING BYTE IS REPEAT (0-255)
    offset = 0;

    int idx, wordidx, wordcount, previdx;
    int word, baseexp;

    baseexp = -COMP_DIGITS;
    for(idx = 0; idx < COMP_DIGITS_2; ++idx) {
        if(ATAN_TABLE[idx * COMP_WORDS_1] != baseexp - idx) {
            printf("Exponent!!\n");
        }
        previdx = -1;
        comp_offsets[idx] = offset;
        wordcount = 0;
        for(word = 0; word < COMP_WORDS; ++word) {
            wordidx = find(ATAN_TABLE[COMP_WORDS_1 * idx + 1 + word]);
            if((wordidx == previdx) || (previdx == -1)) {
                ++wordcount;
                previdx = wordidx;
                continue;
            }
            else {
                // WE HAVE AN INDEX AND A COUNT
                uint8_t byte;
                if(previdx > 63)
                    byte = (previdx & 0x3f) | 0x40;
                else
                    byte = previdx;
                if(wordcount > 1)
                    byte |= 0x80;
                //else byte|=wordcount<<5;

                compressed_table[offset] = byte;
                ++offset;
                if(previdx > 63) {
                    compressed_table[offset] = (previdx >> 6) & 0xff;
                    ++offset;
                }
                if(wordcount > 1) {
                    compressed_table[offset] = wordcount;
                    ++offset;
                }
                previdx = wordidx;
                wordcount = 1;
            }
        }
        // PROCESS THE LAST WORD
        {
            // WE HAVE AN INDEX AND A COUNT
            uint8_t byte;
            if(wordidx > 63)
                byte = (wordidx & 0x3f) | 0x40;
            else
                byte = wordidx;
            if(wordcount > 1)
                byte |= 0x80;
            //else byte|=wordcount<<5;

            compressed_table[offset] = byte;
            ++offset;
            if(wordidx > 63) {
                compressed_table[offset] = (wordidx >> 6) & 0xff;
                ++offset;
            }
            if(wordcount > 1) {
                compressed_table[offset] = wordcount;
                ++offset;
            }
        }

    }

    printf("Size of compressed table=%d bytes\n", offset);
    printf("Size of index = %d bytes\n",
            COMP_DIGITS_2 * sizeof(comp_offsets[0]));
    printf("Size of dictionary=%d bytes\n", lastindex * sizeof(index0[0]));
    printf("-------------------------------\n");
    printf("Total size of table=%d bytes\n",
            offset + COMP_DIGITS_2 * sizeof(comp_offsets[0]) +
            lastindex * sizeof(index0[0]));

}

void compresstable2()
{
    // THE EXPONENT IS -COMP_DIGITS-EXPONENT, SO NO NEED TO STORE IT

// FIRST BYTE IS:
// LOW 5-BITS = INDEX 0-31
// HIGH 3-BITS =
// IF INDEX == 31 THEN 2 FOLLOWING BYTES ARE THE 16-BIT INDEX
// IF REPEAT == 7 THEN FOLLOWING BYTE IS REPEAT (0-255)
    offset = 0;

    int maxidx = 0;
    int idx, wordidx, wordcount, previdx;
    int word, baseexp;
    int offsetrepeat;

    baseexp = -COMP_DIGITS;
    for(idx = 0; idx < COMP_DIGITS_2; ++idx) {
        if(ATAN_TABLE[idx * COMP_WORDS_1] != baseexp - idx) {
            printf("Exponent!!\n");
        }
        previdx = -1;
        comp_offsets[idx] = offset;
        wordcount = 0;
        offsetrepeat = -1;
        for(word = 0; word < COMP_WORDS; ++word) {
            wordidx = find(ATAN_TABLE[COMP_WORDS_1 * idx + 1 + word]);
            if((wordidx == previdx) || (previdx == -1)) {
                ++wordcount;
                previdx = wordidx;
                continue;
            }
            else {

                // WE HAVE AN INDEX AND A COUNT
                uint8_t byte;
                if(frequency[previdx] < 3) {
                    if(offsetrepeat == -1) {
                        offsetrepeat = offset;
                        byte = 0x80 | 1;
                        compressed_table[offset] = byte;
                        ++offset;
                    }
                    else {
                        if(compressed_table[offsetrepeat] == 0xff) {
                            offsetrepeat = offset;
                            byte = 0x80 | 1;
                            compressed_table[offset] = byte;
                            ++offset;
                        }
                        else
                            ++compressed_table[offsetrepeat];
                    }
                    compressed_table[offset] = index0[previdx] & 0xff;
                    ++offset;
                    compressed_table[offset] = (index0[previdx] >> 8) & 0xff;
                    ++offset;
                    compressed_table[offset] = (index0[previdx] >> 16) & 0xff;
                    ++offset;
                    compressed_table[offset] = (index0[previdx] >> 24) & 0xff;
                    ++offset;
                }
                else {
                    offsetrepeat = -1;
                    if(previdx > maxidx)
                        maxidx = previdx;

                    if(previdx > 31)
                        byte = (previdx & 0x1f) | 0x20;
                    else
                        byte = previdx;
                    if(wordcount > 1)
                        byte |= 0x40;
                    //else byte|=wordcount<<5;
                    compressed_table[offset] = byte;
                    ++offset;
                    if(previdx > 31) {
                        compressed_table[offset] = (previdx >> 5) & 0xff;
                        ++offset;
                    }
                    if(wordcount > 1) {
                        compressed_table[offset] = wordcount;
                        ++offset;
                    }
                }
                previdx = wordidx;
                wordcount = 1;
            }
        }
        // PROCESS THE LAST WORD
        {
            // WE HAVE AN INDEX AND A COUNT
            uint8_t byte;
            if(frequency[wordidx] < 3) {
                if(offsetrepeat == -1) {
                    offsetrepeat = offset;
                    byte = 0x80 | 1;
                    compressed_table[offset] = byte;
                    ++offset;
                }
                else {
                    if(compressed_table[offsetrepeat] == 0xff) {
                        offsetrepeat = offset;
                        byte = 0x80 | 1;
                        compressed_table[offset] = byte;
                        ++offset;
                    }
                    else
                        ++compressed_table[offsetrepeat];
                }

                compressed_table[offset] = index0[wordidx] & 0xff;
                ++offset;
                compressed_table[offset] = (index0[wordidx] >> 8) & 0xff;
                ++offset;
                compressed_table[offset] = (index0[wordidx] >> 16) & 0xff;
                ++offset;
                compressed_table[offset] = (index0[wordidx] >> 24) & 0xff;
                ++offset;
            }
            else {
                if(wordidx > maxidx)
                    maxidx = wordidx;

                if(wordidx > 31)
                    byte = (wordidx & 0x1f) | 0x20;
                else
                    byte = wordidx;
                if(wordcount > 1)
                    byte |= 0x40;
                //else byte|=wordcount<<5;
                compressed_table[offset] = byte;
                ++offset;
                if(wordidx > 31) {
                    compressed_table[offset] = (wordidx >> 5) & 0xff;
                    ++offset;
                }
                if(wordcount > 1) {
                    compressed_table[offset] = wordcount;
                    ++offset;
                }
            }
        }

    }

    printf("Size of compressed table=%d bytes\n", offset);
    printf("Size of index = %d bytes\n",
            COMP_DIGITS_2 * sizeof(comp_offsets[0]));
    printf("Size of dictionary=%d bytes\n", maxidx * sizeof(index0[0]));
    printf("-------------------------------\n");
    printf("Total size of table=%d bytes\n",
            offset + COMP_DIGITS_2 * sizeof(comp_offsets[0]) +
            maxidx * sizeof(index0[0]));

}

uint32_t dictionary[COMP_WORDS * COMP_DIGITS_2_2];
int lastdict = 0;

int findindict(uint32_t word)
{
    int k;
    for(k = 0; k < lastdict; ++k) {
        if(dictionary[k] == word)
            return k;
    }
    return -1;
}

int findstreamindict(int startidx, int matchlen)
{
    int idx;

    for(idx = 0; idx < lastdict; ++idx) {
        if(ATAN_TABLE[startidx] == dictionary[idx]) {
            int count;
            int limit = lastdict - idx;
            if(limit > matchlen)
                limit = matchlen;
            for(count = 1; count < limit; ++count) {
                if(ATAN_TABLE[startidx + count] != dictionary[idx + count])
                    break;
            }
            if(count == matchlen)
                return idx;
            if(count == limit) {
                // THE STRING WAS FOUND AT THE END OF DICTIONARY BUT INCOMPLETE, ADD THE REST
                while(count < matchlen) {
                    dictionary[lastdict++] = ATAN_TABLE[startidx + count];
                }
                return idx;
            }
        }
    }

    return -1;
}

// FIND THE BEST REPEATING STREAM IN THIS NUMBER, STARTING WITH THIS WORD
int findbestmatch(int startidx, int endidx, int *matchrepeats)
{
    int idx;
    int matchlen;
    int matchidx;
    int totalsize;
    int maxmatchlen = 0;
    int maxrepeat = 0;
    double maxcompratio = 0.0;
    int maxtotal = 0;
//    printf("idx=%d ******************************************\n",startidx);

    for(idx = endidx - 1; idx > startidx; --idx) {
        if(ATAN_TABLE[idx] == ATAN_TABLE[startidx]) {
            // FOUND INITIAL MATCH, COUNT HOW MANY WORDS MATCH
            matchlen = idx - startidx;
            int idx2 = idx;
            int repeat;
            int count;
            repeat = 0;
            while(idx2 < endidx) {

                for(count = 0; count < matchlen; ++count) {
                    if(idx2 + count >= endidx)
                        break;
                    if(ATAN_TABLE[startidx + count] != ATAN_TABLE[idx2 + count])
                        break;
                }
                // HERE count HAS THE NUMBER OF WORDS THAT MATCHED
                if(count == matchlen) {
                    ++repeat;
                    idx2 += matchlen;
                }
                else
                    break;
            }
            // HERE repeat HAS THE NUMBER OF REPETITIONS OF THIS STREAM

            // ALTERNATIVE ONE: SEEK THE HIGHEST COMPRESSION RATIO OF THE KNOWN WORDS
            /*
               if(findstreamindict(startidx,matchlen)==-1) totalsize=3+matchlen*4;
               else totalsize=3;
               double compratio=((double)repeat+1)*(double)matchlen*4.0/(double)totalsize;
               if(maxcompratio<=compratio) {
               maxmatchlen=matchlen;
               maxrepeat=repeat;
               maxcompratio=compratio;
               }
             */
            // ALTERNATIVE TWO: SEEK TO COMPRESS THE MOST WORDS IN A SINGLE OPERATION (SPEED)
//            printf("alternative = %d words\t\t %d times\n",matchlen,repeat+1);

            if(maxtotal <= repeat * matchlen) {
                maxmatchlen = matchlen;
                maxrepeat = repeat;
                maxtotal = repeat * matchlen;
            }

        }
    }

//    if(maxrepeat) printf("chosen = %d words\t\t %d times\n",maxmatchlen,maxrepeat+1);

    *matchrepeats = maxrepeat;
    return maxmatchlen;
}

int getrepeatcount(int matchoffset, int matchlength, int startidx, int endidx)
{
    int count = 0;
    int repeatnum = 0;
    int offset = startidx;

    while(offset < endidx) {
        count = 0;
        while((count <= matchlength)
                && (ATAN_TABLE[offset + count] ==
                    dictionary[matchoffset + count]))
            ++count;
        if(count == matchlength) {
            ++repeatnum;
            offset += matchlength;
        }
        else
            break;
    }

    return repeatnum;

}

void compresstable3()
{
    // THE EXPONENT IS -COMP_DIGITS-EXPONENT, SO NO NEED TO STORE IT

// USE SEPARATE WORD DICTIONARY WHERE WORDS ARE WORD-ALIGNED, COMMANDS ARE IN BYTES

// LZ4-LIKE FORMAT:

// 1-byte w/LOWER 4-BITS = MATCH LENGTH-1 (1-16 WORDS)
// UPPER 4-BITS = REPEAT COUNT-1 (1-16 TIMES)
// 2-BYTES = OFFSET INTO DICTIONARY
// IF REPEAT COUNT==15 --> MORE REPEAT BYTES FOLLOWS
// IF MATCH LENGTH==15 --> MORE MATCH LENGTH FOLLOWS
    offset = 0;

    int matchlength;
    int matchoffset;
    int matchrepeat;
    int openmatch;
    int openmatchstart;
    uint8_t byte;
    int baseexp;
    int idx;
    int word;
    baseexp = -COMP_DIGITS;
    for(idx = 0; idx < COMP_DIGITS_2 + ONE_EXTRA_VALUE; ++idx) {
#ifdef EXPONENT_FIXED
        if(ATAN_TABLE[idx * COMP_WORDS_1] != EXPONENT_FIXED) {
            printf("Exponent!!\n");
        }
#else
        if(ATAN_TABLE[idx * COMP_WORDS_1] != baseexp - idx) {
            printf("Exponent!!\n");
        }
#endif
        comp_offsets[idx] = offset;
        openmatch = 0;
        for(word = 0; word < COMP_WORDS; ++word) {
            matchlength =
                    findbestmatch(COMP_WORDS_1 * idx + 1 + word,
                    COMP_WORDS_1 * (idx + 1), &matchrepeat);
            if(matchrepeat * matchlength < 1) {
                // ADD WORD TO THE DICTIONARY
                dictionary[lastdict++] =
                        ATAN_TABLE[COMP_WORDS_1 * idx + 1 + word];
                matchlength = 1;
                if(openmatch) {
                    ++openmatch;
                    if(openmatch - 1 > 14)
                        compressed_table[offset - 3] = 0xf;
                    else
                        compressed_table[offset - 3] = openmatch - 1;
                }
                else {
                    matchoffset = lastdict - 1;
                    compressed_table[offset++] = 0;
                    compressed_table[offset++] = matchoffset & 0xff;
                    compressed_table[offset++] = matchoffset >> 8;
                    ++openmatch;
                    openmatchstart = COMP_WORDS_1 * idx + 1 + word;
                }
                continue;
            }

            // WE DID FIND A MATCH

            if(openmatch) {
                // FINALIZE PREVIOUS UNFINISHED MATCH
                int prevoffset = findstreamindict(openmatchstart, openmatch);
                if((prevoffset > 0) && (prevoffset < matchoffset)) {
                    // THIS STREAM WAS ALREADY IN THE DICTIONARY
                    lastdict -= openmatch;      // REMOVE THE SECOND COPY
                    compressed_table[offset - 2] = prevoffset & 0xff;   // AND REUSE THE EXISTING ONE
                    compressed_table[offset - 1] = prevoffset >> 8;
                }

                if(openmatch - 1 > 14) {
                    compressed_table[offset - 3] = 0xf;
                    compressed_table[offset++] = openmatch - 1;
                }
                else
                    compressed_table[offset - 3] = openmatch - 1;
                openmatch = 0;
            }

            matchoffset =
                    findstreamindict(COMP_WORDS_1 * idx + 1 + word,
                    matchlength);

            if(matchoffset == -1) {
                // ADD ENTIRE STREAM TO THE DICTIONARY
                int count;
                matchoffset = lastdict;
                for(count = 0; count < matchlength; ++count)
                    dictionary[lastdict++] =
                            ATAN_TABLE[COMP_WORDS_1 * idx + 1 + word + count];
            }

            // ENCODE IT

            if(matchlength - 1 > 14)
                byte = 15;
            else
                byte = matchlength - 1;
            if(matchrepeat > 14)
                byte |= 0xf0;   // matchrepeat ALREADY CONTAINS THE NUMBER OF COPIES NOT INCLUDING THE ORIGINAL STREAM, SO DON'T SUBTRACT ONE
            else
                byte |= (matchrepeat) << 4;

            compressed_table[offset++] = byte;
            compressed_table[offset++] = matchoffset & 0xff;
            compressed_table[offset++] = matchoffset >> 8;
            if(matchlength - 1 > 14)
                compressed_table[offset++] = matchlength - 1;
            if(matchrepeat > 14)
                compressed_table[offset++] = matchrepeat;

            word += matchlength * (matchrepeat + 1) - 1;

        }

        if(openmatch) {
            // FINALIZE PREVIOUS UNFINISHED MATCH
            int prevoffset = findstreamindict(openmatchstart, openmatch);
            if((prevoffset > 0) && (prevoffset < matchoffset)) {
                // THIS STREAM WAS ALREADY IN THE DICTIONARY
                lastdict -= openmatch;  // REMOVE THE SECOND COPY
                compressed_table[offset - 2] = prevoffset & 0xff;       // AND REUSE THE EXISTING ONE
                compressed_table[offset - 1] = prevoffset >> 8;
            }

            if(openmatch - 1 > 14) {
                compressed_table[offset - 3] = 0xf;
                compressed_table[offset++] = openmatch - 1;
            }
            else
                compressed_table[offset - 3] = openmatch - 1;
        }

    }

    printf("Size of compressed table=%d bytes\n", offset);
    printf("Size of index = %d bytes\n",
            COMP_DIGITS_2 * sizeof(comp_offsets[0]));
    printf("Size of dictionary=%d bytes\n", lastdict * sizeof(dictionary[0]));
    printf("-------------------------------\n");
    printf("Total size of table=%d bytes\n",
            offset + COMP_DIGITS_2 * sizeof(comp_offsets[0]) +
            lastdict * sizeof(dictionary[0]));

}

// DECOMPRESS A REAL NUMBER FROM THE TABLES

uint32_t real[COMP_WORDS_1];

void getreal_table(int exp)
{
    uint8_t *byte = &(compressed_table[comp_offsets[exp]]);

#ifdef EXPONENT_FIXED
    int exponent = EXPONENT_FIXED;
#else
    int exponent = -COMP_DIGITS - exp;
#endif
    int count = 1;
    int _index, repeat;

    real[0] = exponent;

    while(count < COMP_WORDS_1) {
        _index = *byte & 0x3f;
        repeat = *byte & 0xc0;
        ++byte;
        if(repeat & 0x40) {
            _index |= (*byte) << 6;
            ++byte;
        }
        if(repeat & 0x80) {
            repeat = *byte;
            ++byte;
        }
        else
            repeat = 1;
        while(repeat--)
            real[count++] = index0[_index];
    }
}

void getreal_table2(int exp)
{
    uint8_t *byte = &(compressed_table[comp_offsets[exp]]);
    uint32_t word;

    int exponent = -COMP_DIGITS - exp;
    int count = 1;
    int _index, repeat;

    real[0] = exponent;

    while(count < COMP_WORDS_1) {
        if(*byte & 0x80) {
            repeat = *byte & 0x7f;
            ++byte;
            while(repeat--) {
                word = byte[0] | (byte[1] << 8) | (byte[2] << 16) | (byte[3] <<
                        24);
                byte += 4;
                real[count++] = word;
            }
        }
        else {
            _index = *byte & 0x1f;
            repeat = *byte & 0x60;
            ++byte;
            if(repeat & 0x20) {
                _index |= (*byte) << 5;
                ++byte;
            }
            if(repeat & 0x40) {
                repeat = *byte;
                ++byte;
            }
            else
                repeat = 1;
            while(repeat--)
                real[count++] = index0[_index];
        }
    }
}

void getreal_table3(int exp)
{
    uint8_t *byte = &(compressed_table[comp_offsets[exp]]);
    uint32_t word;

#ifdef EXPONENT_FIXED
    int exponent = EXPONENT_FIXED;
#else
    int exponent = -COMP_DIGITS - exp;
#endif
    int count = 1;
    int _index, repeat, len, len2, idx;

    real[0] = exponent;

    // 1-byte w/LOWER 4-BITS = MATCH LENGTH-1 (1-16 WORDS)
    // UPPER 4-BITS = REPEAT COUNT-1 (1-16 TIMES)
    // 2-BYTES = OFFSET INTO DICTIONARY
    // IF REPEAT COUNT==15 --> MORE REPEAT BYTES FOLLOWS
    // IF MATCH LENGTH==15 --> MORE MATCH LENGTH FOLLOWS

    while(count < COMP_WORDS_1) {
        len = *byte;
        ++byte;
        _index = *byte | (byte[1] << 8);
        byte += 2;

        if(len >> 4 == 0xf)
            repeat = *byte++;
        else
            repeat = len >> 4;
        if((len & 0xf) == 0xf)
            len = *byte++;
        else
            len &= 0xf;

        while(repeat-- >= 0) {
            len2 = len;
            idx = _index;
            while(len2-- >= 0)
                real[count++] = dictionary[idx++];
        }
    }
}

BINT real_storage[2 * ORG_WORDS];

int main_compressor()
{
    int idx, word, j;
    int freqidx;
    REAL num;

    // EXTRACT ORIGINAL TABLE FROM COMPRESSED VERSION
    num.data = real_storage;

    for(idx = 0; idx < COMP_DIGITS_2 + ONE_EXTRA_VALUE; ++idx) {
        ORG_ATAN_TABLE(idx + INDEX_OFFSET, &num);

        // TRUNCATE WORDS
        if(COMP_DIGITS < ORG_DIGITS) {
            BINT roundword;
            int skipwords = num.len - COMP_WORDS;
            if(skipwords < 0)
                skipwords = 0;
            if(skipwords) {
                roundword = num.data[skipwords - 1];
                copy_words(num.data, num.data + skipwords, num.len - skipwords);
                num.exp += skipwords * 8;
                num.len -= skipwords;
                if(roundword >= 50000000) {
                    // DO PROPER ROUNDING
                    num.data[0]++;
                    normalize(&num);
                }

            }
        }

        for(j = 0; j < COMP_WORDS_1; ++j)
            ATAN_TABLE[idx * COMP_WORDS_1 + j] = 0;

        copy_words((BINT *) ATAN_TABLE + idx * COMP_WORDS_1 + 1, num.data,
                num.len);
        ATAN_TABLE[idx * COMP_WORDS_1] = num.exp;
    }

    // CLEAR THE FREQUENCY COUNTERS

    for(idx = 0; idx < COMP_WORDS * COMP_DIGITS_2_2; ++idx) {
        index0[idx] = 0;
        frequency[idx] = 0;
    }
    lastindex = 0;
    int baseexp = -COMP_DIGITS;
    for(idx = 0; idx < COMP_DIGITS_2 + ONE_EXTRA_VALUE; ++idx) {
#ifdef EXPONENT_FIXED
        if(ATAN_TABLE[idx * COMP_WORDS_1] != EXPONENT_FIXED) {
            printf("Exponent!!\n");
        }
#else
        if((BINT) (ATAN_TABLE[idx * COMP_WORDS_1]) != baseexp - idx) {
            printf("Exponent!!\n");
        }
#endif
        for(word = 0; word < COMP_WORDS; ++word) {
            freqidx = find(ATAN_TABLE[COMP_WORDS_1 * idx + 1 + word]);
            if(freqidx < 0) {
                index0[lastindex] = ATAN_TABLE[COMP_WORDS_1 * idx + 1 + word];
                freqidx = lastindex;
                ++lastindex;
            }
            ++frequency[freqidx];
        }
    }

    sortbyfrequency();

    printf("Finished analyzing frequencies.\n");
    printf("Total different words = %d\n", lastindex);
    printf("Number of words repeated at least 3 times = %d\n", numrepeats);

    compresstable3();
    clock_t start, end;

    start = clock();
    int nruns;
    printf("Verifying table...\n");

    for(nruns = 0; nruns < 1000; ++nruns) {

        for(idx = 0; idx < COMP_DIGITS_2 + ONE_EXTRA_VALUE; ++idx) {
            getreal_table3(idx);

            if(ATAN_TABLE[idx * COMP_WORDS_1] != real[0]) {
                printf("Error: Exponent!!\n");
            }
            for(word = 0; word < COMP_WORDS; ++word) {
                if(ATAN_TABLE[COMP_WORDS_1 * idx + 1 + word] != real[word + 1]) {
                    printf("Error: idx=%d, word=%d\n", idx, word);
                }
            }

        }
    }
    end = clock();

    printf("Finished checking table in %.10lf\n",
            ((double)end - (double)start) / (double)CLOCKS_PER_SEC);
    printf("Decompression speed = %.1lf MBytes/sec\n",
            ((double)COMP_WORDS_1 * (double)COMP_DIGITS_2 * 4.0 * 1000.0) /
            ((double)end -
                (double)start) * (double)CLOCKS_PER_SEC / 1024.0 / 1024.0);

    char filename[100] = FILE_NAME;

    strcat(filename, "_comp.c");
    FILE *file = fopen(filename, "w");
    if(!file) {
        printf("ERROR creating file\n");
        return 0;
    }

    fprintf(file, "#include <stdint.h>\n");
    fprintf(file, "\n\n// COMPRESSED TABLE FOR %s\n\n", FILE_NAME);

    fprintf(file, "\n\n// DICTIONARY TABLE WITH UNIQUE STREAMS\n\n");
    fprintf(file, "uint32_t %s_dict[%d]= {\n", FILE_NAME, lastdict);

    for(idx = 0; idx < lastdict; ++idx) {
        fprintf(file, "%ld", dictionary[idx]);
        if(idx != lastdict - 1)
            fprintf(file, ", ");
        if(idx % 10 == 9)
            fprintf(file, "\n");
    }
    fprintf(file, "\n};\n\n");

    fprintf(file,
            "\n\n// TABLE WITH OFFSET OF EACH NUMBER WITHIN THE COMPRESSION STREAM\n\n");

    fprintf(file, "uint16_t %s_offsets[%d]= {\n", FILE_NAME,
            COMP_DIGITS_2 + ONE_EXTRA_VALUE);

    for(idx = 0; idx < COMP_DIGITS_2 + ONE_EXTRA_VALUE; ++idx) {
        fprintf(file, "%ld", comp_offsets[idx]);
        if(idx != COMP_DIGITS_2 - 1 + ONE_EXTRA_VALUE)
            fprintf(file, ", ");
        if(idx % 10 == 9)
            fprintf(file, "\n");
    }
    fprintf(file, "\n};\n\n");

    fprintf(file, "\n\n// TABLE WITH COMPRESSION STREAM\n\n");

    fprintf(file, "uint8_t %s_stream[%d]= {\n", FILE_NAME, offset);

    for(idx = 0; idx < offset; ++idx) {
        fprintf(file, "%d", compressed_table[idx]);
        if(idx != offset - 1)
            fprintf(file, ", ");
        if(idx % 10 == 9)
            fprintf(file, "\n");
    }
    fprintf(file, "\n};\n\n");

    fclose(file);
    return 0;
}
