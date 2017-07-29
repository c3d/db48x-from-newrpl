#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <mem.h>
//#include <malloc.h>

#include "utf8lib.h"

// UNICODE DATA FOR NFC NORMALIZATION

int used_CCData;
unsigned int packed_CCData[50000];

int used_CCBytes;
unsigned char packed_CCBytes[50000];

int used_singletonRanges;
unsigned int packed_singletonRanges[50000];

int used_singletonData;
unsigned int packed_singletonData[50000];

int used_doubleRanges;
unsigned int packed_doubleRanges[50000];

int used_doubleData;
unsigned int packed_doubleData[50000];

int used_combiners;
unsigned int packed_combiners[50000];

int used_starteroff;
unsigned int packed_starters[50000];

int used_starterdata;
unsigned int packed_starterData[50000];














unsigned int getHEXValue(char *string,int len)
{
    unsigned int result=0;
    char *end=string+len;
    while(string<end) {
        result<<=4;
        if(*string>='0' && *string<='9') result+=*string-'0';
        else if(*string>='a' && *string<='f') result+=*string-'a'+10;
             else if(*string>='A' && *string<='F') result+=*string-'A'+10;
                  else return result>>4;
        ++string;
    }
    return result;
}
unsigned int skipHEXValue(char *string,int len)
{
    char *end=string+len;
    char *start=string;
    while(string<end) {
        if(*string>='0' && *string<='9') ;
        else if(*string>='a' && *string<='f') ;
             else if(*string>='A' && *string<='F') ;
                  else break;
        ++string;
    }
    return string-start;
}

unsigned int skipRange(char * string,int len)
{
    char *ptr=string+skipHEXValue(string,len);
    if(*ptr=='.') {
        ++ptr;
        if(*ptr=='.') ++ptr;
            else --ptr;
    }

    return ptr-string;
}

unsigned int skipBlanks(char *string,int len)
{
    char *end=string+len;
    char *start=string;
    while(string<end) {
        if( (*string!=' ')&&(*string!='\t')&&(*string!='\n')&&(*string!='\r') ) break;
        ++string;
    }
    return string-start;
}



unsigned int getDECValue(char *string,int len)
{
    unsigned int result=0;
    char *end=string+len;
    while(string<end) {
        if(*string>='0' && *string<='9') result=result*10+(*string-'0');
        else return result;
        ++string;
    }
    return result;
}



int isRange(char *string,int len)
{
    char *end=string+len;

    while(string<end) {
        if(string[0]=='.' && string[1]=='.') return 1;
        ++string;
    }
    return 0;
}

int fieldEquals(char *string,int len,char *compare)
{
    if(!strncmp(string,compare,len)) return 1;
    return 0;
}


int getFieldLength(char *string,int len)
{
    char *end=string+len;
    int count=0;
    while(string<end) {
        if(*string==';') return count;
        ++string;
        ++count;
    }
    return count;
}

int getLineLength(char *string,int len)
{
    char *end=string+len;
    int count=0;
    while(string<end) {
        if(*string=='\n') return count;
        ++string;
        ++count;
    }
    return count;
}

char *database;
int totalsize=0;

void loadDatabase(char *name)
{
    FILE *handle=fopen(name,"rt");
    if(handle==NULL) {
        printf("Unable to load database.\n");
        exit(0);
    }
    fseek(handle,0,SEEK_END);
    int length=ftell(handle);
    fseek(handle,0,SEEK_SET);

    database=malloc(length+0x100);
    if(!database) {
        printf("Not enough memory.\n");
        exit(0);
    }
    totalsize=fread(database,1,length,handle);
    fclose(handle);
}

typedef struct cp_t codepoint;


typedef struct cp_t {
    int code;       // CODE
    int endcode;    // WHEN THIS IS !=0, THIS IS A RANGE OF CODES
    int cc;         // COMBINING CLASS
    int decomp[2];    // DECOMPOSITION
    int nfc_qc;       // NFC QUICK CHECK
    int exclusion;    // DECOMPOSITION EXCLUSION
    codepoint *next;    // LINKED LIST
} codepoint;

codepoint *first_code,*last_code;

codepoint *searchDB(int codept)
{
    codepoint *ptr=first_code;

    while(ptr!=NULL) {
        if(ptr->code==codept) break;
        ptr=ptr->next;
    }
    return ptr;
}



void setCodePointNFC(codepoint *pt)
{
    int k;
    int endcode=pt->endcode;
    codepoint *dbitem;
    if(!endcode) endcode=pt->code;
    for(k=pt->code;k<=endcode;++k)
    {
        // FIND THE CODE IN THE DATABASE
        dbitem=searchDB(k);
        if(!dbitem) {
            printf("ERROR: Item not found in database: %04X\n",k);
        }
        else {
            dbitem->nfc_qc=pt->nfc_qc;
        }
    }
}

void setCodePointExclusion(codepoint *pt)
{
    int k;
    int endcode=pt->endcode;
    codepoint *dbitem;
    if(!endcode) endcode=pt->code;
    for(k=pt->code;k<=endcode;++k)
    {
        // FIND THE CODE IN THE DATABASE
        dbitem=searchDB(k);
        if(!dbitem) {
            printf("ERROR: Item not found in database: %04X\n",k);
        }
        else {
            dbitem->exclusion=1;
        }
    }
}









