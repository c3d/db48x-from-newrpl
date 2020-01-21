/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/  
        
#include <newrpl.h>
#include <ui.h>
        
// TEMPORARY MEMORY ALLOCATION USES NEWRPL DECIMAL LIBRARY MEMORY MANAGER
           
#include "../fsystem/fsyspriv.h"
// SD MODULE
        
#define EXTINT0 ((unsigned int *)(IO_REGS+0x88))
#define SRCPND ((volatile unsigned int *)(INT_REGS+0))
  int GetPCLK()  { int CLKSLOW = *HWREG(CLKREG, 0x10);
    int FCLK = 12000000;
     if(CLKSLOW & 0x10)       // slow mode
    {
        if(CLKSLOW & 7)
            FCLK /= (CLKSLOW & 7) << 1;
    }
    
    else        // fast mode
    {
        int PLLCON = *HWREG(CLKREG, 0x4);
        int m = ((PLLCON >> 12) & 0xff) + 8, p =
                ((PLLCON >> 4) & 0x1f) + 2, s = PLLCON & 3;
         FCLK = ((unsigned)m * 12000000) / (p * (1 << s));
    } int CLKDIVN = *HWREG(CLKREG, 0x14);
    if(CLKDIVN & 2)
        FCLK >>= 1;
    if(CLKDIVN & 1)
        FCLK >>= 1;
    return FCLK;
}
  int SDGetClock()  { int pclk = GetPCLK();
     return pclk / ((*SDIPRE + 1) << 1);
 }
   void SDSetClock(int sdclk)  { int pclk = GetPCLK();
    
//      printf("PCLK=%d\n",pclk);
//      printf("requested=%d\n",sdclk);
    int prescaler = (pclk + sdclk) / (sdclk << 1) - 1;  // +sdclk to round up
    if(prescaler < 0)
        prescaler = 0;
    if(prescaler > 0xff)
        prescaler = 0xff;
    
//      printf("Prescaler=%d\n",prescaler);
//      keyb_getkeyM(1);
            *SDIPRE = prescaler;
}

  
#ifndef CONFIG_NO_FSYSTEM
        
