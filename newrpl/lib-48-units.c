/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// THIS LIBRARY PROVIDES COMPILATION OF STACK RELATED COMMANDS AND OTHER BASIC COMMANDS


// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  48
#define LIB_ENUM lib480enum
#define LIB_NAMES lib48_names
#define LIB_HANDLER lib48_handler
#define LIB_TOKENINFO lib48_tokeninfo
#define LIB_NUMBEROFCMDS LIB48_NUMBEROFCMDS
#define ROMPTR_TABLE    romptr_table48

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };


// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(UNITDEF,MKTOKENINFO(6,TITYPE_NOTALLOWED,2,2)), \
    CMD(UNITPURGE,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(UVAL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(UBASE,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(CONVERT,MKTOKENINFO(7,TITYPE_NOTALLOWED,2,2))
// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY

#define CMD_EXTRANAME \
    "", \
    "→UNIT"
#define CMD_EXTRAENUM \
    UNITOP, \
    TOUNIT

#define CMD_EXTRAINFO \
    MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,1,2), \
    MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)


// INTERNAL DECLARATIONS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a,b) a
enum LIB_ENUM { CMD_LIST , CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a,b) #a
const char * const LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME };
#undef CMD

// AND INFORMATION FOR THE SYMBOLIC COMPILER
#define CMD(a,b) b
const BINT const LIB_TOKENINFO[]=
{
        CMD_LIST ,
        CMD_EXTRAINFO
};

// LIST OF ALL KNOWN SYSTEM UNIT OBJECTS
ROMOBJECT system_units[]={

    // 'm'
    MKPROLOG(DOIDENT,1),
    TEXT2WORD('m',0,0,0),
    0
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
     0
};


// RETURN A POINTER TO THE END OF THE NEXT UNIT IDENTIFIER
// SEPARATOR SYMBOLS ALLOWED ARE ( ) * / ^

BYTEPTR rplNextUnitToken(BYTEPTR start,BYTEPTR end)
{
    while(start<end) {
        if((*start=='*')||(*start=='/')||(*start=='^')||(*start=='(')||(*start==')')) break;
        start=(BYTEPTR)utf8skip((char *)start,(char *)end);
    }
    return start;
}