void processDatabase()
{
    char *end=database+totalsize,*lineend,*columnend;
    char *ptr=database;
    int linelen;
    int columnlen;
    codepoint newcode;

    first_code=last_code=NULL;


    while(ptr<=end) {
        linelen=getLineLength(ptr,end-ptr);
        lineend=ptr+linelen;

        // GET CODE FIELD
            columnlen=getFieldLength(ptr,lineend-ptr);
            columnend=ptr+columnlen;
            if(!columnlen) break;
            ptr+=skipBlanks(ptr,columnend-ptr);
            newcode.code=getHEXValue(ptr,columnend-ptr);
            ptr=columnend+1;
            // SKIP NAME FIELD
            columnlen=getFieldLength(ptr,lineend-ptr);
            ptr+=columnlen+1;

            // SKIP CLASS
            columnlen=getFieldLength(ptr,lineend-ptr);
            ptr+=columnlen+1;

            // GET THE COMBINING CLASS
            columnlen=getFieldLength(ptr,lineend-ptr);
            columnend=ptr+columnlen;
            ptr+=skipBlanks(ptr,columnend-ptr);
            newcode.cc=getDECValue(ptr,columnend-ptr);
            ptr=columnend+1;

            // SKIP DIRECTION
            columnlen=getFieldLength(ptr,lineend-ptr);
            ptr+=columnlen+1;

            // GET DECOMPOSITION CHARACTRERS
            columnlen=getFieldLength(ptr,lineend-ptr);
            columnend=ptr+columnlen;
            int k;
            // RESET TO ZERO
            for(k=0;k<2;++k) newcode.decomp[k]=-1;
            if(*ptr!='<') {
                // DISREGARD TAGGED DECOMPOSITIONS
                // AND READ AS MANY AS POSSIBLE
            k=0;
            while(ptr<columnend) {
                ptr+=skipBlanks(ptr,columnend-ptr);
                newcode.decomp[k]=getHEXValue(ptr,columnend-ptr);
                ptr+=skipHEXValue(ptr,columnend-ptr);
                ++k;
            }
            }
            ptr=columnend+1;



            // ADD HERE MORE COLUMNS, FOR NOW THIS WILL SUFFICE



            // SKIP TO THE NEXT LINE
            ptr=lineend+skipBlanks(lineend,end-lineend);



            // NOW FINISH THE CODEPOINT BY FILLING IN THE EMPTY FIELDS

            newcode.exclusion=0;
            newcode.nfc_qc=0;
            newcode.next=0;
            newcode.endcode=0;


            // APPEND THE NEW CODE TO THE LIST
            codepoint *tmp=malloc(sizeof(codepoint));
            if(!tmp) {
                printf("Out of memory\n");
                exit(0);
            }

            memcpy(tmp,&newcode,sizeof(codepoint));
            if(!first_code) first_code=last_code=tmp;
            else { last_code->next=tmp; last_code=tmp; }


            // DONE, PROCESS THE NEXT


    }

}



void processNFCQC()
{
    char *end=database+totalsize,*lineend,*columnend;
    char *ptr=database;
    int linelen;
    int columnlen;
    codepoint newcode;

    while(ptr<=end) {
        linelen=getLineLength(ptr,end-ptr);
        lineend=ptr+linelen;

        // GET CODE FIELD
            columnlen=getFieldLength(ptr,lineend-ptr);
            columnend=ptr+columnlen;
            if(!columnlen) break;
            ptr+=skipBlanks(ptr,columnend-ptr);

            if(*ptr=='#') {
                // SKIP TO THE NEXT LINE
                ptr=lineend+skipBlanks(lineend,end-lineend);
                continue;
            }

            newcode.code=getHEXValue(ptr,columnend-ptr);

            if(isRange(ptr,columnend-ptr)) {
                ptr+=skipRange(ptr,columnend-ptr);
                newcode.endcode=getHEXValue(ptr,columnend-ptr);
            } else newcode.endcode=0;
            ptr=columnend+1;

            // GET THE PROPERTY NAME
            columnlen=getFieldLength(ptr,lineend-ptr);
            columnend=ptr+columnlen;
            ptr+=skipBlanks(ptr,columnend-ptr);
            if(!fieldEquals(ptr,columnend-ptr,"NFC_QC")) {
                // SKIP TO THE NEXT LINE
                ptr=lineend+skipBlanks(lineend,end-lineend);
                continue;
            }
            ptr=columnend+1;

            // GET QUICK CHECK PROPERTY VALUE
            columnlen=getFieldLength(ptr,lineend-ptr);
            columnend=ptr+columnlen;
            ptr+=skipBlanks(ptr,columnend-ptr);
            if(*ptr=='N') newcode.nfc_qc=1;
            else if(*ptr=='M')
                newcode.nfc_qc=2;
            else newcode.nfc_qc=0;

            ptr=columnend+1;

            // ADD HERE MORE COLUMNS, FOR NOW THIS WILL SUFFICE

            // SKIP TO THE NEXT LINE
            ptr=lineend+skipBlanks(lineend,end-lineend);


            // SET THE NFC PROPERTY ON THE AFFECTED CODEPOINTS
             setCodePointNFC(&newcode);

            // DONE, PROCESS THE NEXT


    }

}

void processExclusions()
{
    char *end=database+totalsize,*lineend,*columnend;
    char *ptr=database;
    int linelen;
    int columnlen;
    codepoint newcode;

    while(ptr<=end) {
        linelen=getLineLength(ptr,end-ptr);
        lineend=ptr+linelen;

        // GET CODE FIELD
            columnlen=getFieldLength(ptr,lineend-ptr);
            columnend=ptr+columnlen;
            if(!columnlen) break;
            ptr+=skipBlanks(ptr,columnend-ptr);

            if(*ptr=='#') {
                // SKIP TO THE NEXT LINE
                ptr=lineend+skipBlanks(lineend,end-lineend);
                continue;
            }

            newcode.code=getHEXValue(ptr,columnend-ptr);
            newcode.endcode=newcode.code;
            ptr=columnend+1;

            // ADD HERE MORE COLUMNS, FOR NOW THIS WILL SUFFICE

            // SKIP TO THE NEXT LINE
            ptr=lineend+skipBlanks(lineend,end-lineend);


            // SET THE NFC PROPERTY ON THE AFFECTED CODEPOINTS
             setCodePointExclusion(&newcode);

            // DONE, PROCESS THE NEXT


    }

    // SCAN THE ENTIRE DATABASE FOR NON-STARTER DECOMPOSITIONS
    codepoint *cp_ptr=first_code;

    while(cp_ptr!=NULL) {

    // EXCLUDE ALL SINGLETONS FROM COMPOSITION
    if((cp_ptr->decomp[0]!=-1)&&(cp_ptr->decomp[1]==-1)) cp_ptr->exclusion=1;

     // EXCLUDE ALL NON-STARTER DECOMPOSITIONS
     if((cp_ptr->cc!=0)&&(cp_ptr->decomp[0]!=-1)) cp_ptr->exclusion=1;

     if(cp_ptr->decomp[0]!=-1) {
         // CHARACTER HAS A DECOMPOSITION WITH A NON-STARTER CHARACTER

         codepoint *other=searchDB(cp_ptr->decomp[0]);
         if(other) {
         if(other->cc!=0) cp_ptr->exclusion=1;
         }

         }
     cp_ptr=cp_ptr->next;
    }




}




/*
int combiningClass(unsigned int cp)
{
    // SLOW VERSION - REPLACE WITH OPTIMIZED VERSION LATER
    codepoint *ptr=first_code;

    while(ptr) {
        if(ptr->code==cp) return ptr->cc;
        ptr=ptr->next;
    }
    return 0;
}

int quickCheckNFC(unsigned int cp)
{
    // SLOW VERSION - REPLACE WITH OPTIMIZED VERSION LATER
    codepoint *ptr=first_code;

    while(ptr) {
        if(ptr->code==cp) return ptr->nfc_qc;
        ptr=ptr->next;
    }
    return 0;
}
*/

