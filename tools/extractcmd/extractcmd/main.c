/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>

FILE *fopenindir(char *dirpath, char *filename, char *mode)
{
    int lenpath = strlen(dirpath);
    char *name = malloc(lenpath + strlen(filename) + 100);
    if(!name)
        return NULL;
    strcpy(name, dirpath);
    if(name[lenpath - 1] != '/')
        name[lenpath++] = '/';
    strcpy(name + lenpath, filename);
    FILE *retvalue = fopen(name, mode);

    free(name);

    return retvalue;

}

int main(int argc, char *argv[])
{

    char *mainbuffer;

    if(argc < 2) {
        printf("NewRPL command extract - Version 1.0\n");
        printf("Usage: extractcmd <path-to-sources> [-d <output-path>]\n");
        printf("\nOptions:\n");
        printf("\t\t-d <output-path>\tSpecify the directory where the output file will be created (by default, same as the source file)\n\n\n");
        return 0;
    }

    int argidx = 1;
    int needoutputpath = 0;
    int needcleanup = 0;
    int nameoffset, namelen;
    char *outputfile = NULL;
    char *outputpath = NULL;
    char *inputpath = NULL;
    while(argidx < argc) {
        if(needoutputpath) {
            outputpath = argv[argidx];
            needoutputpath = 0;
        }
        else if((argv[argidx][0] == '-') && (argv[argidx][1] == 'd')) {
            if(argv[argidx][2] == 0)
                needoutputpath = 1;
            else
                outputpath = argv[argidx] + 2;
        }
        else
            inputpath = argv[argidx];

        ++argidx;
    }

    // HERE WE HAVE ALL ARGUMENTS PROCESSED
    if(!inputpath) {
        fprintf(stderr, "Error: No path to sources given\n");
        return 1;
    }

    DIR *dirhandle;
    struct dirent *entry;

    dirhandle = opendir(inputpath);
    if(!dirhandle) {
        fprintf(stderr, "Error: Can't find source directory '%s'\n", inputpath);
        return 1;
    }

    while((entry = readdir(dirhandle))) {
        if(!strncmp(entry->d_name, "lib-", 4)) {

            // CREATE AN OUTPUT FILE NAME FROM THE INPUT FILE
            char *end = entry->d_name + strlen(entry->d_name) - 1;
            char *libname = entry->d_name;

            while((end > entry->d_name) && (*end != '.') && (*end != '/')
                    && (*end != '\\'))
                --end;
            if(end <= entry->d_name)
                end = entry->d_name + strlen(entry->d_name);
            else if(*end != '.') {
                libname = end;
                end = entry->d_name + strlen(entry->d_name);
            }
            needcleanup++;

            while(strrchr(libname, '-') > libname)
                libname = strrchr(libname, '-') + 1;

            outputfile =
                    malloc(end - libname + 10 +
                    ((outputpath) ? strlen(outputpath) : 0));
            if(!outputfile) {
                fprintf(stderr, "error: Memory allocation error\n");
                closedir(dirhandle);
                return 1;
            }
            nameoffset = 0;
            namelen = end - libname;
            if(outputpath) {
                strcpy(outputfile, outputpath);
                nameoffset = strlen(outputpath);
                if(outputfile[nameoffset - 1] != '/')
                    outputfile[nameoffset++] = '/';
            }
            memmove(outputfile + nameoffset, libname, namelen);
            strcpy(outputfile + nameoffset + namelen, ".txt");

            // HERE WE HAVE THE SECTION NAME AT outputfile[nameoffset]..outputfile[nameoffset+namelen-1]

            // READ THE INPUT FILE INTO A BUFFER
            FILE *f = fopenindir(inputpath, entry->d_name, "rb");
            if(f == NULL) {
                fprintf(stderr, "error: File not found %s\n", inputpath);
                if(needcleanup)
                    free(outputfile);
                closedir(dirhandle);

                return 1;
            }
            fseek(f, 0, SEEK_END);
            long long length = ftell(f);
            fseek(f, 0, SEEK_SET);

            mainbuffer = malloc(length);
            if(!mainbuffer) {
                fprintf(stderr, "error: Memory allocation error\n");
                if(needcleanup)
                    free(outputfile);
                closedir(dirhandle);

                return 1;
            }
            if(fread(mainbuffer, 1, length, f) != (size_t)length) {
                fprintf(stderr, "error: Can't read from input file\n");
                if(needcleanup)
                    free(outputfile);
                free(mainbuffer);
                closedir(dirhandle);
                return 1;
            }
            fclose(f);

            // HERE WE HAVE THE MAIN FILE LOADED, EXTRACT THE COMMANDS

            // FIND COMMANDS

            char *chunk = mainbuffer;
            int ncmd = 0, extcommand;
            char *cmdname[1000], *cmdstring[1000], *cmddesc[1000];
            int cmdnamelen[1000], cmdstrlen[1000], cmddesclen[1000],
                    cmdflags[1000];
            int line = 1;
            char *libtitle = 0;
            int libtitlelen = 0;
            int currentcmd = 0;

            int k;
            // SET ALL LENGTHS TO ZERO
            for(k = 0; k < 1000; ++k) {
                cmdnamelen[k] = cmdstrlen[k] = cmddesclen[k] = cmdflags[k] = 0;
            }

            while(chunk - mainbuffer < length) {

                // SKIP ANY TABS OR SPACES
                while((chunk - mainbuffer < length) && ((*chunk == ' ')
                            || (*chunk == '\t')))
                    ++chunk;

                if((chunk - mainbuffer < length - 9)
                        && !strncmp(chunk, "//@TITLE=", 9)) {
                    // FOUND TITLE OF THE LIBRARY
                    libtitle = chunk + 9;
                    while((*chunk != '\n') && (*chunk != '\r')
                            && (chunk - mainbuffer < length))
                        ++chunk;
                    libtitlelen = chunk - libtitle;
                }

                // KEEP TRACK OF CURRENT COMMAND DEFINITION
                if((chunk - mainbuffer < length - 4)
                        && !strncmp(chunk, "case", 4)) {
                    // FOUND COMMAND DEFINITION
                    chunk += 4;
                    // SKIP ANY TABS OR SPACES
                    while((chunk - mainbuffer < length) && ((*chunk == ' ')
                                || (*chunk == '\t')))
                        ++chunk;
                    char *findcmd = chunk;
                    int findcmdlen;
                    // SKIP UNTIL THE COLON IS FOUND
                    while((*chunk != ':') && (*chunk != '\n')
                            && (chunk - mainbuffer < length))
                        ++chunk;
                    if(*chunk == ':') {
                        // FOUND A COMMAND
                        findcmdlen = chunk - findcmd;
                        for(k = 0; k < ncmd; ++k) {
                            if((cmdnamelen[k] == findcmdlen)
                                    && !strncmp(cmdname[k], findcmd,
                                        findcmdlen)) {
                                currentcmd = k;
                                break;
                            }
                        }
                    }

                }

                if((chunk - mainbuffer < length - 6)
                        && !strncmp(chunk, "//@NEW", 6)) {
                    // CURRENT COMMAND IS NEW ON NEWRPL
                    cmdflags[currentcmd] = 1;   // MARK THIS COMMAND IS NEW ON NEWRPL
                }

                if((chunk - mainbuffer < length - 11)
                        && !strncmp(chunk, "//@INCOMPAT", 11)) {
                    // CURRENT COMMAND IS INCOMPATIBLE OR CHANGED FROM USERRPL
                    cmdflags[currentcmd] = 2;   // MARK THIS COMMAND IS NEW ON NEWRPL
                }

                if((chunk - mainbuffer < length - 14)
                        && !strncmp(chunk, "//@SHORT_DESC=", 14)) {
                    // FOUND SHORT DESCRIPTION OF THE CURRENT COMMAND
                    cmddesc[currentcmd] = chunk + 14;
                    while((*chunk != '\n') && (*chunk != '\r')
                            && (chunk - mainbuffer < length))
                        ++chunk;
                    cmddesclen[currentcmd] = chunk - cmddesc[currentcmd];

                    if((cmddesclen[currentcmd] >= 5)
                            && !strncmp(cmddesc[currentcmd], "@HIDE", 5)) {
                        // HIDE THE CURRENT COMMAND FROM THE LIST
                        --ncmd;
                        for(k = currentcmd; k < ncmd; ++k) {
                            cmdname[k] = cmdname[k + 1];
                            cmdnamelen[k] = cmdnamelen[k + 1];
                            cmdstring[k] = cmdstring[k + 1];
                            cmdstrlen[k] = cmdstrlen[k + 1];
                            cmddesc[k] = cmddesc[k + 1];
                            cmddesclen[k] = cmddesclen[k + 1];
                        }
                    }
                }

                if(*chunk == 'E') {
                    extcommand = 1;
                    ++chunk;
                }
                else
                    extcommand = 0;
                if(chunk - mainbuffer >= length)
                    break;

                if(*chunk == 'C') {
                    ++chunk;
                    if(chunk - mainbuffer >= length)
                        break;
                    if(*chunk == 'M') {
                        ++chunk;
                        if(chunk - mainbuffer >= length)
                            break;
                        if(*chunk == 'D') {
                            ++chunk;
                            if(chunk - mainbuffer >= length)
                                break;
                            if(*chunk == '(') {
                                ++chunk;
                                if(chunk - mainbuffer >= length)
                                    break;
                                cmdname[ncmd] = chunk;
                                while((chunk - mainbuffer < length)
                                        && ((*chunk != ',') && (*chunk != ' ')
                                            && (*chunk != '\n')))
                                    ++chunk;
                                cmdnamelen[ncmd] = chunk - cmdname[ncmd];

                                if(extcommand) {
                                    if(*chunk != ',') {
                                        // INVALID SYNTAX, FORGET IT
                                        fprintf(stderr,
                                                "Warning: Bad syntax line %d, skipping.\n",
                                                line);
                                    }
                                    else {
                                        ++chunk;
                                        while((chunk - mainbuffer < length)
                                                && ((*chunk != '\"')
                                                    && (*chunk != '\n')
                                                    && (*chunk != ',')))
                                            ++chunk;

                                        if(*chunk == '\"') {
                                            ++chunk;
                                            cmdstring[ncmd] = chunk;
                                            while((chunk - mainbuffer < length)
                                                    && ((*chunk != '\"')
                                                        && (*chunk != '\n')))
                                                ++chunk;
                                            if(*chunk != '\"') {
                                                fprintf(stderr,
                                                        "Warning: Bad syntax line %d, skipping.\n",
                                                        line);
                                            }
                                            else {
                                                cmdstrlen[ncmd] =
                                                        chunk - cmdstring[ncmd];
                                                ++ncmd;
                                            }
                                        }
                                        else {
                                            fprintf(stderr,
                                                    "Warning: Bad syntax line %d, skipping.\n",
                                                    line);
                                        }
                                    }
                                }
                                else {
                                    cmdstring[ncmd] = cmdname[ncmd];
                                    cmdstrlen[ncmd] = cmdnamelen[ncmd];
                                    ++ncmd;
                                }

                                // CHECK IF EVERYTHING IS OK

                                if((cmdstrlen[ncmd - 1] <= 0))
                                    --ncmd;     // THIS IS AN UNNAMED COMMAND, REMOVE FROM THE LIST
                                else {
                                    // TODO: GET THE TOKENINFO DATA

                                }
                            }
                        }
                    }
                }

                // DONE, SKIP TO NEXT LINE

                //SKIP TO THE NEXT LINE
                ++line;
                while((*chunk != '\n') && (*chunk != '\r')
                        && (chunk - mainbuffer < length))
                    ++chunk;
                while(((*chunk == '\n') || (*chunk == '\r'))
                        && (chunk - mainbuffer < length))
                    ++chunk;

            }

            // DONE ANALYZING THE FILE, NOW CREATE THE SUMMARY INFO TEMPLATES

            if(ncmd) {

                f = fopen(outputfile, "wb");
                if(f == NULL) {
                    fprintf(stderr, "error: Can't open %s for writing.\n",
                            outputfile);
                    if(needcleanup)
                        free(outputfile);
                    free(mainbuffer);
                    closedir(dirhandle);
                    return 1;
                }

                int j;

                char buffer[1024];

                int nnew = 0;
                for(j = 0; j < ncmd; ++j)
                    nnew += cmdflags[j] & 1;

                // OUTPUT THE PREAMBLE FOR THE WIKI

                fprintf(f, "<button collapse=\"");
                fwrite(outputfile + nameoffset, namelen, 1, f);
                fprintf(f, "-commands\" block=\"true\" >**");
                if(libtitle)
                    fwrite(libtitle, libtitlelen, 1, f);
                if(!nnew)
                    fprintf(f,
                            "**  <badge>%d</badge></button>\n<collapse id=\"",
                            ncmd);
                else if(ncmd != nnew)
                    fprintf(f,
                            "**  <badge>%d</badge> <badge>%d NEW</badge></button>\n<collapse id=\"",
                            ncmd, nnew);
                else
                    fprintf(f,
                            "**  <badge>%d NEW</badge></button>\n<collapse id=\"",
                            ncmd);
                fwrite(outputfile + nameoffset, namelen, 1, f);
                fprintf(f, "-commands\" collapsed=\"true\">\n");
                fprintf(f, "\n^ Command  ^ Short Description ^  ^\n");

                // OUTPUT THE COMPLETE LIST OF COMMANDS

                for(k = 0; k < ncmd; ++k) {
                    fprintf(f, "| **[[manual:chapter6:");
                    fwrite(outputfile + nameoffset, namelen, 1, f);
                    fprintf(f, ":cmd_");
                    for(j = 0; j < cmdnamelen[k]; ++j)
                        buffer[j] = tolower(cmdname[k][j]);
                    fwrite(buffer, cmdnamelen[k], 1, f);
                    fprintf(f, "|");
                    fwrite(cmdstring[k], cmdstrlen[k], 1, f);
                    fprintf(f, "]]** | ");
                    if(cmddesclen[k])
                        fwrite(cmddesc[k], cmddesclen[k], 1, f);
                    if(cmdflags[k] == 1)
                        fprintf(f, " | <badge>NEW</badge> |\n");
                    else if(cmdflags[k] == 2)
                        fprintf(f, " | <badge>CHANGED</badge> |\n");
                    else
                        fprintf(f, " |  |\n");
                }

                fprintf(f, "</collapse>\n");
                // CLOSE THE OUTPUT FILE
                if(needcleanup) {
                    free(outputfile);
                    needcleanup = 0;
                }
                fclose(f);

                // TODO: CREATE THE DIRECTORY FOR THE TEMPLATES

            }
        }

    }

    closedir(dirhandle);
    return 0;

}