// IRQ HANDLER FOR CARD INSERTION/REMOVAL
void __SD_irqeventinsert()  { halUpdateStatus();
     if(((*EXTINT0) & 0x7000) == 0x2000)
         {
        
                // CARD WAS JUST REMOVED
                
                // SET TO TRIGGER ON INSERTION
                *EXTINT0 = ((*EXTINT0) & (~0x7000)) | (0x4000);
         
                // TODO: CHECK FOR DIRTY FILE SYSTEM, WARN USER
                if(FSIsDirty())
            Exceptions |= EX_DIRTYFS;
        
        else
            FSShutdownNoCard();
         }
    
    else
         {
        
                // CARD WAS JUST INSERTED
                
                // NOTHING TO DO, JUST SET TO TRIGGER ON REMOVAL
                *EXTINT0 = ((*EXTINT0) & (~0x7000)) | (0x2000);
         if(FSIsInit())
             {
            
                    // CHECK IF FILE SYSTEM WAS DIRTY
            int error;
            error = FSVolumePresent(FSystem.Volumes[FSystem.CurrentVolume]);
             if(error != FS_OK)
                FSShutdownNoCard();     // UNMOUNT OLD FILE SYSTEMS
            }
          }
}
         int SDIOSetup(SD_CARD * card, int shutdown)  { int f, k;
      if(!shutdown)
         {
         
// SET GPF3 AS INPUT
                *GPF(CON) = *GPF(CON) & (~0xc0);
        
// DISABLE PULLUP
                *GPF(PULLUP) = *GPF(PULLUP) | 8;
        
// SET WRITE PROTECT DETECTION PIN TO INPUT AND DISABLE PULLUPS
                *GPD(CON) = *GPD(CON) & ~0xc0;
        *GPD(PULLUP) = *GPD(PULLUP) | 0x8;
         *SRCPND = 8; // CLEAR ANY PENDING INTERRUPT REQUESTS
        f = *GPF(DAT) & 8;     // READ CURRENT VALUE, CARD INSERTED?
        if(f)
            *EXTINT0 = ((*EXTINT0) & (~0x7000)) | (0x2000);     // SET TO TRIGGER INTERRUPT ON PROPER EDGE
        else
            *EXTINT0 = ((*EXTINT0) & (~0x7000)) | (0x4000);
         __irq_addhook(3, &__SD_irqeventinsert);      // INSTALL IRQ HANDLER
        *GPF(CON) = (*GPF(CON) & (~0xc0)) | (0X80);    // SET PIN FUNCTION TO EINT3
          
// ENABLE PULLUPS FOR SDCMD AND SDDATx LINES
                *GPE(PULLUP) = *GPE(PULLUP) & 0xf83f;
        
// ENABLE PIN FUNCTION FOR SDCMD AND SDDAT LINES
                *GPE(CON) = ((*GPE(CON)) & ~0x003ffc00) | 0x002aa800;
         *HWREG(CLKREG, 0xc) = *HWREG(CLKREG, 0xc) | 0x200;   // ENABLE CLOCK TO SD INTERFACE
         __irq_unmask(3);     // ENABLE INTERRUPT ON THIS PIN
         if(card)
            card->SysFlags |= 1;
        return TRUE;
        }
    
    else
         {
        
// STOP SD-CARD INTERFACE
                
                // SET GPF3 AS INPUT
                *GPF(CON) = *GPF(CON) & (~0xc0);
        *GPF(PULLUP) = *GPF(PULLUP) | 8;
         f = *GPF(DAT) & 8;   // READ CURRENT VALUE, CARD INSERTED?
        if(f)
            *EXTINT0 = ((*EXTINT0) & (~0x7000)) | (0x2000);     // SET TO TRIGGER INTERRUPT ON PROPER EDGE
        else
            *EXTINT0 = ((*EXTINT0) & (~0x7000)) | (0x4000);
          *GPF(CON) = (*GPF(CON) & (~0xc0)) | (0X80);        // SET PIN FUNCTION TO EINT3
        
// DISABLE PULLUPS FOR SDCMD AND SDDATx LINES
                *GPE(PULLUP) = *GPE(PULLUP) | (~0xf83f);
        
// SET PIN FUNCTION FOR SDCMD AND SDDAT LINES (ALL OUTPUTS)
                *GPE(CON) = ((*GPE(CON)) & ~0x003ffc00) | 0x00155400;
         
// SET WRITE PROTECT DETECTION PIN TO INPUT AND DISABLE PULLUPS
                *GPD(CON) = *GPD(CON) & ~0xc0;
        *GPD(PULLUP) = *GPD(PULLUP) | 0x8;
         
// SEND LOW SIGNAL ON ALL SD LINES
                *GPE(DAT) = *GPE(DAT) & 0xf81f;
         *HWREG(CLKREG, 0xc) = *HWREG(CLKREG, 0xc) & (~0x200);        // DISABLE CLOCK TO SD INTERFACE
        if(card)
            card->SysFlags &= ~1;
        return FALSE;
        }
 }
  int SDCardInserted()  { int prev = *GPF(CON);
     
            // SET GPF3 AS INPUT
            *GPF(CON) = *GPF(CON) & (~0xc0);
    *GPF(PULLUP) = *GPF(PULLUP) | 8;
     int result = (*GPF(DAT)) & 8;
     *GPF(CON) = prev;        // RESTORE PREVIOUS FUNCTION
    return result;
 }
 int SDCardWriteProtected()  { return ((*GPD(DAT)) & 8) ? 0 : 1;
}

  
// SET PCLK AT FASTER SPEED TO ACHIEVE 20 MHz ON THE SD CARD
// IT AFFECTS UART, USB AND OTHER DEVICES, SO DON'T USE IF
// OTHER DEVICES ARE ACTIVE (WILL PROBABLY PREVENT ANY COMMUNICATION)
// RETURNS ORIGINAL PDIVN VALUE
int SDSetFastPCLK()  { unsigned original = *HWREG(CLKREG, 0x14) & 1;
    *HWREG(CLKREG, 0x14) &= ~1;
    return original;
}
 void SDRestorePCLK(int original) 
        { *HWREG(CLKREG, 0x14) =
            (*HWREG(CLKREG, 0x14) & (~1)) | (original & 1);
}     void SDPowerDown()  { *SDICON = *SDICON & 0xfffffffe;
}  void SDPowerUp()  { int f;

    *SDICON = *SDICON | 1;
     tmr_delayus(200);        // MINIMUM 64 SDCLK
//            for(f=0;f<100;++f) ;                      // wait for 64 SDCLK
}    int SDSlowDown()  { int a = *SDIPRE & 0xff;

    if(a == 0xff)
        return FALSE;
    SDPowerDown();
    if(a < 4)
        a++;
    
    else
        a <<= 1;
    if(a > 0xff)
        a = 0xff;
    *SDIPRE = a;
    
//printf("Slowdown=%d\n",a);
//keyb_getkeyM(1);
            SDPowerUp();
     return TRUE;
}

  
#define SD_MAX_WAIT 100000
int SDWaitResp(int mask)  { int volatile a;
    register int count = 0;
     while((((a = (*SDICSTA & mask)) == 0) || (*SDICSTA & 0x100))
            && SDCardInserted() && (count < SD_MAX_WAIT))
         {
        ++count;
        if(*SDIFSTA & 0x100)
            *SDIDCNT = *SDIDAT; // KEEP THE CLOCK MOVING WHEN RECEIVING A TRANSMISSION
        }
    if(count == SD_MAX_WAIT)
         {
        
//printf("Hang: %04X, org=%04X\n",a,*SDICSTA);
//printf("DSTA=%04X\n",*SDIDSTA);
//printf("FSTA=%04X\n",*SDIFSTA);
//keyb_getkeyM(1);
                return 0x400;
        }
    *SDICSTA = *SDICSTA & 0x1e00;
    return a;
}
  int SDSendCmd(int cmdnum, int arg, int cmdmsk, int mask) 
        { int volatile a;
    int trials;
    if(!SDCardInserted())
        return FALSE;
    halFlags |= HAL_NOCLOCKCHANGE;
     for(trials = 0; trials < 100; ++trials)
         {
        *SDICARG = arg;
        *SDICCON = (cmdnum & 63) | cmdmsk;
        a = SDWaitResp(mask);
        if(a & 0x400)
             {
            
                    // TIMEOUT
                    // SIMPLY RETRY
                    continue;
            }
        if(a & 0x1000)
            continue;   // RETRY IF CRC FAILED
        if(a & 0xa00)
            break;      // FINISH IF COMMAND DONE OR RESPONSE DONE
        }
    halFlags &= ~HAL_NOCLOCKCHANGE;
     if(trials >= 100)
        return FALSE;
    return TRUE;
}
  int SDSendACmd(int rca, int cmdnum, int arg, int cmdmsk, int mask) 
        { int volatile a;
    int trials;
    if(!SDCardInserted())
        return FALSE;
    halFlags |= HAL_NOCLOCKCHANGE;
     for(trials = 0; trials < 100; ++trials)
         {
        *SDICARG = rca;
        *SDICCON = 0x377;
        a = SDWaitResp(0x1600);
         if((a & 0x1600) == 0x200)
             {
             *SDICARG = arg;
            *SDICCON = (cmdnum & 63) | cmdmsk;
            a = SDWaitResp(mask);
             }
        if(a & 0x400)
             {
            
                    // ** CODE REMOVED - OBSOLETE (NEVER CALLED IN MY TESTS) **
                    //if(trials<80) continue;           // RETRY 20 TIMES BEFORE GIVING ERROR
//                      printf("Slowdown!\n");
//                      keyb_getkeyM(1);
                    //if(!SDSlowDown()) { printf("A slowdown failed\n"); return FALSE;}  else { trials-=80; continue; }         // retry if Timeout
                    continue;
            }
        if(a & 0x1000)
            continue;   // RETRY IF CRC FAILED
        if(a & 0xa00)
            break;      // FINISH IF COMMAND DONE OR RESPONSE DONE
        }
    
//printf("trials=%d\n",trials);
            halFlags &= ~HAL_NOCLOCKCHANGE;
     if(trials >= 100)
         {
        
                /*printf("A too many trials\n"); */ return FALSE;
        }
    return TRUE;
}
    int SDSendCmdNoResp(int cmdnum, int arg) 
        { return SDSendCmd(cmdnum, arg, 0x140, 0xc00);
}
 int SDSendCmdShortResp(int cmdnum, int arg, int *response) 
        { register int mask;
    if(cmdnum == 1 || cmdnum == 9 || cmdnum == 41)
        mask = 0x600;
    
    else
        mask = 0x1600;
    
//printf("mask=%04X\n",mask);
            if(!SDSendCmd(cmdnum, arg, 0x340, mask))
         {
        return FALSE;
        }
    *response = *SDIRSP0;
    return TRUE;
}
      int SDSendCmdLongResp(int cmdnum, int arg, int *response) 
        {  if(!SDSendCmd(cmdnum, arg, 0x740, 0x600))
        return FALSE;
    response[0] = *SDIRSP3;
    response[1] = *SDIRSP2;
    response[2] = *SDIRSP1;
    response[3] = *SDIRSP0;
     return TRUE;
}
  int SDSendACmdShortResp(int rca, int cmdnum, int arg, int *response) 
        { register int mask;
    if(cmdnum == 1 || cmdnum == 9 || cmdnum == 41)
        mask = 0x600;
    
    else
        mask = 0x1600;
    
//printf("mask=%04X\n",mask);
            if(!SDSendACmd(rca, cmdnum, arg, 0x340, mask))
        return FALSE;
    *response = *SDIRSP0;
    return TRUE;
}
      int SDSendACmdLongResp(int rca, int cmdnum, int arg, int *response) 
        {  if(!SDSendACmd(rca, cmdnum, arg, 0x740, 0x600))
        return FALSE;
    response[0] = *SDIRSP3;
    response[1] = *SDIRSP2;
    response[2] = *SDIRSP1;
    response[3] = *SDIRSP0;
     return TRUE;
}

       
// CLEAN THE SDIO INTERFACE
void SDIOReset()  { int *ptr = (int *)SDREG, f;
     for(f = 0; f < 0x44; f += 4)
        *ptr = 0;
     SDDResetFIFO();
}
  int SDSelect(int RCA)  { int a;
    return (RCA) ? SDSendCmdShortResp(7, RCA, &a) : SDSendCmdNoResp(7, 0);
}

   
// FULLY INITIALIZE THE SDCARD INTERFACE
// RETURNS TRUE IF THERE IS A CARD
// FALSE IF THERE'S NO CARD
    int SDInit(SD_CARD * card)  {  if(card->SysFlags & 2)
        return TRUE;
     SDIOReset();
    if(SDIOSetup(card, FALSE))
         {
         card->WantedClock = 600000;
        SDSetClock(card->WantedClock); // SET INITIAL CLOCK AT 600 kHz
        SDPowerUp();   // START CLOCK
        *SDICSTA = *SDICSTA & 0x1e00;  // CLEAR ALL BITS
        *SDIDTIMER = 0xFFFF;    // DEFAULT TIMEOUT VALUE
        *SDIBSIZE = 512;        // DEFAULT BLOCK SIZE
        *SDIDCON = 0x4000;
         *SDIDSTA = *SDIDSTA & 0x3fc; // CLEAR ALL BITS
        *SDICON |= 3;  // RESET FIFO AND ENABLE CLOCK
        card->SysFlags |= 2;
        return TRUE;
         }
    
    else
         {
        card->SysFlags &= ~2;
        return FALSE;
        }
}

 
#define WAIT_LIMIT 10
// STORE CID OF THE CARD ON _CID (int[4]) AND RETURN NEW RCA
int SDGetNewRCA(int *_CID)  { unsigned int CID[4], rca, sd2;
    int counter, k;
     for(k = 0; k < WAIT_LIMIT; ++k)
         {
         sd2 = 0;
         if(!SDSendCmdNoResp(0, 0))
             {
            continue;
            }
         if(SDSendCmdShortResp(8, 0x1aa, &rca))
             {
            
                    // CARD IS SD 2.0 COMPLIANT!
                    if(rca == 0x1aa)
                sd2 = 1;
            }
         counter = 0;
        
        do
             {
            
//printf("Reset ok\n");
                    if(!SDSendCmdShortResp(55, 0, (int *)CID))
                 {
                ++counter;
                continue;
                }
            
//printf("cmd55 ok\n");
                    if(!SDSendCmdShortResp(41, (sd2 ? 0x40ff8000 : 0x00ff8000),
                        (int *)CID))
                 {
                ++counter;
                continue;
                }
            counter = -1;
            
//printf("cmd41 ok\n");
        } while(!(CID[0] & 0x80000000) && (counter < WAIT_LIMIT));
        if(counter == WAIT_LIMIT)
             {
            continue;
            }
        break;
        }
    if(k == WAIT_LIMIT)
         {
        return FALSE;
        }
     
//printf("Cmd41 loop ok\n");
            
// CHECK IF THE CARD ACKNOWLEDGED SDHC BIT
            if(sd2 && (CID[0] & 0x40000000))
         {
        
                // THIS IS AN SDHC CARD
                sd2 = 2;
        }
      if(!SDSendCmdLongResp(2, 0, _CID))
         {
        return FALSE;
        }
    
//printf("CID ok\n");
            if(!SDSendCmdShortResp(3, 0, (int *)CID))
         {
        return FALSE;
        }
    
//printf("RCA ok\n");
            rca = CID[0] & 0xffff0000;
    if(sd2 == 2)
        rca |= 1;       // MARK THAT THIS IS AN SDHC CARD!
    return rca;
}
  void SDDResetFIFO()  { *SDICON = *SDICON | 2;
}   