char utf8_c1[100];
unsigned int c2[100];
char utf8_c2[100];
char utf8_c3[100];
unsigned int c4[100];
char utf8_c4[100];
char utf8_c5[100];

// APPEND A UNICODE CP TO A NULL-TERMINATED UTF8 STRING
void stringAppendUtf8(char *str,unsigned int cp)
{
    while(*str!=0) ++str;

    unsigned int utf8=Char2utf8(cp);

    if(utf8&0xff) *str++=utf8&0xff;
    if(utf8>>8) *str++=(utf8>>8)&0xff;
    if(utf8>>16) *str++=(utf8>>16)&0xff;
    if(utf8>>24) *str++=(utf8>>24)&0xff;
    *str=0;
    return;
}

void intAppend(unsigned int *str,unsigned int cp)
{
    while(*str!=0) ++str;
    *str++=cp;
    *str=0;
    return;
}

int compareCodes(unsigned int *a,unsigned int *b)
{
    while( (*a!=0)&&(*b!=0)&&(*a==*b)) {++a;++b;}

    if((*a==0) && (*b==0)) return 0;
    return 1;
}

int totalfails=0;


void doNFCTest(int line)
{

    //printf("Line %04d\n: ",line);
    unsigned int tmp[100];
    char *ptr=utf8_c1;
    int len=strlen(ptr);
    char *end=ptr+len;

    // COMPARE C2==NFC(C1)

    tmp[0]=0;
    while(*ptr) {
        ptr+=utf82NFC(ptr,end-ptr);
        int k=0;
        while(unicodeBuffer[k]) intAppend(tmp,unicodeBuffer[k++]);
    }

    if(compareCodes(tmp,c2)) {
        printf("**FAILED c2 == NFC(c1) ***\n");
        ++totalfails; exit(1);
        return;
    }

    ptr=utf8_c2;
    end=ptr+strlen(ptr);

    tmp[0]=0;
    while(*ptr) {
        ptr+=utf82NFC(ptr,end-ptr);
        int k=0;
        while(unicodeBuffer[k]) intAppend(tmp,unicodeBuffer[k++]);
    }

    if(compareCodes(tmp,c2)) {
        printf("**FAILED c2 == NFC(c2) ***\n");
        ++totalfails; exit(1);
        return;
    }

    ptr=utf8_c3;
    end=ptr+strlen(ptr);
    tmp[0]=0;

    while(*ptr) {
        ptr+=utf82NFC(ptr,end-ptr);
        int k=0;
        while(unicodeBuffer[k]) intAppend(tmp,unicodeBuffer[k++]);
    }

    if(compareCodes(tmp,c2)) {
        printf("**FAILED c2 == NFC(c3) ***\n");
        ++totalfails; exit(1);
        return;
    }

    ptr=utf8_c4;
    end=ptr+strlen(ptr);

    tmp[0]=0;
    while(*ptr) {
        ptr+=utf82NFC(ptr,end-ptr);
        int k=0;
        while(unicodeBuffer[k]) intAppend(tmp,unicodeBuffer[k++]);
    }

    if(compareCodes(tmp,c4)) {
        printf("**FAILED c4 == NFC(c4) ***\n");
        ++totalfails; exit(1);
        return;
    }

    ptr=utf8_c5;
    end=ptr+strlen(ptr);

    tmp[0]=0;
    while(*ptr) {
        ptr+=utf82NFC(ptr,end-ptr);
        int k=0;
        while(unicodeBuffer[k]) intAppend(tmp,unicodeBuffer[k++]);
    }

    if(compareCodes(tmp,c4)) {
        printf("**FAILED c4 == NFC(c5) ***\n");
        ++totalfails; exit(1);
        return;
    }

    //printf("PASS\n");



}




void processNormalizationTest()
{
char *end=database+totalsize,*lineend,*columnend;
char *ptr=database;
int linelen;
int columnlen;
unsigned int code;
int count;
int currentline=0;

printf("Testing");

while(ptr<=end) {
    ++currentline;
    if((currentline%100)==0) printf(".");
    linelen=getLineLength(ptr,end-ptr);
    lineend=ptr+linelen;

    // GET C1 FIELD
        columnlen=getFieldLength(ptr,lineend-ptr);
        columnend=ptr+columnlen;
        if(!columnlen) break;
        ptr+=skipBlanks(ptr,columnend-ptr);

        if((*ptr=='#')||(*ptr=='@')) {
            // SKIP TO THE NEXT LINE
            ptr=lineend+skipBlanks(lineend,end-lineend);
            continue;
        }

        utf8_c1[0]=0;
        while(ptr<columnend) {
            ptr+=skipBlanks(ptr,columnend-ptr);
            code=getHEXValue(ptr,columnend-ptr);
            if(code) stringAppendUtf8(utf8_c1,code);
            ptr+=skipHEXValue(ptr,columnend-ptr);
        }

        ptr=columnend+1;

        // GET THE C2 FIELD
        columnlen=getFieldLength(ptr,lineend-ptr);
        columnend=ptr+columnlen;
        utf8_c2[0]=0;
        c2[0]=0;
        while(ptr<columnend) {
            ptr+=skipBlanks(ptr,columnend-ptr);
            code=getHEXValue(ptr,columnend-ptr);
            if(code) { stringAppendUtf8(utf8_c2,code);
                        intAppend(c2,code);
            }
            ptr+=skipHEXValue(ptr,columnend-ptr);
        }

        ptr=columnend+1;

        // GET THE C3 FIELD
        columnlen=getFieldLength(ptr,lineend-ptr);
        columnend=ptr+columnlen;
        utf8_c3[0]=0;
        while(ptr<columnend) {
            ptr+=skipBlanks(ptr,columnend-ptr);
            code=getHEXValue(ptr,columnend-ptr);
            if(code) stringAppendUtf8(utf8_c3,code);
            ptr+=skipHEXValue(ptr,columnend-ptr);
        }

        ptr=columnend+1;

        // GET THE C4 FIELD
        columnlen=getFieldLength(ptr,lineend-ptr);
        columnend=ptr+columnlen;
        utf8_c4[0]=0;
        c4[0]=0;
        while(ptr<columnend) {
            ptr+=skipBlanks(ptr,columnend-ptr);
            code=getHEXValue(ptr,columnend-ptr);
            if(code) { stringAppendUtf8(utf8_c4,code);
                        intAppend(c4,code);
            }
            ptr+=skipHEXValue(ptr,columnend-ptr);
        }

        ptr=columnend+1;

        // GET THE C5 FIELD
        columnlen=getFieldLength(ptr,lineend-ptr);
        columnend=ptr+columnlen;
        utf8_c5[0]=0;
        while(ptr<columnend) {
            ptr+=skipBlanks(ptr,columnend-ptr);
            code=getHEXValue(ptr,columnend-ptr);
            if(code) stringAppendUtf8(utf8_c5,code);
            ptr+=skipHEXValue(ptr,columnend-ptr);
        }

        ptr=columnend+1;

        // ADD HERE MORE COLUMNS, FOR NOW THIS WILL SUFFICE

        // SKIP TO THE NEXT LINE
        ptr=lineend+skipBlanks(lineend,end-lineend);

        // RUN THE TEST

        doNFCTest(currentline);


        // DONE, PROCESS THE NEXT


}

printf("\n");

}