void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplPushData(IPtr);
        return;
    }

    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE)
    {
        // THESE ARE OVERLOADABLE COMMANDS DISPATCHED FROM THE
        // OVERLOADABLE OPERATORS LIBRARY.

        int nargs=OVR_GETNARGS(CurOpcode);

        if(rplDepthData()<nargs) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        switch(OPCODE(CurOpcode))
        {
        case OVR_ABS:
        case OVR_NEG:
        {
            if(!ISUNIT(*rplPeekData(1))) {
                rplError(ERR_UNITEXPECTED);
                return;
            }
            // JUST APPLY THE OPERATOR TO THE VALUE
            WORDPTR *stkclean=DSTop;
            rplPushData(rplPeekData(1)+1);
            rplCallOvrOperator(CurOpcode);
            if(Exceptions) { DSTop=stkclean; return; }
            // AND PUT BACK THE SAME UNIT
            BINT nlevels=rplUnitExplode(rplPeekData(2));
            rplUnitPopItem(nlevels);
            if(Exceptions) { DSTop=stkclean; return; }
            WORDPTR newunit=rplUnitAssemble(nlevels);
            if(!newunit) { DSTop=stkclean; return; }

            // FINAL CLEANUP
            rplDropData(nlevels);
            rplOverwriteData(1,newunit);

        return;
        }
        case OVR_MUL:
        {
            BINT nlevels1,nlevels2;
            WORDPTR *stkclean=DSTop;

            nlevels1=rplUnitExplode(rplPeekData(2));
            nlevels2=rplUnitExplode(rplPeekData(1+nlevels1));

            nlevels1=rplUnitSimplify(nlevels1+nlevels2);
            if(Exceptions) { DSTop=stkclean; return; }
            WORDPTR newunit=rplUnitAssemble(nlevels1);
            if(!newunit) { DSTop=stkclean; return; }

            // FINAL CLEANUP
            rplDropData(nlevels1+1);
            rplOverwriteData(1,newunit);
            return;
        }
        }



    }


    switch(OPCODE(CurOpcode))
    {
    case UNITDEF:
    case UNITPURGE:
    case UVAL:
    case UBASE:
    case CONVERT:
    case TOUNIT:
    return;


        // STANDARIZED OPCODES:
        // --------------------
        // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES

    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors
    {

        BYTEPTR ptr=(BYTEPTR)TokenStart;

        if(*ptr=='_') {
        // STARTS WITH THE UNIT, CHECK IF WE ARE IN A UNIT CONSTRUCT

        if(CurrentConstruct==MKPROLOG(LIBRARY_NUMBER,0)) {
            // THE NUMBER WAS COMPILED PROPERLY, NOW ADD THE UNIT ITSELF

            // START COMPILING THE UNIT EXPRESSION

            // ONLY ALLOWS IDENTIFIERS, * AND / OPERATIONS BETWEEN THEM
            // ALSO ALLOWS ^ BUT ONLY WITH REAL EXPONENTS
            // PARENTHESIS ARE SUPPORTED BUT REMOVED AT COMPILE TIME s^2/(Kg*m) --> s^2*Kg^-1*m^-1
            // MAXIMUM 8 LEVELS SUPPORTED

            BYTEPTR nextptr;
            BINT expisreal=0;
            BINT count=0;
            BINT exponent=1,negexp=0,needident=0,needexp=0;
            BINT groupoff[8];
            BINT groupidx=0;
            WORD Locale=rplGetSystemLocale();

            nextptr=ptr+1;

            needident=1;

            while(count<(BINT)TokenLen-1) {
                if(needident) {

                // HANDLE THE SPECIAL CASE OF A PARENTHESIS

                if(*nextptr=='(') {
                    // SET THE EXPONENT FOR ALL IDENTS
                    if(negexp) {
                        exponent=-exponent;
                        negexp=0;
                    }
                    // OPEN A NEW GROUP
                    groupoff[groupidx]=CompileEnd-*(ValidateTop-1); // STORE THE OFFSET OF THE CURRENT OBJECT
                    ++groupidx;
                    if(groupidx>=8) {
                        // NO MORE THAN 8 NESTED LEVELS ALLOWED
                        RetNum=ERR_SYNTAX;
                        return;
                    }
                    ++nextptr;
                    ++count;
                    continue;
                }


                // GET THE NEXT IDENT
                BYTEPTR nameend=rplNextUnitToken(nextptr,(BYTEPTR)BlankStart);

                if(nameend<=nextptr) {
                    RetNum=ERR_SYNTAX;
                    return;
                }

                if(!rplIsValidIdent(nextptr,nameend)) {
                    RetNum=ERR_SYNTAX;
                    return;
                }

                BINT nletters=utf8nlen((char *)nextptr,(char *)nameend);
                // COMPILE THE IDENT

                rplCompileIDENT(DOIDENT,nextptr,nameend);
                if(Exceptions) {
                RetNum=ERR_INVALID;
                return;
                }


                // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count)+(nameend-nextptr);

                count+=nletters;
                needexp=1;
                needident=0;

                }
                else {
                    // NOT LOOKING FOR AN IDENTIFIER
                if(*nextptr==')') {

                    // END OF A GROUP
                    if(!groupidx) {
                        RetNum=ERR_SYNTAX;
                        return;
                    }
                    if(needexp) {
                        BINT finalexp=(negexp)? -exponent:exponent;
                        rplCompileAppend(MAKESINT(finalexp));
                        rplCompileAppend(MAKESINT(1));
                        // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                        nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count);
                        needexp=0;
                    }

                    if(*(nextptr+1)=='^') {
                        // TODO: HANDLE SPECIAL CASE OF A GROUP TO AN EXPONENT

                        nextptr++;
                        count++;

                        // DO THE EXACT SAME THING TO READ THE EXPONENT

                        if(*(nextptr+1)=='(') {
                            // THE EXPONENT IS A FRACTION OR NUMERIC EXPRESSION


                         nextptr+=2;
                         count+=2;

                             BYTEPTR numend=rplNextUnitToken(nextptr,(BYTEPTR)BlankStart);

                             if(numend<=nextptr) {
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             // GET THE NUMERATOR INTO RReg[0]
                             newRealFromText(&RReg[0],nextptr,numend,Locale);

                             if(RReg[0].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                                 // BAD EXPONENT!
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             BINT nletters=utf8nlen((char *)nextptr,(char *)numend);

                             count+=nletters;
                             nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count);

                             // ONLY OPERATOR ALLOWED HERE IS DIVISION

                             if(*nextptr==')') {
                                 // JUST A NUMBER WITHIN PARENTHESIS, SET DENOMINATOR TO 1
                                 rplOneToRReg(1);
                                 nletters=1;
                             }
                             else {

                             if(*nextptr!='/') {
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             ++nextptr;
                             ++count;

                             BYTEPTR numend=rplNextUnitToken(nextptr,(BYTEPTR)BlankStart);

                             if(numend<=nextptr) {
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             // GET THE DENOMINATOR INTO RReg[1]
                             newRealFromText(&RReg[1],nextptr,numend,Locale);

                             if(RReg[1].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                                 // BAD EXPONENT!
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             nletters=utf8nlen((char *)nextptr,(char *)numend);

                             nextptr+=nletters;

                             if(*nextptr!=')') {
                                 RetNum=ERR_SYNTAX;
                                 return;
                             }

                             ++nletters;

                             }



                             // RESTORE POINTERS AND CONTINUE

                             count+=nletters;
                        }
                        else {
                        // ONLY A REAL NUMBER SUPPORTED AS EXPONENT
                        // GET THE NEXT TOKEN
                        nextptr++;
                        BYTEPTR numend=rplNextUnitToken(nextptr,(BYTEPTR)BlankStart);

                        if(numend<=nextptr) {
                            RetNum=ERR_SYNTAX;
                            return;
                        }

                        newRealFromText(&RReg[0],nextptr,numend,Locale);

                        if(RReg[0].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                            // BAD EXPONENT!
                            RetNum=ERR_SYNTAX;
                            return;
                        }


                        rplOneToRReg(1);

                        BINT nletters=utf8nlen((char *)nextptr,(char *)numend);


                        count+=1+nletters;


                        }

                        // HERE WE HAVE NUMERATOR AND DENOMINATOR

                        // KEEP ONLY THE NUMERATOR SIGN
                        RReg[0].flags^=RReg[1].flags&F_NEGATIVE;
                        RReg[1].flags&=~F_NEGATIVE;


                        // CYCLE THROUGH ALL IDENTIFIERS SINCE THE GROUP STARTED
                        // MULTIPLY THEIR EXPONENTS BY THIS ONE

                        WORDPTR groupptr,unitptr,numptr,denptr;
                        BINT groupsize,offset=0;
                        REAL orgnum,orgden;

                        groupptr=*(ValidateTop-1)+groupoff[groupidx-1];

                        groupsize=CompileEnd-groupptr;

                        while(offset<groupsize) {
                            // FIRST THING IS TO RESTORE POSSIBLY MOVED POINTERS
                            groupptr=*(ValidateTop-1)+groupoff[groupidx-1];
                            unitptr=groupptr+offset;
                            numptr=rplSkipOb(unitptr);
                            denptr=rplSkipOb(numptr);


                            // NOW GET THE EXPONENTS OF THE NEXT UNIT
                            rplReadNumberAsReal(numptr,&orgnum);
                            rplReadNumberAsReal(denptr,&orgden);

                            mulReal(&RReg[2],&RReg[0],&orgnum);
                            mulReal(&RReg[3],&RReg[1],&orgden);

                            // AND COMPILE THEM AS NEW

                            BINT unitlen=rplObjSize(unitptr);

                            rplCompileAppendWords(unitlen);     // MAKE A COPY OF THE IDENT
                            if(Exceptions) {
                                RetNum=ERR_INVALID;
                                return;
                            }

                            groupptr=*(ValidateTop-1)+groupoff[groupidx-1];
                            unitptr=groupptr+offset;
                            numptr=rplSkipOb(unitptr);
                            denptr=rplSkipOb(numptr);

                            // MAKE A COPY OF THE IDENTIFIER
                            memmovew(CompileEnd-unitlen,unitptr,unitlen);

                            offset=rplSkipOb(denptr)-groupptr;

                            if(isintegerReal(&RReg[2]) && inBINT64Range(&RReg[2])) {
                                // EXPONENT IS AN INTEGER
                                BINT64 finalexp=getBINT64Real(&RReg[2]);
                                // COMPILE AS A BINT OR A SINT
                                rplCompileBINT(finalexp,DECBINT);
                                if(Exceptions) {
                                RetNum=ERR_INVALID;
                                return;
                                }


                            }
                            else {
                                // EXPONENT WILL HAVE TO BE A REAL
                                rplCompileReal(&RReg[2]);
                                if(Exceptions) {
                                RetNum=ERR_INVALID;
                                return;
                                }

                            }


                            if(isintegerReal(&RReg[3]) && inBINT64Range(&RReg[3])) {
                                // EXPONENT IS AN INTEGER
                                BINT64 finalexp=getBINT64Real(&RReg[3]);

                                // COMPILE AS A BINT OR A SINT
                                rplCompileBINT(finalexp,DECBINT);
                                if(Exceptions) {
                                RetNum=ERR_INVALID;
                                return;
                                }


                            }
                            else {
                                // EXPONENT WILL HAVE TO BE A REAL
                                rplCompileReal(&RReg[3]);
                                if(Exceptions) {
                                RetNum=ERR_INVALID;
                                return;
                                }

                            }



                        } // AND REPEAT FOR ALL IDENTIFIERS

                        // HERE WE HAVE THE ENTIRE GROUP DUPLICATED, WE NEED TO MOVE THE MEMORY

                        groupptr=*(ValidateTop-1)+groupoff[groupidx-1];

                        memmovew(groupptr,groupptr+groupsize,CompileEnd-(groupptr+groupsize));
                        CompileEnd-=groupsize;



                    }


                    needexp=0;
                    --groupidx;
                    ++nextptr;
                    ++count;
                    continue;
                }

                if(*nextptr=='*') {
                    if(needexp) {
                        BINT finalexp=(negexp)? -exponent:exponent;
                        rplCompileAppend(MAKESINT(finalexp));
                        rplCompileAppend(MAKESINT(1));
                        // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                        nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count);
                        needexp=0;
                    }


                    // NOTHING TO DO ON MULTIPLICATION
                   needident=1;
                   ++nextptr;
                    ++count;
                    continue;
                }
                if(*nextptr=='/') {
                    if(needexp) {
                        BINT finalexp=(negexp)? -exponent:exponent;
                        rplCompileAppend(MAKESINT(finalexp));
                        rplCompileAppend(MAKESINT(1));
                        // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                        nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count);
                        needexp=0;
                    }

                    // NEGATE THE EXPONENT FOR THE NEXT IDENT
                    negexp^=1;
                    needident=1;
                    ++nextptr;
                    ++count;
                    continue;
                }

                if(*nextptr=='^') {

                    if(!needexp) {
                        RetNum=ERR_SYNTAX;
                        return;
                    }


                    if(*(nextptr+1)=='(') {
                        // THE EXPONENT IS A FRACTION OR NUMERIC EXPRESSION


                     nextptr+=2;
                     count+=2;

                         BYTEPTR numend=rplNextUnitToken(nextptr,(BYTEPTR)BlankStart);

                         if(numend<=nextptr) {
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         // GET THE NUMERATOR INTO RReg[0]
                         newRealFromText(&RReg[0],nextptr,numend,Locale);

                         if(RReg[0].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                             // BAD EXPONENT!
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         BINT nletters=utf8nlen((char *)nextptr,(char *)numend);

                         count+=nletters;
                         nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count);

                         // ONLY OPERATOR ALLOWED HERE IS DIVISION

                         if(*nextptr==')') {
                             // JUST A NUMBER WITHIN PARENTHESIS, SET DENOMINATOR TO 1
                             rplOneToRReg(1);
                             nletters=1;
                         }
                         else {

                         if(*nextptr!='/') {
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         ++nextptr;
                         ++count;

                         BYTEPTR numend=rplNextUnitToken(nextptr,(BYTEPTR)BlankStart);

                         if(numend<=nextptr) {
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         // GET THE DENOMINATOR INTO RReg[1]
                         newRealFromText(&RReg[1],nextptr,numend,Locale);

                         if(RReg[1].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                             // BAD EXPONENT!
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         nletters=utf8nlen((char *)nextptr,(char *)numend);

                         nextptr+=nletters;

                         if(*nextptr!=')') {
                             RetNum=ERR_SYNTAX;
                             return;
                         }

                         ++nletters;

                         }


                         // HERE WE HAVE NUMERATOR AND DENOMINATOR

                         // KEEP ONLY THE NUMERATOR SIGN
                         RReg[0].flags^=RReg[1].flags&F_NEGATIVE;
                         RReg[1].flags&=~F_NEGATIVE;

                         if(isintegerReal(&RReg[0]) && inBINT64Range(&RReg[0])) {
                             // EXPONENT IS AN INTEGER
                             BINT64 finalexp=getBINT64Real(&RReg[0]);
                             finalexp*=exponent;
                             if(negexp) finalexp=-finalexp;

                             // COMPILE AS A BINT OR A SINT
                             rplCompileBINT(finalexp,DECBINT);
                             if(Exceptions) {
                             RetNum=ERR_INVALID;
                             return;
                             }


                         }
                         else {
                             // EXPONENT WILL HAVE TO BE A REAL
                             BINT sign=(negexp)? -exponent:exponent;

                             if(sign<0) RReg[0].flags^=F_NEGATIVE;

                             rplCompileReal(&RReg[0]);
                             if(Exceptions) {
                             RetNum=ERR_INVALID;
                             return;
                             }

                         }


                         if(isintegerReal(&RReg[1]) && inBINT64Range(&RReg[1])) {
                             // EXPONENT IS AN INTEGER
                             BINT64 finalexp=getBINT64Real(&RReg[1]);

                             // COMPILE AS A BINT OR A SINT
                             rplCompileBINT(finalexp,DECBINT);
                             if(Exceptions) {
                             RetNum=ERR_INVALID;
                             return;
                             }


                         }
                         else {
                             // EXPONENT WILL HAVE TO BE A REAL
                             rplCompileReal(&RReg[1]);
                             if(Exceptions) {
                             RetNum=ERR_INVALID;
                             return;
                             }

                         }


                         // RESTORE POINTERS AND CONTINUE

                         count+=nletters;
                         // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                         nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count);
                         needexp=0;
                         continue;



                    }
                    else {
                    // ONLY A REAL NUMBER SUPPORTED AS EXPONENT
                    // GET THE NEXT TOKEN
                    nextptr++;
                    BYTEPTR numend=rplNextUnitToken(nextptr,(BYTEPTR)BlankStart);

                    if(numend<=nextptr) {
                        RetNum=ERR_SYNTAX;
                        return;
                    }

                    newRealFromText(&RReg[0],nextptr,numend,Locale);

                    if(RReg[0].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                        // BAD EXPONENT!
                        RetNum=ERR_SYNTAX;
                        return;
                    }


                    BINT nletters=utf8nlen((char *)nextptr,(char *)numend);


                    if(isintegerReal(&RReg[0]) && inBINT64Range(&RReg[0])) {
                        // EXPONENT IS AN INTEGER
                        BINT64 finalexp=getBINT64Real(&RReg[0]);
                        finalexp*=exponent;
                        if(negexp) finalexp=-finalexp;

                        // COMPILE AS A BINT OR A SINT
                        rplCompileBINT(finalexp,DECBINT);
                        if(Exceptions) {
                        RetNum=ERR_INVALID;
                        return;
                        }
                        rplCompileAppend(MAKESINT(1));


                    }
                    else {
                        // EXPONENT WILL HAVE TO BE A REAL
                        BINT sign=(negexp)? -exponent:exponent;

                        if(sign<0) RReg[0].flags^=F_NEGATIVE;

                        rplCompileReal(&RReg[0]);
                        if(Exceptions) {
                        RetNum=ERR_INVALID;
                        return;
                        }

                        rplCompileAppend(MAKESINT(1));

                    }

                    count+=1+nletters;
                    // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                    nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count);
                    needexp=0;
                    continue;
                    }

                }

                // AT THIS POINT ANYTHING ELSE IS A SYNTAX ERROR
                RetNum=ERR_SYNTAX;
                return;

                }
            }   // END WHILE

            if(needexp) {
                BINT finalexp=(negexp)? -exponent:exponent;
                rplCompileAppend(MAKESINT(finalexp));
                rplCompileAppend(MAKESINT(1));

            }
            if(needident) {
                RetNum=ERR_SYNTAX;
                return;
            }

            // HERE WE SHOULD HAVE A UNIT OBJECT PROPERLY COMPILED!

            RetNum=OK_ENDCONSTRUCT;
            return;
        }
        // IF  WE ARE NOT IN A UNIT CONSTRUCT, IT'S SYNTAX ERROR
            RetNum=ERR_SYNTAX;
            return;
        }

        // DOESN'T START WITH '_'
        // FIRST LOOK FOR THE PRESENCE OF THE '_' SEPARATOR INSIDE THE TOKEN

        int f;

        for(f=0;f<(int)TokenLen;++f)
        {
            if(*ptr=='_') break;
            ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart);
        }

        if(f==(int)TokenLen) {
            // NOT FOUND, THIS IS NOT A UNIT
            libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
            return;
        }

        // THERE IS A '_', NOW SPLIT THE TOKEN AND START A PROLOG OF A UNIT
        rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,0));

        BlankStart=NextTokenStart=(WORDPTR)utf8nskip((char * )TokenStart,(char *)BlankStart,f);
        RetNum=OK_STARTCONSTRUCT_SPLITTOKEN;
        return;
    }


    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors


        if(ISPROLOG(*DecompileObject)) {


            // DO AN EMBEDDED DECOMPILATION OF THE VALUE OBJECT

            rplDecompile(DecompileObject+1,DECOMP_EMBEDDED | ((CurOpcode==OPCODE_DECOMPEDIT)? DECOMP_EDIT:0));    // RUN EMBEDDED
            if(Exceptions) { RetNum=ERR_INVALID; return; }

            // NOW ADD THE UNIT
            rplDecompAppendChar('_');

            BINT offset=1;
            BINT totalsize=rplObjSize(DecompileObject);
            BINT needmult=0;
            BINT Format=4 | ((CurOpcode==OPCODE_DECOMPEDIT)? FMT_CODE:0);  // SIMPLE FORMAT FOR ALL EXPONENTS, ONLY 4 DECIMAL PLACES IS ENOUGH
            WORD Locale=rplGetSystemLocale();

            offset+=rplObjSize(DecompileObject+1);  // SKIP THE MAIN VALUE

            while(offset<totalsize) {

                // TAKE A LOOK AT THE EXPONENT
                WORDPTR expnum,expden;
                REAL rnum,rden;
                expnum=rplSkipOb(DecompileObject+offset);
                expden=rplSkipOb(expnum);
                rplReadNumberAsReal(expnum,&rnum);
                rplReadNumberAsReal(expden,&rden);

                if(needmult) {
                    // CHECK FOR THE SIGN OF THE EXPONENT, ADD A '*' IF POSITIVE, '/' IF NEGATIVE
                if(rnum.flags&F_NEGATIVE) { rplDecompAppendChar('/'); rnum.flags^=F_NEGATIVE; }
                else rplDecompAppendChar('*');
                }

                // DECOMPILE THE IDENTIFIER

                BYTEPTR ptr=(BYTEPTR)(DecompileObject+offset+OBJSIZE(*(DecompileObject+offset)));
                if(ptr[3]==0)
                    // WE HAVE A NULL-TERMINATED STRING, SO WE CAN USE THE STANDARD FUNCTION
                    rplDecompAppendString((BYTEPTR) (DecompileObject+offset+1));
                else
                    rplDecompAppendString2((BYTEPTR)(DecompileObject+offset+1),OBJSIZE(*(DecompileObject+offset))<<2);

                if(Exceptions) { RetNum=ERR_INVALID; return; }

                // ONLY ADD AN EXPONENT IF IT'S NOT ONE

                if(!((rden.len==1) && (rden.data[0]==1) && (rnum.len==1) && (rnum.data[0]==1))) {
                    rplDecompAppendChar('^');
                    if(!((rden.len==1) && (rden.data[0]==1))) {
                        // THIS IS A FRACTION
                        rplDecompAppendChar('(');

                        // ESTIMATE THE MAXIMUM STRING LENGTH AND RESERVE THE MEMORY

                        BYTEPTR string;

                        BINT len=formatlengthReal(&rnum,Format);

                        // RESERVE THE MEMORY FIRST
                        rplDecompAppendString2(DecompStringEnd,len);

                        // NOW USE IT
                        string=(BYTEPTR)DecompStringEnd;
                        string-=len;

                        if(Exceptions) {
                            RetNum=ERR_INVALID;
                            return;
                        }
                        DecompStringEnd=(WORDPTR) formatReal(&rnum,string,Format,Locale);


                        rplDecompAppendChar('/');


                        len=formatlengthReal(&rden,Format);

                        // RESERVE THE MEMORY FIRST
                        rplDecompAppendString2(DecompStringEnd,len);

                        // NOW USE IT
                        string=(BYTEPTR)DecompStringEnd;
                        string-=len;

                        if(Exceptions) {
                            RetNum=ERR_INVALID;
                            return;
                        }
                        DecompStringEnd=(WORDPTR) formatReal(&rden,string,Format,Locale);

                        rplDecompAppendChar(')');

                    }
                    else {
                        // JUST A NUMBER
                        // ESTIMATE THE MAXIMUM STRING LENGTH AND RESERVE THE MEMORY

                        BYTEPTR string;

                        BINT len=formatlengthReal(&rnum,Format);

                        // RESERVE THE MEMORY FIRST
                        rplDecompAppendString2(DecompStringEnd,len);

                        // NOW USE IT
                        string=(BYTEPTR)DecompStringEnd;
                        string-=len;

                        if(Exceptions) {
                            RetNum=ERR_INVALID;
                            return;
                        }
                        DecompStringEnd=(WORDPTR) formatReal(&rnum,string,Format,Locale);




                    }
                }

                needmult=1;

                // SKIP THE THREE OBJECTS
                offset+=rplObjSize(DecompileObject+offset);
                offset+=rplObjSize(DecompileObject+offset);
                offset+=rplObjSize(DecompileObject+offset);


            }

            // DONE
            RetNum=OK_CONTINUE;
            return;

        }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // Library receives:
        // CurrentConstruct = SET TO THE CURRENT ACTIVE CONSTRUCT TYPE
        // LastCompiledObject = POINTER TO THE LAST OBJECT THAT WAS COMPILED, THAT NEEDS TO BE VERIFIED

        // VALIDATE RETURNS:
        // RetNum =  OK_CONTINUE IF THE OBJECT IS ACCEPTED, ERR_INVALID IF NOT.


        RetNum=OK_CONTINUE;
        return;

    case OPCODE_PROBETOKEN:
        // PROBETOKEN FINDS A VALID WORD AT THE BEGINNING OF THE GIVEN TOKEN AND RETURNS
        // INFORMATION ABOUT IT. THIS OPCODE IS MANDATORY

        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
        libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_GETINFO:
        // MANUALLY RETURN INFO FOR THE UNIT OPERATOR
//        if(OPCODE(*DecompileObject)==OVR_UMINUS) { RetNum=OK_TOKENINFO | MKTOKENINFO(1,TITYPE_PREFIXOP,1,4); return; }
        libGetInfo2(*DecompileObject,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        return;


    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) { RetNum=ERR_INVALID; return; }

        RetNum=OK_CONTINUE;
        return;


    case OPCODE_LIBINSTALL:
        LibraryList=(WORDPTR)libnumberlist;
        RetNum=OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;

    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode)>=MIN_RESERVED_OPCODE) {
        RetNum=ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    rplError(ERR_INVALIDOPCODE);

    return;


}