// STOP DATA TRANSMISSION
int SDDStop()  { 
// FINISH TRANSMISSION
//printf("ABout to stop transmission\n");
//keyb_getkeyM(1);
//*SDIDCON=0x34000;
//printf("SDIDCNT=%04X\n",*SDIDCNT);
//printf("SDIBSIZE=%04X\n",*SDIBSIZE);
            while(!SDSendCmd(12, 0, 0x340, 0x1600))
        {if((*SDIDSTA & 1) && (*SDIFSTA & 0x100))  {
// ONGOING READ OPERATION W/FULL FIFO, CLOCK MAY BE STOPPED
//printf("Clock stopped!\n");
//keyb_getkeyM(1);
                    while(*SDIFSTA & 0x1000) * SDIDCNT = *SDIDAT; continue; }
                    
                    else
                     {
                        
//printf("Failed to STOP transmission\n");
//*SDIDCON=0x34000;
//keyb_getkeyM(1);
                    break; }
                     }
                 SDDResetFIFO(); 
//printf("Stopped= %04X\n",*SDIDSTA);
//printf("Data status=%04X\n",*SDIDSTA);
//printf("FIFO Status=%04X\n",*SDIFSTA);
            return TRUE; }
             int SDDSetBlockLen(SD_CARD * card, int bitlen) 
            { int a;
                if(bitlen > card->MaxBlockLen) bitlen = card->MaxBlockLen;
                if(bitlen == card->CurrentBLen) return TRUE;
                if(!SDSendCmdShortResp(16, 1 << bitlen, &a)) return FALSE;
                *SDIBSIZE = 1 << bitlen; card->CurrentBLen = bitlen;
                return TRUE; }
              int SDDInTransfer(SD_CARD * card)  { int a = (*SDIDSTA & 3);
                 if(a) return a;   return a;  *SDIDSTA = *SDIDSTA & ~3;
                 
// CHECK FOR BUSY SIGNAL
                *SDIDCON = 0x1000;  
// WAIT UNTIL BUSY DETECT STARTS
//while( !(*SDIDSTA&3) ) { printf("u %04X\n",*SDIDSTA); }
                
// CHECK FOR BUSY FINISH
                while(!(a = *SDIDSTA & 0x2C));  
//printf("busy finished %04X\n",a);
            *SDIDCON = 0x4000; *SDIDSTA = *SDIDSTA & ~3;
                 if(a & 0x28) return 0; return 1; }
                