void countCombiningClasses()
{
    codepoint *ptr=first_code;

    int classes[257];
    int n=1;
    int k;

    classes[0]=0;

    while(ptr!=NULL) {
        for(k=0;k<n;++k) {
            if(ptr->cc==classes[k]) break;
        }
        if(k==n) classes[n++]=ptr->cc;
        ptr=ptr->next;
    }

    printf("Total different classes=%d\n",n);
    for(k=0;k<n;++k)
    {
        printf("%d, ",classes[k]);
    }
    printf("\n");
}

#define PACK_THRESHOLD  100



// FIND DUPLICATED BYTES IN THE STREAM
int searchCCBytes(unsigned char *start,int nbytes)
{
    int j,k;

    for(j=0;j<used_CCBytes-nbytes;++j) {
        for(k=0;k<nbytes;++k) if(start[k]!=packed_CCBytes[j+k]) break;
        if(k==nbytes) return j;
    }
    return -1;
}




void packClassData()
{
    codepoint *ptr=first_code;

    int classes[257];
    int n=1;
    int k;

    classes[0]=0;

    while(ptr!=NULL) {
        for(k=0;k<n;++k) {
            if(ptr->cc==classes[k]) break;
        }
        if(k==n) {
            classes[n++]=ptr->cc;
            --k;
            while(k>=0 && classes[k]>classes[k+1]) { int tmp=classes[k+1]; classes[k+1]=classes[k]; classes[k]=tmp; --k; }
        }

        ptr=ptr->next;
    }

    printf("Total different classes=%d\n",n);
    for(k=0;k<n;++k)
    {
        printf("%d, ",classes[k]);
    }
    printf("\n");


    unsigned char packedData[0x110000];
    memset(packedData,0,0x110000);
    int different[256];
    int r=0;
    int j;

    int count=0;
    ptr=first_code;
    int min=0x1000000,max=0;

    while(ptr!=NULL) {
        //if(ptr->code<0x10000) {
            for(j=0;j<n;++j) if(classes[j]==ptr->cc) break;

            int pack=j | ((ptr->nfc_qc)? 0x40:0) | ((ptr->exclusion)? 0x80:0);

            if(pack!=0) {
                if(min>ptr->code) min=ptr->code;
            }
            else {
                if(min<ptr->code) {
                    //printf("Range: %04X..%04X\n",min,ptr->code);
                    min=0x10000000;
                }
            }

            for(j=0;j<r;++j) if(different[j]==pack) break;
            if(j==r) different[r++]=pack;


            packedData[ptr->code]=pack;
            ++count;
        //}
        ptr=ptr->next;
    }

    // BEGIN ANALYSIS OF RANGES
    used_CCData=used_CCBytes=0;

    int countranges=0;
    int tablebytes=0;
    int prevrange=0;
    j=0;
    do {
        r=j+1;
        while((packedData[r]==packedData[j])&&(r<0x110000)) ++r;
        if(r-j>PACK_THRESHOLD) {
            if(j!=prevrange) {
                // THERE'S A GAP OF NON-REPEATED BYTES
                while(j-prevrange>MAX_NCHARS) {

                    int location=searchCCBytes(packedData+prevrange,MAX_NCHARS);
                    printf("Range: %04X..%04X, LEN=%d --> OFFSET=%d\n",prevrange,prevrange+MAX_NCHARS-1,MAX_NCHARS,location<0? used_CCBytes:location);
                    unsigned int data;

                    if(location<0) {
                        // APPEND NEW DATA
                        data=MK_PACKDATA(MAX_NCHARS,0xff,used_CCBytes);
                        int f;
                        for(f=prevrange;f<prevrange+MAX_NCHARS;++f,++used_CCBytes) packed_CCBytes[used_CCBytes]=packedData[f];
                    }
                    else {
                        // DATA IS REPEATED, REUSE
                        data=MK_PACKDATA(MAX_NCHARS,0xff,location);
                    }

                    packed_CCData[used_CCData]=data;
                    ++used_CCData;
                    prevrange+=MAX_NCHARS;
                }


                int location=searchCCBytes(packedData+prevrange,j-prevrange);
                unsigned int data;
                printf("Range: %04X..%04X, LEN=%d --> OFFSET=%d\n",prevrange,j-1,j-prevrange,location<0? used_CCBytes:location);
                if(location<0) {
                data=MK_PACKDATA(j-prevrange,0xff,used_CCBytes);
                int f;
                for(f=prevrange;f<j;++f,++used_CCBytes) packed_CCBytes[used_CCBytes]=packedData[f];
                }
                else {
                    data=MK_PACKDATA(j-prevrange,0xff,location);
                }
                packed_CCData[used_CCData]=data;
                ++used_CCData;

            }
            // ADD THE RANGE WITH REPETITIVE DATA

            printf("Range: %04X..%04X = %02X, LEN=%d\n",j,r-1,packedData[j],r-j);
            packed_CCData[used_CCData]=MK_LONGPACKDATA(r-j,packedData[j]);
            ++used_CCData;
            prevrange=r;
        }
        j=r;
    } while(j<0x110000);



    printf("Total ranges=%d\n",used_CCData);
    printf("Total table bytes=%d\n",used_CCBytes);

    printf("Total characters recorded=%d\n",count);
    printf("Total different data bytes=%d\n",r);
    printf("Min code=%04X\n,max=%04X\n",min,max);
}

