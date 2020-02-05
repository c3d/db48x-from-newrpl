/*
* Copyright (c) 2014-2016, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/
#ifndef _SDDRIVER_H
#define _SDDRIVER_H

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CON 0
#define DAT 4
#define PULLUP 8

typedef struct
{
    int SysFlags;       // 1=SDIO interface setup, 2=SDCard initialized, 4=Valid RCA obtained, 8=Bus configured OK, 16=SDHC/SDXC card
    int Rca;
    int BusWidth;
    int MaxBlockLen;
    int WriteBlockLen;
    int CurrentBLen;
    int WantedClock;
    unsigned int CardSize;
    unsigned int CID[4];
} SD_CARD;

int SDIOSetup(SD_CARD * card, int shutdown);
int SDCardInserted();
int SDCardWriteProtected();
int GetPCLK();
int SDSetFastPCLK();
void SDRestorePCLK(int original);
void SDSetClock(int sdclk);
int SDGetClock();
void SDPowerDown();
void SDPowerUp();
int SDSlowDown();
int SDWaitResp(int mask);
int SDSendCmd(int cmdnum, int arg, int cmdmsk, int mask);
int SDSendACmd(int rca, int cmdnum, int arg, int cmdmsk, int mask);
int SDSendCmdNoResp(int cmdnum, int arg);
int SDSendCmdShortResp(int cmdnum, int arg, int *response);
int SDSendCmdLongResp(int cmdnum, int arg, int *response);
int SDSendACmdShortResp(int rca, int cmdnum, int arg, int *response);
int SDSendACmdLongResp(int rca, int cmdnum, int arg, int *response);
void SDIOReset();
int SDSelect(int RCA);
int SDInit(SD_CARD * card);
int SDGetNewRCA(int *_CID);
void SDDResetFIFO();
int SDDStop();
int SDDSetBlockLen(SD_CARD * card, int bitlen);
int SDDInTransfer(SD_CARD * card);
int SDDRead(uint64_t SDAddr, int NumBytes, unsigned char *buffer,
        SD_CARD * card);
int SDCardInit(SD_CARD * card);
int SDDWrite(uint64_t SDAddr, int NumBytes, unsigned char *buffer,
        SD_CARD * card);

#endif