// READS WORDS DIRECTLY INTO BUFFER
// AT THE CURRENT BLOCK LENGTH
// CARD MUST BE SELECTED
int SDDRead(uint64_t SDAddr, int NumBytes, unsigned char *buffer, SD_CARD * card)  {  int blocks; uint64_t endaddr, startaddr; int status; unsigned int word; restartall:startaddr = SDAddr >> card->CurrentBLen;
                endaddr = SDAddr + NumBytes;
                blocks =
                ((endaddr + (1 << card->CurrentBLen) -
                        1) >> card->CurrentBLen) - startaddr;
                startaddr <<= card->CurrentBLen;
                 halFlags |= HAL_NOCLOCKCHANGE;
                 SDSetClock(card->WantedClock);   
//printf("DSTA=%04X\n",*SDIDSTA);
//printf("FSTA=%04X\n",*SDIFSTA);
                *SDIDSTA = *SDIDSTA & (~3);    // RESET ALL BITS
                SDDResetFIFO();
                  *SDIDCON = 0xa2000 | blocks | card->BusWidth; 
//printf("blocks=%d\n",blocks);
                if(!SDSendCmdShortResp((blocks == 1) ? 17 : 18,
                        ((card->SysFlags & 16) ? startaddr >> 9 : startaddr),
                        &status))  { 
//printf("failed read cmd\n");
                halFlags &= ~HAL_NOCLOCKCHANGE; return FALSE; }
                
//printf("Read=%08X\n",status);
//printf("cmd READ started\n");
                
//status=*SDIDSTA;
//*SDIDSTA=status&(~3);         // RESET ALL BITS
                
// OBSOLETE - NEVER HAPPENS
                
                /*
                   printf("status=%04X\n",status);
                   keyb_getkeyM(1);
                   if(status&0x20) {                // CHECK FOR TIMEOUT
                   printf("Timeout!\n");
                   if(!SDSlowDown()) {
                   printf("Slowdown failed\n");
                   return FALSE;
                   }
                   else continue;
                   SDDStop();
                   continue;
                   
                   }
                 */ 
                 
//printf("First loop done\n");
                if(((int)SDAddr | (int)endaddr | (int)buffer) & 3)  { 
// SLOW BYTE-ALIGNED LOOP
//printf("Byte loop active\n");
                    while(startaddr < endaddr)  {
//printf("2nd loop\n");
// START RECEIVING DATA
                            while(!(*SDIFSTA & 0x1000)) 
                            {if(!SDCardInserted())  {
//printf("no card\n");
                                    SDDStop();
                                            halFlags &=
                                            ~HAL_NOCLOCKCHANGE;
                                             return FALSE; }
                                    status = *SDIDSTA; 
//printf("pt2\n");
                                    if((status & 0x164) || !(status & 1))  { 
// ANY ERROR
//printf("read Data error=%04X\n",status);
                                        SDDStop(); if(status & 0x20)  { 
//printf("Timeout!\n");
                                            if(!SDSlowDown())  {
//printf("Slowdown failed\n");
                                                halFlags &=
                                                    ~HAL_NOCLOCKCHANGE;
                                                     return FALSE; }
                                                while((*SDIFSTA & 0x1000))  {
                                                    *SDIDCNT = *SDIDAT; }
                                            while((*SDIDSTA & 0x3));
                                                goto restartall;  }
                                             if(status & 0x4)  { 
// FAILED TO GET START BIT
// MOST LIKELY CARD IS BUSY
//printf("Card busy?\n");
                                                while((*SDIFSTA & 0x1000)) 
                                                {*SDIDCNT = *SDIDAT; }
                                                while((*SDIDSTA & 0x3));
                                                    goto restartall;  }
                                             *SDIDSTA = status & (~3);
                                                 halFlags &=
                                                ~HAL_NOCLOCKCHANGE;
                                                 return FALSE; }
                                            
//printf("loop %d\n",startaddr);
                                            /*
                                               if(keyb_isKeyPrM(KB_ENT)) {
                                               printf("DSTA=%04X\n",*SDIDSTA);
                                               printf("FSTA=%04X\n",*SDIFSTA);
                                               }
                                               if(keyb_isKeyPrM(KB_SPC)) {
                                               printf("DSTA=%04X\n",*SDIDSTA);
                                               printf("FSTA=%04X\n",*SDIFSTA);
                                               return FALSE;
                                               }
                                             */ 
                                            }
                                             word =
                                            *((unsigned char *)SDIDAT);
                                            if(startaddr >=
                                                SDAddr)  { *buffer = word;
                                                ++buffer; }
                                        ++startaddr; }
                                        
// EXIT BYTE LOOP
                                        unsigned int r1 = 0, r2 = 0, r3 =
                                        0, r4 = 0;  
                                        do
                                             {
                                            while((r1 = *SDIFSTA) != r2) r2 = *SDIFSTA;        // READ UNTIL STABLE VALUE
                                            while((r3 = *SDIDSTA) != r4) r4 = *SDIDSTA; // READ UNTIL STABLE VALUE
                                            if((r1 & 0x1200) == 0x200) break;
                                            if(!(r3 & 3)) break; 
//printf("FSTA=%04X\n",*SDIFSTA);
                                            if(r1 & 0x1000) *
                                            ((unsigned char *)SDIDCNT) =
                                            *((unsigned char *)SDIDAT);
                                            } while(1);   } 
                                        else
                                         {
                                         
// OPTIMIZED WORD LOOP
//printf("Word loop\n");
                                        while(startaddr < endaddr) 
                                        { while(!(*SDIFSTA & 0x1000)) 
                                            {if(!SDCardInserted())  {
//printf("no card\n");
                                                SDDStop();
                                                            halFlags
                                                            &=
                                                            ~HAL_NOCLOCKCHANGE;
                                                             return FALSE; }
                                                status = *SDIDSTA;
                                                if((status & 0x164)
                                                    || !(status & 1))  { 
//printf("read data error=%04X\n",status);
                                                    SDDStop();
                                                    if(status & 0x20)  { 
//printf("Timeout!\n");
                                                        if(!SDSlowDown())  {
//printf("Slowdown failed\n");
                                                            halFlags &=
                                                                ~HAL_NOCLOCKCHANGE;
                                                                 return
                                                                FALSE; }
                                                            while((*SDIFSTA &
                                                                    0x1000))  {
                                                                *SDIDCNT =
                                                                *SDIDAT; }
                                                        while((*SDIDSTA &
                                                                    0x3));
                                                            goto
                                                            restartall;  }
                                                        if(status & 0x4)  { 
// FAILED TO GET START BIT
// MOST LIKELY CARD IS BUSY
//printf("Card busy?\n");
//keyb_getkeyM(1);
                                                            while((*SDIFSTA &
                                                                    0x1000)) 
                                                            {*SDIDCNT =
                                                                    *SDIDAT; }
                                                            while((*SDIDSTA
                                                                        &
                                                                        0x3));
                                                                goto
                                                                restartall;  }
                                                        *SDIDSTA =
                                                            status & (~3);
                                                            halFlags &=
                                                            ~HAL_NOCLOCKCHANGE;
                                                             return FALSE; }
                                                        }
                                                         word = *SDIDAT;
                                                         if(startaddr >=
                                                            SDAddr)  {
                                                            *((unsigned int *)
                                                                buffer) = word;
                                                            buffer += 4;
                                                            } startaddr += 4;
                                                        }   
// EXIT WORD LOOP
                                                        unsigned int r1 =
                                                        0, r2 = 0, r3 = 0, r4 =
                                                        0;  
                                                        do
                                                             {
                                                            while((r1 = *SDIFSTA) != r2) r2 = *SDIFSTA;        // READ UNTIL STABLE VALUE
                                                            while((r3 = *SDIDSTA) != r4) r4 = *SDIDSTA; // READ UNTIL STABLE VALUE
                                                            if((r1 & 0x1200) ==
                                                                0x200) break;
                                                            if(!(r3 & 3))
                                                            break; 
//printf("FSTA=%04X\n",*SDIFSTA);
                                                            if(r1 & 0x1000) *
                                                            SDIDCNT = *SDIDAT;
                                                            } while(1);
                                                         }     
//printf("DSTA=%04X\n",r3);
//printf("FSTA=%04X\n",r1);
                                                        if(blocks > 1) 
                                                    { SDDStop(); 
//printf("DSTA=%04X\n",*SDIDSTA);
//printf("bytes=%d\n",startaddr-SDAddr);
//printf("st-e=%d\n",endaddr-startaddr);
                                                        }   
// FINISH THE LAST BLOCK
                                                        else
                                                         {
                                                        int g = 0;
                                                            while((*SDIDSTA
                                                                    & 0x3)
                                                                && g <
                                                                100)++ g; }
                                                         if((*SDIDSTA & 0x1fc)
                                                            != 0x10)  { 
//keyb_getkeyM(1);
//printf("blocks=%d\n",blocks);
//printf("DSTA=%04X\n",*SDIDSTA);
//printf("FSTA=%04X\n",*SDIFSTA);
//printf("Data Error!!!\n");
                                                            *SDIDSTA = 0x7fc;   // RESET ALL BITS
//keyb_getkeyM(1);
                                                        halFlags &=
                                                            ~HAL_NOCLOCKCHANGE;
                                                             return FALSE; }
                                                         *SDIDSTA = 0x7fc;    // RESET ALL BITS
//if(startaddr-SDAddr!=NumBytes) {
//printf("What??? %d==%d\n",startaddr-SDAddr,NumBytes);
//keyb_getkeyM(1);
//}
                                                        halFlags &= ~HAL_NOCLOCKCHANGE;  return (startaddr - SDAddr); // RETURN NUMBER OF BYTES RECEIVED
                                                        }
                                                           int
                                                        SDCardInit(SD_CARD *
                                                            card)  { unsigned
                                                            CSD[4];
                                                            halFlags |=
                                                            HAL_NOCLOCKCHANGE;
                                                             card->SysFlags =
                                                            0;
                                                            if(!SDInit(card)) 
                                                            { halFlags &=
                                                                ~HAL_NOCLOCKCHANGE;
                                                                 return FALSE;
                                                                }
                                                        
//printf("A");
                                                        card->Rca =
                                                        SDGetNewRCA((int *)
                                                            card->CID);
                                                        if(!card->
                                                            Rca)  { halFlags
                                                            &=
                                                            ~HAL_NOCLOCKCHANGE;
                                                            return FALSE; }
                                                        
//printf("B");
//else printf("RCA ok\n");
                                                        card->SysFlags |= 4;    // MARK VALID RCA OBTAINED
                                                        if(card->Rca & 1) 
                                                        { card->SysFlags |= 16;
                                                            card->Rca &= ~1; }
                                                         
// SWITCH TO HIGH SPEED MODE
                                                        SDPowerDown();
                                                        card->WantedClock =
                                                        6000000;
                                                        SDSetClock(card->
                                                            WantedClock);
                                                        SDPowerUp();  
// GET CSD TO OBTAIN TOTAL SIZE OF CARD, SECTOR AND BLOCK LENGTH
                                                        if(!SDSendCmdLongResp(9,
                                                                card->Rca,
                                                                (int *)CSD))  {
                                                            halFlags &=
                                                            ~HAL_NOCLOCKCHANGE;
                                                            return FALSE; }
                                                        
//printf("C");
//printf("CSD ok\n");
                                                        
// CHECK IF CSD IS VERSION 2.0
switch (CSD[3] & 0xc0000000)  { case 0:
                                                            
                                                            // CSD 1.0
                                                            card->CardSize = ((((CSD[2] & 0x3ff) << 2) | (CSD[1] >> 30)) + 1) << (((CSD[1] >> 15) & 0x7) + 2); card->CurrentBLen = card->MaxBlockLen = ((CSD[2] >> 16) & 0xf); card->WriteBlockLen = ((CSD[0] >> 22) & 0xf); card->CardSize *= 1 << (card->MaxBlockLen - 9); // IN 512 BYTE SECTORS
break; case 0x40000000:
                                                            
                                                            // CSD 2.0
                                                            card->CurrentBLen = card->MaxBlockLen = 9; card->WriteBlockLen = 9; card->CardSize = (((CSD[1] >> 16) | ((CSD[2] & 0x3f) << 16)) + 1) * 1024;     // IN 512 BYTE SECTORS
                                                            card->SysFlags |= 16;       // THIS IS AN SDHC OR SDXC CARD
                                                        break; }
                                                            if(!SDSelect
                                                            (card->
                                                                Rca))  {
                                                            halFlags &=
                                                            ~HAL_NOCLOCKCHANGE;
                                                            return FALSE; }
                                                         
// DISCONNECT CARD DETECT PULL-UP RESISTOR ON DAT3
                                                        SDSendACmdShortResp
                                                        (card->Rca, 42, 0,
                                                            (int *)CSD);   
// REQUEST SCR REGISTER FOR BUS WIDTH
                                                        if(!SDSendACmdLongResp
                                                            (card->Rca, 51,
                                                                card->Rca,
                                                                (int *)CSD))  {
                                                            halFlags &=
                                                            ~HAL_NOCLOCKCHANGE;
                                                            return FALSE; }
                                                         card->BusWidth = 0;
                                                        if((CSD[1] & 0x40000))
                                                         { 
// ENABLE WIDE BUS SUPPORT
                                                            if
                                                            (!SDSendACmdShortResp
                                                                (card->Rca, 6,
                                                                    2,
                                                                    (int *)CSD))
                                                             {halFlags &=
                                                                    ~HAL_NOCLOCKCHANGE;
                                                                    return
                                                                    FALSE; }
                                                                
//printf("Wide bus selected\n");
                                                            card->BusWidth =
                                                                0x10000; }
                                                              card->SysFlags |= 8;   // SysFlags==0xf --> CARD WAS FULLY INITIALIZED WITH NO PROBLEMS
                                                             SDDSetBlockLen(card, 9); // FORCE CARD BLOCK LEN TO 512 BYTE SECTORS
                                                             SDSelect(0);     // DESELECT CARD
                                                            halFlags &=
                                                            ~HAL_NOCLOCKCHANGE;
                                                             
//printf("Initcard ok\n");
                                                            return TRUE; }
                                                                             