#define SING_THRESHOLD 128
#define DBL_THRESHOLD  64
#define COMB_THRESHOLD 128


// PACK AND CREATE TABLES FOR DECOMPOSITION AND COMPOSITION
void packDecompositions()
{
    int singletons=0,doubles=0;
    unsigned int minsingleton=-1,lastsingleton;
    codepoint *ptr=first_code;

    unsigned int singleton_from[16384];
    unsigned int singleton_to[16384];
    unsigned int doubles_from[16384];
    unsigned int doubles_to1[16384];
    unsigned int doubles_to2[16384];
    int used_sing=0,used_doubles=0;

    int min_dbldiff=0,max_dbldiff=0;


    while(ptr!=NULL) {
        if(ptr->decomp[0]!=-1) {
            if(ptr->decomp[1]!=-1) {
                ++doubles;
                doubles_from[used_doubles]=ptr->code;
                doubles_to1[used_doubles]=ptr->decomp[0];
                doubles_to2[used_doubles]=ptr->decomp[1];
                if((int)ptr->decomp[1]-(int)ptr->decomp[0]>max_dbldiff) max_dbldiff=ptr->decomp[1]-ptr->decomp[0];
                if((int)ptr->decomp[1]-(int)ptr->decomp[0]<min_dbldiff) min_dbldiff=(int)ptr->decomp[1]-(int)ptr->decomp[0];

                ++used_doubles;
            }
            else {
                singleton_from[used_sing]=ptr->code;
                singleton_to[used_sing]=ptr->decomp[0];
                ++used_sing;
                ++singletons;
                if(minsingleton==-1) minsingleton=ptr->code;
                lastsingleton=ptr->code;
            }
        }

        ptr=ptr->next;
    }

    printf("Total singletons=%d (from %04X to %04X\n",singletons,minsingleton,lastsingleton);
    printf("Total doubles=%d\n",doubles);
    printf("Max dbldiff=%d\n",max_dbldiff);
    printf("Min dbldiff=%d\n",min_dbldiff);

    // ANALYZE RANGES AND PACK SINGLETONS

    used_singletonData=used_singletonRanges=0;

    int r,j;

    int countrange=0;
    int totalsing=0;
    int prevrange=0;
    j=0;

        // CHECK IF THERE'S A GAP AT THE BEGINNING
    if(singleton_from[j]!=0) {

        // GAP TOO LONG, SEPARATE THE RANGES
        printf("Range %04X..%04X, LEN=%d ***GAP***\n",0,singleton_from[j]-1,singleton_from[j]);
        ++countrange;

        packed_singletonRanges[used_singletonRanges]=MK_SINGGAP(0,singleton_from[j]-1);
        ++used_singletonRanges;

        }




    do {
        r=j+1;
        while((singleton_from[r]==singleton_from[r-1]+1)&&(r<used_sing)) ++r;



        if(r<used_sing) {
            int gapstart=singleton_from[r-1]+1;
            int gapend=singleton_from[r]-1;

            if(gapend-gapstart+1<SING_THRESHOLD) {
                // TOO SMALL, ADD GAP TO TABLE AND KEEP GOING
                j=r;
                continue;

            }
            else {


                printf("Range %04X..%04X, LEN=%d \n",singleton_from[prevrange],singleton_from[r-1],singleton_from[r-1]-singleton_from[prevrange]+1);
                totalsing+=singleton_from[r-1]-singleton_from[prevrange]+1;
                ++countrange;

                packed_singletonRanges[used_singletonRanges]=MK_SINGRANGE(singleton_from[prevrange],singleton_from[r-1],used_singletonData);
                ++used_singletonRanges;
                int f;
                // FIRST FILL THE TABLE WITH -1
                for(f=0;f<singleton_from[r-1]-singleton_from[prevrange]+1;++f) packed_singletonData[used_singletonData+f]=-1;

                // NOW FILL THE CORRESPONDING VALUES
                for(f=0;f<r-prevrange;++f) {
                    packed_singletonData[used_singletonData+singleton_from[prevrange+f]-singleton_from[prevrange]]=singleton_to[prevrange+f];
                }
                used_singletonData+=singleton_from[r-1]-singleton_from[prevrange]+1;


                // GAP TOO LONG, SEPARATE THE RANGES
                printf("Range %04X..%04X, LEN=%d ***GAP***\n",gapstart,gapend,gapend-gapstart+1);
                ++countrange;

                packed_singletonRanges[used_singletonRanges]=MK_SINGGAP(gapstart,gapend);
                ++used_singletonRanges;


            }


        } else {

            printf("Range %04X..%04X, LEN=%d \n",singleton_from[prevrange],singleton_from[r-1],singleton_from[r-1]-singleton_from[prevrange]+1);
            totalsing+=singleton_from[r-1]-singleton_from[prevrange]+1;
            ++countrange;

            packed_singletonRanges[used_singletonRanges]=MK_SINGRANGE(singleton_from[prevrange],singleton_from[r-1],used_singletonData);
            ++used_singletonRanges;
            int f;
            // FIRST FILL THE TABLE WITH -1
            for(f=0;f<singleton_from[r-1]-singleton_from[prevrange]+1;++f) packed_singletonData[used_singletonData+f]=-1;

            // NOW FILL THE CORRESPONDING VALUES
            for(f=0;f<r-prevrange;++f) {
                packed_singletonData[used_singletonData+singleton_from[prevrange+f]-singleton_from[prevrange]]=singleton_to[prevrange+f];
            }
            used_singletonData+=singleton_from[r-1]-singleton_from[prevrange]+1;





            if(singleton_from[r-1]<0x10ffff) {
            printf("Range %04X..%04X, LEN=%d ***GAP***\n",singleton_from[r-1]+1,0x10ffff,0x10ffff-singleton_from[r-1]);
            ++countrange;

            packed_singletonRanges[used_singletonRanges]=MK_SINGGAP(singleton_from[r-1]+1,0x10ffff);
            ++used_singletonRanges;
            }


        }

        prevrange=r;
        j=r;

    } while(j<used_sing);

    printf("Total singletons=%d\n",totalsing);
    printf("Total used data=%d\n",used_singletonData);
    printf("Total used ranges=%d\n",used_singletonRanges);


    // ANALYZE RANGES AND PACK DOUBLES

    used_doubleData=used_doubleRanges=0;


    countrange=0;
    int totaldbl=0;
    prevrange=0;
    j=0;

        // CHECK IF THERE'S A GAP AT THE BEGINNING
    if(doubles_from[j]!=0) {

        // GAP TOO LONG, SEPARATE THE RANGES
        printf("Range %04X..%04X, LEN=%d ***GAP***\n",0,doubles_from[j]-1,doubles_from[j]);
        ++countrange;

        packed_doubleRanges[used_doubleRanges]=MK_SINGGAP(0,doubles_from[j]-1);
        ++used_doubleRanges;

        }




    do {
        r=j+1;
        while((doubles_from[r]==doubles_from[r-1]+1)&&(r<used_doubles)) ++r;



        if(r<used_doubles) {
            int gapstart=doubles_from[r-1]+1;
            int gapend=doubles_from[r]-1;

            if(gapend-gapstart+1<DBL_THRESHOLD) {
                // TOO SMALL, ADD GAP TO TABLE AND KEEP GOING
                j=r;
                continue;

            }
            else {


                printf("Range %04X..%04X, LEN=%d \n",doubles_from[prevrange],doubles_from[r-1],doubles_from[r-1]-doubles_from[prevrange]+1);
                totaldbl+=doubles_from[r-1]-doubles_from[prevrange]+1;
                ++countrange;

                packed_doubleRanges[used_doubleRanges]=MK_SINGRANGE(doubles_from[prevrange],doubles_from[r-1],used_doubleData);
                ++used_doubleRanges;
                int f;
                // FIRST FILL THE TABLE WITH -1
                for(f=0;f<doubles_from[r-1]-doubles_from[prevrange]+1;++f) {
                    packed_doubleData[2*(used_doubleData+f)]=-1;
                    packed_doubleData[2*(used_doubleData+f)+1]=-1;
                }

                // NOW FILL THE CORRESPONDING VALUES
                for(f=0;f<r-prevrange;++f) {
                    packed_doubleData[2*(used_doubleData+(doubles_from[prevrange+f]-doubles_from[prevrange]))]=doubles_to1[prevrange+f];
                    packed_doubleData[2*(used_doubleData+(doubles_from[prevrange+f]-doubles_from[prevrange]))+1]=doubles_to2[prevrange+f];

                }
                used_doubleData+=(doubles_from[r-1]-doubles_from[prevrange]+1);


                // GAP TOO LONG, SEPARATE THE RANGES
                printf("Range %04X..%04X, LEN=%d ***GAP***\n",gapstart,gapend,gapend-gapstart+1);
                ++countrange;

                packed_doubleRanges[used_doubleRanges]=MK_SINGGAP(gapstart,gapend);
                ++used_doubleRanges;


            }


        } else {

            printf("Range %04X..%04X, LEN=%d \n",doubles_from[prevrange],doubles_from[r-1],doubles_from[r-1]-doubles_from[prevrange]+1);
            totaldbl+=doubles_from[r-1]-doubles_from[prevrange]+1;
            ++countrange;

            packed_doubleRanges[used_doubleRanges]=MK_SINGRANGE(doubles_from[prevrange],doubles_from[r-1],used_doubleData);
            ++used_doubleRanges;
            int f;
            // FIRST FILL THE TABLE WITH -1
            for(f=0;f<doubles_from[r-1]-doubles_from[prevrange]+1;++f) {
                packed_doubleData[2*(used_doubleData+f)]=-1;
                packed_doubleData[2*(used_doubleData+f)+1]=-1;
            }

            // NOW FILL THE CORRESPONDING VALUES
            for(f=0;f<r-prevrange;++f) {
                packed_doubleData[2*(used_doubleData+(doubles_from[prevrange+f]-doubles_from[prevrange]))]=doubles_to1[prevrange+f];
                packed_doubleData[2*(used_doubleData+(doubles_from[prevrange+f]-doubles_from[prevrange]))+1]=doubles_to2[prevrange+f];
            }
            used_doubleData+=(doubles_from[r-1]-doubles_from[prevrange]+1);

            if(doubles_from[r-1]<0x10ffff) {
            printf("Range %04X..%04X, LEN=%d ***GAP***\n",doubles_from[r-1]+1,0x10ffff,0x10ffff-doubles_from[r-1]);
            ++countrange;

            packed_doubleRanges[used_doubleRanges]=MK_SINGGAP(doubles_from[r-1]+1,0x10ffff);
            ++used_doubleRanges;
            }

        }

        prevrange=r;
        j=r;

    } while(j<used_doubles);

    printf("Total doubles=%d\n",totaldbl);
    printf("Total used data=%d\n",used_doubleData);
    printf("Total used ranges=%d\n",used_doubleRanges);

    // ANALYZE TABLES FOR COMPOSITION

    // ONLY DOUBLES NEED TO BE IN THE TABLES,
    // AND ONLY DOUBLES THAT ARE NOT IN THE EXCLUSION LIST

    int index_to[16384];
    unsigned int hash[16384];
    int k;
    // SORT TABLE BY DECOMPOSITION FORMS
    for(k=0;k<used_doubles;++k) {
        index_to[k]=k;
        for(j=0;j<k;++j)
            {
            if(doubles_to2[index_to[j]]>doubles_to2[index_to[k]]) {
                int tmp=index_to[k];
                index_to[k]=index_to[j];
                index_to[j]=tmp;
                }
            else {
                if(doubles_to2[index_to[j]]==doubles_to2[index_to[k]]) {
                    if(doubles_to1[index_to[j]]>doubles_to1[index_to[k]]) {
                    int tmp=index_to[k];
                    index_to[k]=index_to[j];
                    index_to[j]=tmp;
                    }

                }

            }
        }
    }

    // REMOVE THE COMPOSITION EXCLUSIONS
    int totalcomp=used_doubles;
    codepoint *cpt;
    for(k=0;k<totalcomp;++k) {
        cpt=searchDB(doubles_from[index_to[k]]);
        if(cpt) {
            if(cpt->exclusion) {
                // REMOVE FROM THE LIST
                for(j=k+1;j<totalcomp;++j)
                {
                    index_to[j-1]=index_to[j];
                }
                --totalcomp;
                --k;
            }

        }

    }

    // COUNT HOW MANY DIFFERENT STARTERS
    int countstarters=0,countcomb=0;
    int combiners[1020],combfreq[1020];
    int minstarter=0xffffff,maxstarter=0;
    int i;

    for(k=0;k<totalcomp;++k)
    {
        for(j=0;j<k;++j) {
            if(doubles_to1[index_to[j]]==doubles_to1[index_to[k]]) break;

        }
        for(i=0;i<countcomb;++i) if(doubles_to2[index_to[k]]==combiners[i]) break;
        if(i==countcomb) {
            combiners[countcomb]=doubles_to2[index_to[k]];
            combfreq[countcomb]=1;
            ++countcomb;
        } else {
            ++combfreq[i];
        }

        if(j==k) {
            ++countstarters;
            if(doubles_to1[index_to[k]]<minstarter) minstarter=doubles_to1[index_to[k]];
            if(doubles_to1[index_to[k]]>maxstarter) maxstarter=doubles_to1[index_to[k]];

        }
    }

    printf("Total composition = %d\n",totalcomp);
    printf("Total different starters=%d\n",countstarters);
    printf("Minimum starter=%04X, max=%04X\n",minstarter,maxstarter);
    printf("Total number of combiners=%d\n",countcomb);
    for(i=0;i<countcomb;++i)
    {
        printf("Comb %04X --> Freq=%d\n",combiners[i],combfreq[i]);
    }

    // PACK COMBINATIONS DATA

    // MAKE TABLE FOR COMBINERS

    used_combiners=used_starterdata=used_starteroff=0;

    countrange=0;
    int totalcomb=0;
    prevrange=0;
    j=0;

        // CHECK IF THERE'S A GAP AT THE BEGINNING
    if(combiners[j]!=0) {

        // GAP TOO LONG, SEPARATE THE RANGES
        printf("Range %04X..%04X, LEN=%d ***GAP***\n",0,combiners[j]-1,combiners[j]);
        ++countrange;

        packed_combiners[used_combiners]=MK_SINGGAP(0,combiners[j]-1);
        ++used_combiners;

        }




    do {
        r=j+1;
        while((combiners[r]==combiners[r-1]+1)&&(r<countcomb)) ++r;

        if(r<countcomb) {
            int gapstart=combiners[r-1]+1;
            int gapend=combiners[r]-1;

            if(gapend-gapstart+1<COMB_THRESHOLD) {
                // TOO SMALL, ADD GAP TO TABLE AND KEEP GOING
                j=r;
                continue;

            }
            else {


                printf("Range %04X..%04X, LEN=%d \n",combiners[prevrange],combiners[r-1],combiners[r-1]-combiners[prevrange]+1);
                totalcomb+=combiners[r-1]-combiners[prevrange]+1;
                ++countrange;

                packed_combiners[used_combiners]=MK_SINGRANGE(combiners[prevrange],combiners[r-1],used_starteroff);
                ++used_combiners;

                int f;
                // FIRST FILL THE TABLE WITH -1
                for(f=0;f<combiners[r-1]-combiners[prevrange]+1;++f) {
                    packed_starters[used_starteroff+f]=-1;
                }

                // NOW FILL THE CORRESPONDING VALUES
                for(f=0;f<r-prevrange;++f) {

                    //   FOR EACH COMBINER, PACK THE DATA FOR THE STARTERS
                    unsigned int *count=packed_starterData+used_starterdata;
                    *count=0;
                    ++used_starterdata;
                    for(k=0;k<totalcomp;++k)
                    {
                        if(doubles_to2[index_to[k]]==combiners[prevrange+f]) {
                            // FOUND A STARTER, ADD TO THE DATA
                            // STORE THE STARTER AND THE COMPOSITE RESULT
                            packed_starterData[used_starterdata++]=doubles_to1[index_to[k]];
                            packed_starterData[used_starterdata++]=doubles_from[index_to[k]];
                            ++*count;
                        }
                    }
                    // STORE THE OFFSET INTO THE DATA
                    packed_starters[used_starteroff+combiners[f+prevrange]-combiners[prevrange]]=count-packed_starterData;

                }
                used_starteroff+=combiners[r-1]-combiners[prevrange]+1;

                // GAP TOO LONG, SEPARATE THE RANGES
                printf("Range %04X..%04X, LEN=%d ***GAP***\n",gapstart,gapend,gapend-gapstart+1);
                ++countrange;

                packed_combiners[used_combiners]=MK_SINGGAP(gapstart,gapend);
                ++used_combiners;


            }

        // ************************ DONE UNTIL HERE
        } else {

            printf("Range %04X..%04X, LEN=%d \n",combiners[prevrange],combiners[r-1],combiners[r-1]-combiners[prevrange]+1);
            totalcomb+=combiners[r-1]-combiners[prevrange]+1;
            ++countrange;

            packed_combiners[used_combiners]=MK_SINGRANGE(combiners[prevrange],combiners[r-1],used_starteroff);
            ++used_combiners;

            int f;
            // FIRST FILL THE TABLE WITH -1
            for(f=0;f<combiners[r-1]-combiners[prevrange]+1;++f) {
                packed_starters[used_starteroff+f]=-1;
            }

            // NOW FILL THE CORRESPONDING VALUES
            for(f=0;f<r-prevrange;++f) {

                //   FOR EACH COMBINER, PACK THE DATA FOR THE STARTERS
                unsigned int *count=packed_starterData+used_starterdata;
                *count=0;
                ++used_starterdata;
                for(k=0;k<totalcomp;++k)
                {
                    if(doubles_to2[index_to[k]]==combiners[prevrange+f]) {
                        // FOUND A STARTER, ADD TO THE DATA
                        // STORE THE STARTER AND THE COMPOSITE RESULT
                        packed_starterData[used_starterdata++]=doubles_to1[index_to[k]];
                        packed_starterData[used_starterdata++]=doubles_from[index_to[k]];
                        ++*count;
                    }
                }
                // STORE THE OFFSET INTO THE DATA
                packed_starters[used_starteroff+combiners[f+prevrange]-combiners[prevrange]]=count-packed_starterData;
            }

            used_starteroff+=combiners[r-1]-combiners[prevrange]+1;


            if(combiners[r-1]<0x10ffff) {
            printf("Range %04X..%04X, LEN=%d ***GAP***\n",combiners[r-1]+1,0x10ffff,0x10ffff-combiners[r-1]);
            ++countrange;

            packed_combiners[used_combiners]=MK_SINGGAP(combiners[r-1]+1,0x10ffff);
            ++used_combiners;
            }

        }

        prevrange=r;
        j=r;

    } while(j<countcomb);

    printf("Total combiner ranges=%d\n",used_combiners);
    printf("Total starter offset data=%d\n",used_starteroff);
    printf("Total used data=%d\n",used_starterdata);




}