// WRITE BYTES AT SPECIFIC ADDRESS
// AT THE CURRENT BLOCK LENGTH
// CARD MUST BE SELECTED
                                                            int
                                                            SDDWrite(uint64_t
                                                                SDAddr,
                                                                int NumBytes,
                                                                unsigned char
                                                                *buffer,
                                                                SD_CARD *
                                                                card)  {  int
                                                                blocks;
                                                                uint64_t
                                                                endaddr,
                                                                startaddr,
                                                                finalblock;
                                                                int status,
                                                                blmask;
                                                                char
                                                                *startbuffer =
                                                                NULL,
                                                                *endbuffer =
                                                                NULL;
                                                                 blmask =
                                                                (1 << card->
                                                                    CurrentBLen)
                                                                - 1;
                                                                 startaddr =
                                                                SDAddr >> card->
                                                                CurrentBLen;
                                                                endaddr =
                                                                SDAddr +
                                                                NumBytes;
                                                                blocks =
                                                                ((endaddr +
                                                                        blmask)
                                                                    >> card->
                                                                    CurrentBLen)
                                                                - startaddr;
                                                                startaddr <<=
                                                                card->
                                                                CurrentBLen;
                                                                finalblock =
                                                                startaddr +
                                                                (blocks <<
                                                                    card->
                                                                    CurrentBLen);
                                                                 halFlags |=
                                                                HAL_NOCLOCKCHANGE;
                                                                 SDSetClock
                                                                (card->
                                                                    WantedClock);
                                                                 if(startaddr
                                                                    !=
                                                                    SDAddr)  {
                                                                    startbuffer
                                                                    =
                                                                    simpmallocb
                                                                    (blmask +
                                                                        1);
                                                                    if
                                                                    (!startbuffer)
                                                                     { halFlags &= ~HAL_NOCLOCKCHANGE; return 0; }
                                                            if(SDDRead
                                                                (startaddr,
                                                                    blmask + 1,
                                                                    startbuffer,
                                                                    card) !=
                                                                blmask +
                                                                1)  {
                                                                simpfree
                                                                (startbuffer);
                                                                halFlags &=
                                                                ~HAL_NOCLOCKCHANGE;
                                                                return 0; }
                                                            }
                                                             if(endaddr !=
                                                                finalblock)  {
                                                                if(blocks > 1
                                                                    ||
                                                                    (!startbuffer))
                                                                 {endbuffer =
                                                                        simpmallocb
                                                                        (blmask
                                                                            +
                                                                            1);
                                                                        if
                                                                        (!endbuffer)
                                                                         { if
                                                                            (startbuffer)
                                                                            simpfree
                                                                            (startbuffer);
                                                                            halFlags
                                                                            &=
                                                                            ~HAL_NOCLOCKCHANGE;
                                                                            return
                                                                            0;
                                                                            }
                                                                if(SDDRead
                                                                    (finalblock
                                                                        -
                                                                        (blmask
                                                                            +
                                                                            1),
                                                                        1 <<
                                                                        card->
                                                                        CurrentBLen,
                                                                        endbuffer,
                                                                        card) !=
                                                                    blmask +
                                                                    1)  {
                                                                    simpfree
                                                                    (endbuffer);
                                                                    if
                                                                    (startbuffer)
                                                                    simpfree
                                                                    (startbuffer);
                                                                    halFlags &=
                                                                    ~HAL_NOCLOCKCHANGE;
                                                                    return 0;
                                                                    }
                                                                }
                                                                
                                                                else
                                                                endbuffer =
                                                                startbuffer; }
       restartall:                                            startaddr = SDAddr & (~blmask);     *SDIDSTA = *SDIDSTA & (~3); // RESET ALL BITS
                                                                SDDResetFIFO();
                                                                  *SDIDCON =
                                                                0x23000 | blocks
                                                                | card->
                                                                BusWidth; 
//printf("blocks=%d\n",blocks);
                                                                if
                                                                (!SDSendCmdShortResp
                                                                    ((blocks ==
                                                                            1) ?
                                                                        24 : 25,
                                                                        (int)(
                                                                            (card->
                                                                                SysFlags
                                                                                &
                                                                                16)
                                                                            ?
                                                                            (startaddr
                                                                                >>
                                                                                9)
                                                                            :
                                                                            startaddr),
                                                                        &status))
                                                                 { 
//printf("failed read cmd\n");
                                                                if
                                                                    (startbuffer)
                                                                    simpfree
                                                                    (startbuffer);
                                                                    if
                                                                    (endbuffer
                                                                        &&
                                                                        endbuffer
                                                                        !=
                                                                        startbuffer)
                                                                    simpfree
                                                                    (endbuffer);
                                                                    halFlags
                                                                    &=
                                                                    ~HAL_NOCLOCKCHANGE;
                                                                     return
                                                                    FALSE; }
                                                                 
//printf("cmd WRITE started\n");
                                                                
//status=*SDIDSTA;
//*SDIDSTA=status&(~3);         // RESET ALL BITS
                                                                
// OBSOLETE - NEVER HAPPENS
                                                                
                                                                /*
                                                                   printf("status=%04X\n",status);
                                                                   keyb_getkeyM(1);
                                                                   if(status&0x20) {                // CHECK FOR TIMEOUT
                                                                   printf("Timeout!\n");
                                                                   if(!SDSlowDown()) {
                                                                   printf("Slowdown failed\n");
                                                                   return FALSE;
                                                                   }
                                                                   else continue;
                                                                   SDDStop();
                                                                   continue;
                                                                   
                                                                   }
                                                                 */ 
                                                                 
//printf("First loop done\n");
                                                                if(((unsigned
                                                                            int)
                                                                        SDAddr |
                                                                        (unsigned
                                                                            int)
                                                                        endaddr
                                                                        |
                                                                        (unsigned
                                                                            int)
                                                                        buffer)
                                                                    & 3)  { 
// SLOW BYTE-ALIGNED LOOP
//printf("Write Byte loop active\n");
                                                                    while
                                                                    (startaddr <
                                                                        finalblock)
                                                                     {
//printf("2nd loop\n");
// START RECEIVING DATA
                                                                            while
                                                                            (!(*SDIFSTA & 0x2000))  {if(!SDCardInserted())  {
//printf("no card\n");
                                                                                    SDDStop
                                                                                            ();
                                                                                            if
                                                                                            (startbuffer)
                                                                                            simpfree
                                                                                            (startbuffer);
                                                                                            if
                                                                                            (endbuffer
                                                                                                &&
                                                                                                endbuffer
                                                                                                !=
                                                                                                startbuffer)
                                                                                            simpfree
                                                                                            (endbuffer);
                                                                                            halFlags
                                                                                            &=
                                                                                            ~HAL_NOCLOCKCHANGE;
                                                                                             return
                                                                                            FALSE;
                                                                                            }
                                                                                    status
                                                                                    =
                                                                                    *SDIDSTA;
                                                                                    
//printf("pt2\n");
                                                                                    if
                                                                                    ((status & 0x1a4) || !(status & 2))  { 
// ANY ERROR
//printf("Data error=%04X\n",status);
                                                                                        SDDStop
                                                                                        ();
                                                                                        if
                                                                                        (status
                                                                                            &
                                                                                            0x20)
                                                                                        
                                                                                        { 
//printf("Timeout!\n");
                                                                                            if
                                                                                            (!SDSlowDown
                                                                                                ())
                                                                                            
                                                                                            {
//printf("Slowdown failed\n");
                                                                                                if
                                                                                                    (startbuffer)
                                                                                                    simpfree
                                                                                                    (startbuffer);
                                                                                                    if
                                                                                                    (endbuffer
                                                                                                        &&
                                                                                                        endbuffer
                                                                                                        !=
                                                                                                        startbuffer)
                                                                                                    simpfree
                                                                                                    (endbuffer);
                                                                                                    halFlags
                                                                                                    &=
                                                                                                    ~HAL_NOCLOCKCHANGE;
                                                                                                    return
                                                                                                    FALSE;
                                                                                                    }
                                                                                            while
                                                                                                (
                                                                                                    (*SDIDSTA
                                                                                                        &
                                                                                                        0x3));
                                                                                                goto
                                                                                                restartall;
                                                                                                 }
                                                                                        *SDIDSTA
                                                                                            =
                                                                                            status
                                                                                            &
                                                                                            (~3);
                                                                                            if
                                                                                            (startbuffer)
                                                                                            simpfree
                                                                                            (startbuffer);
                                                                                            if
                                                                                            (endbuffer
                                                                                                &&
                                                                                                endbuffer
                                                                                                !=
                                                                                                startbuffer)
                                                                                            simpfree
                                                                                            (endbuffer);
                                                                                            halFlags
                                                                                            &=
                                                                                            ~HAL_NOCLOCKCHANGE;
                                                                                            return
                                                                                            FALSE;
                                                                                            }
                                                                                         }
                                                                                         if
                                                                                        (startaddr
                                                                                            <
                                                                                            SDAddr)
                                                                                        
                                                                                        { *((unsigned char *)SDIDAT) = startbuffer[startaddr & blmask]; } 
                                                                                        else
                                                                                         {
                                                                                            if
                                                                                            (startaddr
                                                                                                >=
                                                                                                endaddr)
                                                                                            
                                                                                            { *((unsigned char *)SDIDAT) = endbuffer[startaddr & blmask]; } 
                                                                                            else
                                                                                             {
                                                                                *
                                                                                                (
                                                                                                    (unsigned
                                                                                                        char
                                                                                                        *)
                                                                                                    SDIDAT)
                                                                                                =
                                                                                                *buffer;
                                                                                                ++buffer;
                                                                                                }
                                                                                            }
                                                                                        startaddr++;
                                                                                         }
                                                                                     }
                                                                                
                                                                                else
                                                                                 {
                                                                                     
//printf("write word loop active\n");
                                                                                    while
                                                                                    (startaddr
                                                                                        <
                                                                                        finalblock)
                                                                                    
                                                                                    { 
//printf("2nd loop\n");
// START RECEIVING DATA
                                                                                        while
                                                                                        (!(*SDIFSTA & 0x2000))  {if(!SDCardInserted())  {
//printf("no card\n");
                                                                                                SDDStop
                                                                                                        ();
                                                                                                        if
                                                                                                        (startbuffer)
                                                                                                        simpfree
                                                                                                        (startbuffer);
                                                                                                        if
                                                                                                        (endbuffer
                                                                                                            &&
                                                                                                            endbuffer
                                                                                                            !=
                                                                                                            startbuffer)
                                                                                                        simpfree
                                                                                                        (endbuffer);
                                                                                                        halFlags
                                                                                                        &=
                                                                                                        ~HAL_NOCLOCKCHANGE;
                                                                                                         return
                                                                                                        FALSE;
                                                                                                        }
                                                                                                status
                                                                                                =
                                                                                                *SDIDSTA;
                                                                                                
//printf("pt2\n");
                                                                                                if
                                                                                                ((status & 0x1a4) || !(status & 2))  { 
// ANY ERROR
//printf("Data error=%04X\n",status);
                                                                                                    SDDStop
                                                                                                    ();
                                                                                                    if
                                                                                                    (status
                                                                                                        &
                                                                                                        0x20)
                                                                                                    
                                                                                                    { 
//printf("Timeout!\n");
                                                                                                        if
                                                                                                        (!SDSlowDown
                                                                                                            ())
                                                                                                        
                                                                                                        {
//printf("Slowdown failed\n");
                                                                                                            if
                                                                                                                (startbuffer)
                                                                                                                simpfree
                                                                                                                (startbuffer);
                                                                                                                if
                                                                                                                (endbuffer
                                                                                                                    &&
                                                                                                                    endbuffer
                                                                                                                    !=
                                                                                                                    startbuffer)
                                                                                                                simpfree
                                                                                                                (endbuffer);
                                                                                                                halFlags
                                                                                                                &=
                                                                                                                ~HAL_NOCLOCKCHANGE;
                                                                                                                 return
                                                                                                                FALSE;
                                                                                                                }
                                                                                                        while
                                                                                                            (
                                                                                                                (*SDIDSTA
                                                                                                                    &
                                                                                                                    0x3));
                                                                                                            goto
                                                                                                            restartall;
                                                                                                             }
                                                                                                    *SDIDSTA
                                                                                                        =
                                                                                                        status
                                                                                                        &
                                                                                                        (~3);
                                                                                                        if
                                                                                                        (startbuffer)
                                                                                                        simpfree
                                                                                                        (startbuffer);
                                                                                                        if
                                                                                                        (endbuffer
                                                                                                            &&
                                                                                                            endbuffer
                                                                                                            !=
                                                                                                            startbuffer)
                                                                                                        simpfree
                                                                                                        (endbuffer);
                                                                                                        halFlags
                                                                                                        &=
                                                                                                        ~HAL_NOCLOCKCHANGE;
                                                                                                         return
                                                                                                        FALSE;
                                                                                                        }
                                                                                                     }
                                                                                                     if
                                                                                                    (startaddr
                                                                                                        <
                                                                                                        SDAddr)
                                                                                                    
                                                                                                    { *SDIDAT = *(unsigned int *)(startbuffer + (startaddr & blmask)); } 
                                                                                                    else
                                                                                                     {
                                                                                                        if
                                                                                                        (startaddr
                                                                                                            >=
                                                                                                            endaddr)
                                                                                                        
                                                                                                        { *SDIDAT = *(unsigned int *)(endbuffer + (startaddr & blmask)); } 
                                                                                                        else
                                                                                                         {
                                                                                            *SDIDAT
                                                                                                            =
                                                                                                            *
                                                                                                            (
                                                                                                                (unsigned
                                                                                                                    int
                                                                                                                    *)
                                                                                                                buffer);
                                                                                                            buffer
                                                                                                            +=
                                                                                                            4;
                                                                                                            }
                                                                                                        }
                                                                                                    startaddr
                                                                                                    +=
                                                                                                    4;
                                                                                                     }
                                                                                                   }
                                                                                               if
                                                                                            (startbuffer)
                                                                                            simpfree
                                                                                            (startbuffer);
                                                                                            if
                                                                                            (endbuffer
                                                                                                &&
                                                                                                endbuffer
                                                                                                !=
                                                                                                startbuffer)
                                                                                            simpfree
                                                                                            (endbuffer);
                                                                                              
// WAIT UNTIL END OF TRANSMISSION
                                                                                            do
                                                                                                 {
                                                                                                status
                                                                                                =
                                                                                                *SDIDSTA;
                                                                                                if
                                                                                                (status
                                                                                                    &
                                                                                                    0x124)
                                                                                                
                                                                                                { 
// ANY ERROR
//printf("Data error END=%04X\n",status);
                                                                                                    SDDStop
                                                                                                    ();
                                                                                                    if
                                                                                                    (status
                                                                                                        &
                                                                                                        0x20)
                                                                                                    
                                                                                                    { 
//printf("Timeout!\n");
                                                                                                        if
                                                                                                        (!SDSlowDown
                                                                                                            ())
                                                                                                        
                                                                                                        {
//printf("Slowdown failed\n");
                                                                                                            halFlags
                                                                                                                &=
                                                                                                                ~HAL_NOCLOCKCHANGE;
                                                                                                                 return
                                                                                                                FALSE;
                                                                                                                }
                                                                                                        while
                                                                                                            (
                                                                                                                (*SDIDSTA
                                                                                                                    &
                                                                                                                    0x3));
                                                                                                            goto
                                                                                                            restartall;
                                                                                                             }
                                                                                                     }
                                                                                                    }
                                                                                                while
                                                                                                (!(status & 0x10));   
//printf("DSTA=%04X\n",*SDIDSTA);
//printf("FSTA=%04X\n",*SDIFSTA);
                                                                                                  if
                                                                                                (blocks
                                                                                                    >
                                                                                                    1)
                                                                                                
                                                                                            { SDDStop(); 
//printf("DSTA=%04X\n",*SDIDSTA);
//printf("bytes=%d\n",startaddr-SDAddr);
//printf("st-e=%d\n",endaddr-startaddr);
                                                                                                }
                                                                                                  *SDIDSTA
                                                                                                =
                                                                                                *SDIDSTA
                                                                                                &
                                                                                                ~3;
                                                                                                 
// CHECK FOR BUSY SIGNAL
                                                                                                *SDIDCON
                                                                                                =
                                                                                                0x1000;
                                                                                                 
// WAIT UNTIL BUSY DETECT STARTS
//while( !(*SDIDSTA&3) ) { printf("u %04X\n",*SDIDSTA); }
                                                                                                
// CHECK FOR BUSY FINISH
                                                                                                while
                                                                                                (!(status = *SDIDSTA & 0x2C));  
//printf("busy finished %04X\n",status);
                                                                                                *SDIDCON = 0x4000;  *SDIDSTA = 0x7fc;        // RESET ALL BITS
                                                                                                halFlags &= ~HAL_NOCLOCKCHANGE;  return NumBytes;     // RETURN NUMBER OF BYTES TRANSMITTED
                                                                                                }    
#endif  /*  */