void savePackedData(char * filename)
{
    int f;
    int totalbytes;
    FILE *handle=fopen(filename,"wt");
    if(handle==NULL) {
        printf("Unable to create '%s'' file.\n",filename);
        exit(0);
    }

    fprintf(handle,"// *****************************************************************************\n");
    fprintf(handle,"// ************ THIS FILE WAS GENERATED AUTOMATICALLY FROM UNICODE *************\n");
    fprintf(handle,"// ************ DATA TABLES PER UNICODE 7.0 -- DO NOT MODIFY --    *************\n");
    fprintf(handle,"// *****************************************************************************\n");


    totalbytes=0;
    // SAVE PACKED_CCDATA

    fprintf(handle,"\n\n\n");
    fprintf(handle,"const int used_CCData=%d;\n",used_CCData);
    totalbytes+=4;
    fprintf(handle,"const unsigned int const packed_CCData[]= { ");

    for(f=0;f<used_CCData;++f) {
        if(f%10==0) fprintf(handle,"\n");
        fprintf(handle,"0x%X",packed_CCData[f]);
        totalbytes+=4;
        if(f!=used_CCData-1) fprintf(handle,",");
        }
    fprintf(handle,"\n};");

    // SAVE PACKED_CCBYTES

    fprintf(handle,"\n\n\n");
    fprintf(handle,"const int used_CCBytes=%d;\n",used_CCBytes);
    totalbytes+=4;
    fprintf(handle,"const unsigned char const packed_CCBytes[]= { ");

    for(f=0;f<used_CCBytes;++f) {
        if(f%20==0) fprintf(handle,"\n");
        fprintf(handle,"%d",packed_CCBytes[f]);
        totalbytes+=1;
        if(f!=used_CCBytes-1) fprintf(handle,",");
        }
    fprintf(handle,"\n};");

    // SAVE PACKED_SINGLETONRANGES

    fprintf(handle,"\n\n\n");
    fprintf(handle,"const int used_singletonRanges=%d;\n",used_singletonRanges);
    totalbytes+=4;
    fprintf(handle,"const unsigned int const packed_singletonRanges[]= { ");

    for(f=0;f<used_singletonRanges;++f) {
        if(f%10==0) fprintf(handle,"\n");
        fprintf(handle,"0x%X",packed_singletonRanges[f]);
        totalbytes+=4;
        if(f!=used_singletonRanges-1) fprintf(handle,",");
        }
    fprintf(handle,"\n};");


    // SAVE PACKED_SINGLETONDATA

    fprintf(handle,"\n\n\n");
    fprintf(handle,"const int used_singletonData=%d;\n",used_singletonData);
    totalbytes+=4;
    fprintf(handle,"const unsigned int const packed_singletonData[]= { ");

    for(f=0;f<used_singletonData;++f) {
        if(f%10==0) fprintf(handle,"\n");
        fprintf(handle,"0x%X",packed_singletonData[f]);
        totalbytes+=4;
        if(f!=used_singletonData-1) fprintf(handle,",");
        }
    fprintf(handle,"\n};");


    // SAVE PACKED_DOUBLESRANGES

    fprintf(handle,"\n\n\n");
    fprintf(handle,"const int used_doubleRanges=%d;\n",used_doubleRanges);
    totalbytes+=4;
    fprintf(handle,"const unsigned int const packed_doubleRanges[]= { ");

    for(f=0;f<used_doubleRanges;++f) {
        if(f%10==0) fprintf(handle,"\n");
        fprintf(handle,"0x%X",packed_doubleRanges[f]);
        totalbytes+=4;

        if(f!=used_doubleRanges-1) fprintf(handle,",");
        }
    fprintf(handle,"\n};");


    // SAVE PACKED_DOUBLEDATA

    fprintf(handle,"\n\n\n");
    fprintf(handle,"const int used_doubleData=%d;\n",used_doubleData);
    totalbytes+=4;

    fprintf(handle,"const unsigned int const packed_doubleData[]= { ");

    for(f=0;f<2*used_doubleData;++f) {
        if(f%10==0) fprintf(handle,"\n");
        fprintf(handle,"0x%X",packed_doubleData[f]);
        totalbytes+=4;
        if(f!=used_doubleData-1) fprintf(handle,",");
        }
    fprintf(handle,"\n};");


    // SAVE PACKED COMBINERS


    fprintf(handle,"\n\n\n");
    fprintf(handle,"const int used_combiners=%d;\n",used_combiners);
    totalbytes+=4;

    fprintf(handle,"const unsigned int const packed_combiners[]= { ");

    for(f=0;f<used_combiners;++f) {
        if(f%10==0) fprintf(handle,"\n");
        fprintf(handle,"0x%X",packed_combiners[f]);
        totalbytes+=4;
        if(f!=used_combiners-1) fprintf(handle,",");
        }
    fprintf(handle,"\n};");


    // SAVE PACKED STARTERS

    fprintf(handle,"\n\n\n");
    fprintf(handle,"const int used_starteroff=%d;\n",used_starteroff);
    totalbytes+=4;

    fprintf(handle,"const unsigned int const packed_starters[]= { ");

    for(f=0;f<used_starteroff;++f) {
        if(f%10==0) fprintf(handle,"\n");
        fprintf(handle,"0x%X",packed_starters[f]);
        totalbytes+=4;
        if(f!=used_starteroff-1) fprintf(handle,",");
        }
    fprintf(handle,"\n};");


    // SAVE PACKED STARTER DATA


    fprintf(handle,"\n\n\n");
    fprintf(handle,"const int used_starterdata=%d;\n",used_starterdata);
    totalbytes+=4;

    fprintf(handle,"const unsigned int const packed_starterData[]= { ");

    for(f=0;f<used_starterdata;++f) {
        if(f%10==0) fprintf(handle,"\n");
        fprintf(handle,"0x%X",packed_starterData[f]);
        totalbytes+=4;
        if(f!=used_starterdata-1) fprintf(handle,",");
        }
    fprintf(handle,"\n};");




    printf("Saved tables to '%s' file. Total space = %d bytes\n",filename,totalbytes);

    fclose(handle);

}



int main(void)
{
    printf("Unicode data extractor.\n");

    loadDatabase("UnicodeData.txt");

    processDatabase();

    free(database);

    // LOAD THE NFC QUICK CHECK PROPERTY

    loadDatabase("DerivedNormalizationProps.txt");

    processNFCQC();

    // LOAD THE COMP. EXCLUSIONS

    free(database);

    loadDatabase("CompositionExclusions.txt");

    processExclusions();

    free(database);



    countCombiningClasses();

    packClassData();

    packDecompositions();

    savePackedData("utf8data.c");


    loadDatabase("NormalizationTest.txt");

    processNormalizationTest();

    printf("Total failures = %d\n",totalfails);


    return 0;
}

