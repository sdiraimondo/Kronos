/*  Copyright 2003-2005 Guillaume Duhamel
    Copyright 2004-2006, 2013 Theo Berkau

    This file is part of Yabause.

    Yabause is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Yabause is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yabause; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef SH2CORE_H
#define SH2CORE_H

#include "core.h"
#include "threads.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SH2CORE_DEFAULT     -1
#define MAX_INTERRUPTS 50

#ifdef MACH
#undef MACH
#endif

//#define DMPHISTORY
#define MAX_DMPHISTORY (512)

// UBC Flags
#define BBR_CPA_NONE			(0 << 6)
#define BBR_CPA_CPU				(1 << 6)
#define BBR_CPA_PER				(2 << 6)

#define BBR_IDA_NONE			(0 << 4)
#define BBR_IDA_INST	 		(1 << 4)
#define BBR_IDA_DATA			(2 << 4)

#define BBR_RWA_NONE			(0 << 2)
#define BBR_RWA_READ	 		(1 << 2)
#define BBR_RWA_WRITE			(2 << 2)

#define BBR_SZA_NONE			(0 << 0)
#define BBR_SZA_BYTE	 		(1 << 0)
#define BBR_SZA_WORD			(2 << 0)
#define BBR_SZA_LONGWORD	 	(3 << 0)

#define BRCR_CMFCA				(1 << 15)
#define BRCR_CMFPA				(1 << 14)
#define BRCR_EBBA				(1 << 13)
#define BRCR_UMD				(1 << 12)
#define BRCR_PCBA				(1 << 10)

#define BRCR_CMFCB				(1 << 7)
#define BRCR_CMFPB				(1 << 6)
#define BRCR_SEQ				(1 << 4)
#define BRCR_DBEB				(1 << 3)
#define BRCR_PCBB				(1 << 2)

typedef struct
{
   u32 R[16];

#ifdef WORDS_BIGENDIAN
  union {
    struct {
      u32 reserved1:22;
      u32 M:1;
      u32 Q:1;
      u32 I:4;
      u32 reserved0:2;
      u32 S:1;
      u32 T:1;
    } part;
    u32 all;
  } SR;
#else
  union {
    struct {
      u32 T:1;
      u32 S:1;
      u32 reserved0:2;
      u32 I:4;
      u32 Q:1;
      u32 M:1;
      u32 reserved1:22;
    } part;
    u32 all;
  } SR;
#endif

   u32 GBR;
   u32 VBR;
   u32 MACH;
   u32 MACL;
   u32 PR;
   u32 PC;
} sh2regs_struct;

typedef struct
{
   u8 SMR;     // 0xFFFFFE00
   u8 BRR;     // 0xFFFFFE01
   u8 SCR;     // 0xFFFFFE02
   u8 TDR;     // 0xFFFFFE03
   u8 SSR;     // 0xFFFFFE04
   u8 RDR;     // 0xFFFFFE05
   u8 TIER;    // 0xFFFFFE10
   u8 FTCSR;   // 0xFFFFFE11

#ifdef WORDS_BIGENDIAN
  union {
    struct {
      u16 H:8; // 0xFFFFFE12
      u16 L:8; // 0xFFFFFE13
    } part;
    u16 all;
  } FRC;
#else
  union {
    struct {
      u16 L:8; // 0xFFFFFE13
      u16 H:8; // 0xFFFFFE12
    } part;
    u16 all;
  } FRC;
#endif
   u16 OCRA;   // 0xFFFFFE14/0xFFFFFE15
   u16 OCRB;   // 0xFFFFFE14/0xFFFFFE15
   u8 TCR;     // 0xFFFFFE16
   u8 TOCR;    // 0xFFFFFE17
   u16 FICR;   // 0xFFFFFE18
               // 0xFFFFFE19
   u16 IPRB;   // 0xFFFFFE60
   u16 VCRA;   // 0xFFFFFE62
   u16 VCRB;   // 0xFFFFFE64
   u16 VCRC;   // 0xFFFFFE66
   u16 VCRD;   // 0xFFFFFE68
   u8 DRCR0;   // 0xFFFFFE71
   u8 DRCR1;   // 0xFFFFFE72
   u8 WTCSR;   // 0xFFFFFE80
   u32 WTCSRM;   // 0xFFFFFE80 mirror
   u8 WTCNT;   // 0xFFFFFE81
   u8 RSTCSR;  // 0xFFFFFE83
   u8 SBYCR;   // 0xFFFFFE91
   u8 CCR;     // 0xFFFFFE92
   u16 ICR;    // 0xFFFFFEE0
   u16 IPRA;   // 0xFFFFFEE2
   u16 VCRWDT; // 0xFFFFFEE4
   u32 DVSR;   // 0xFFFFFF00
   u32 DVDNT;  // 0xFFFFFF04
   u32 DVCR;   // 0xFFFFFF08
   u32 VCRDIV; // 0xFFFFFF0C
   u32 DVDNTH; // 0xFFFFFF10
   u32 DVDNTL; // 0xFFFFFF14
   u32 DVDNTUH; // 0xFFFFFF18
   u32 DVDNTUL; // 0xFFFFFF1C
#ifdef WORDS_BIGENDIAN
  union {
    struct {
      u32 H:16; // 0xFFFFFF40
      u32 L:16; // 0xFFFFFF42
    } part;
    u16 all;
  } BARA;

  union {
    struct {
      u32 H:16; // 0xFFFFFF44
      u32 L:16; // 0xFFFFFF46
    } part;
    u16 all;
  } BAMRA;
#else
  union {
    struct {
      u32 L:16; // 0xFFFFFF42
      u32 H:16; // 0xFFFFFF40
    } part;
    u16 all;
  } BARA;

  union {
    struct {
      u32 L:16; // 0xFFFFFF46
      u32 H:16; // 0xFFFFFF44
    } part;
    u16 all;
  } BAMRA;
#endif
   u32 BBRA;   // 0xFFFFFF48
#ifdef WORDS_BIGENDIAN
  union {
    struct {
      u32 H:16; // 0xFFFFFF60
      u32 L:16; // 0xFFFFFF62
    } part;
    u16 all;
  } BARB;

  union {
    struct {
      u32 H:16; // 0xFFFFFF64
      u32 L:16; // 0xFFFFFF66
    } part;
    u16 all;
  } BAMRB;
#else
  union {
    struct {
      u32 L:16; // 0xFFFFFF62
      u32 H:16; // 0xFFFFFF60
    } part;
    u16 all;
  } BARB;

  union {
    struct {
      u32 L:16; // 0xFFFFFF66
      u32 H:16; // 0xFFFFFF64
    } part;
    u16 all;
  } BAMRB;
#endif
   u32 BBRB;   // 0xFFFFFF68
#ifdef WORDS_BIGENDIAN
  union {
    struct {
      u32 H:16; // 0xFFFFFF70
      u32 L:16; // 0xFFFFFF72
    } part;
    u16 all;
  } BDRB;

  union {
    struct {
      u32 H:16; // 0xFFFFFF74
      u32 L:16; // 0xFFFFFF76
    } part;
    u16 all;
  } BDMRB;
#else
  union {
    struct {
      u32 L:16; // 0xFFFFFF72
      u32 H:16; // 0xFFFFFF70
    } part;
    u16 all;
  } BDRB;

  union {
    struct {
      u32 L:16; // 0xFFFFFF76
      u32 H:16; // 0xFFFFFF74
    } part;
    u16 all;
  } BDMRB;
#endif
   u32 BRCR;   // 0xFFFFFF78
   u32 SAR0;   // 0xFFFFFF80
   u32 DAR0;   // 0xFFFFFF84
   u32 TCR0;   // 0xFFFFFF88
   u32 CHCR0;  // 0xFFFFFF8C
   u32 SAR1;   // 0xFFFFFF90
   u32 DAR1;   // 0xFFFFFF94
   u32 TCR1;   // 0xFFFFFF98
   u32 CHCR1;  // 0xFFFFFF9C
   u32 CHCR1M;
   u32 VCRDMA0;// 0xFFFFFFA0
   u32 VCRDMA1;// 0xFFFFFFA8
   u32 DMAOR;  // 0xFFFFFFB0
   u16 BCR1;   // 0xFFFFFFE0
   u16 BCR2;   // 0xFFFFFFE4
   u16 WCR;    // 0xFFFFFFE8
   u16 MCR;    // 0xFFFFFFEC
   u16 RTCSR;  // 0xFFFFFFF0
   u16 RTCNT;  // 0xFFFFFFF4
   u16 RTCOR;  // 0xFFFFFFF8
} Onchip_struct;

typedef struct
{
   u8 vector;
   u8 level;
} interrupt_struct;

enum SH2STEPTYPE
{
   SH2ST_STEPOVER,
   SH2ST_STEPOUT
};

typedef struct
{
   u32 addr;
   u64 count;
} tilInfo_struct;

typedef struct SH2_struct_s
{
   sh2regs_struct regs;
   Onchip_struct onchip;

   struct
   {
      u32 leftover;
      u32 shift;
   } frc;

   struct
   {
        int isenable;
        int isinterval;
        u32 leftover;
        u32 shift;
   } wdt;

   interrupt_struct interrupts[MAX_INTERRUPTS];
   u32 NumberOfInterrupts;
   u32 AddressArray[0x100];
   u8 DataArray[0x1000];
   u32 delay;
   u32 cycles;
   u8 isslave;
   u8 isSleeping;
   u16 instruction;
   u16 isInIt;
   int depth;

#ifdef DMPHISTORY
   u32 pchistory[MAX_DMPHISTORY];
   sh2regs_struct regshistory[MAX_DMPHISTORY];
   u32 pchistory_index;
#endif

   void * ext;

#ifdef USE_CACHE
   u8 cacheOn;
   u8 nbCacheWay;
   u8 cacheLRU[64];
   u8 cacheData[64][4][16];
   u8 tagWay[64][0x80000];
   u32 cacheTagArray[64][4];
#endif
   u32 cycleFrac;
   u32 cycleLost;
   int cdiff;
   int trace;
#ifdef xSH2_ASYNC
   int cycles_request;
   int thread_running;
   YabEventQueue *start;
   YabEventQueue *end;
   sem_t lock;
   int thread_id;
#endif
    u32 frtcycles;
    u32 wdtcycles;
} SH2_struct;

typedef struct
{
   int id;
   const char *Name;

   int (*Init)(void);
   void (*DeInit)(void);
   void (*Reset)(SH2_struct *context);
   void FASTCALL (*Exec)(SH2_struct *context, u32 cycles);
   void FASTCALL (*TestExec)(SH2_struct *context, u32 cycles);

   void (*GetRegisters)(SH2_struct *context, sh2regs_struct *regs);
   u32 (*GetGPR)(SH2_struct *context, int num);
   u32 (*GetSR)(SH2_struct *context);
   u32 (*GetGBR)(SH2_struct *context);
   u32 (*GetVBR)(SH2_struct *context);
   u32 (*GetMACH)(SH2_struct *context);
   u32 (*GetMACL)(SH2_struct *context);
   u32 (*GetPR)(SH2_struct *context);
   u32 (*GetPC)(SH2_struct *context);

   void (*SetRegisters)(SH2_struct *context, const sh2regs_struct *regs);
   void (*SetGPR)(SH2_struct *context, int num, u32 value);
   void (*SetSR)(SH2_struct *context, u32 value);
   void (*SetGBR)(SH2_struct *context, u32 value);
   void (*SetVBR)(SH2_struct *context, u32 value);
   void (*SetMACH)(SH2_struct *context, u32 value);
   void (*SetMACL)(SH2_struct *context, u32 value);
   void (*SetPR)(SH2_struct *context, u32 value);
   void (*SetPC)(SH2_struct *context, u32 value);
   void (*OnFrame)(SH2_struct *context);
   void (*SendInterrupt)(SH2_struct *context, u8 vector, u8 level);
   void (*RemoveInterrupt)(SH2_struct *context, u8 vector, u8 level);
   int (*GetInterrupts)(SH2_struct *context,
                        interrupt_struct interrupts[MAX_INTERRUPTS]);
   void (*SetInterrupts)(SH2_struct *context, int num_interrupts,
                         const interrupt_struct interrupts[MAX_INTERRUPTS]);

   void (*WriteNotify)(u32 start, u32 length);
   void(*AddCycle)(SH2_struct *context, u32 value);
} SH2Interface_struct;

extern SH2_struct *MSH2;
extern SH2_struct *SSH2;
extern SH2Interface_struct *SH2Core;

int SH2Init(int coreid);
void SH2DeInit(void);
void SH2Reset(SH2_struct *context);
void SH2PowerOn(SH2_struct *context);
void FASTCALL SH2Exec(SH2_struct *context, u32 cycles);
void FASTCALL SH2TestExec(SH2_struct *context, u32 cycles);
void SH2SendInterrupt(SH2_struct *context, u8 vector, u8 level);
void SH2RemoveInterrupt(SH2_struct *context, u8 vector, u8 level);
void SH2NMI(SH2_struct *context);

void SH2GetRegisters(SH2_struct *context, sh2regs_struct * r);
void SH2SetRegisters(SH2_struct *context, sh2regs_struct * r);
void SH2WriteNotify(SH2_struct *context, u32 start, u32 length);

void SH2Disasm(u32 v_addr, u16 op, int mode, sh2regs_struct *r, char *string);
void SH2DumpHistory(SH2_struct *context);


int BackupHandled(SH2_struct * sh, u32 addr);
int isBackupHandled(u32 addr);

#ifdef USE_CACHE
u8 CacheReadByte(SH2_struct *context, u8* mem, u32 addr);
u16 CacheReadWord(SH2_struct *context, u8* mem, u32 addr);
u32 CacheReadLong(SH2_struct *context, u8* mem, u32 addr);
void CacheWriteByte(SH2_struct *context, u8* mem, u32 addr, u8 val);
void CacheWriteShort(SH2_struct *context, u8* mem, u32 addr, u16 val);
void CacheWriteLong(SH2_struct *context, u8* mem, u32 addr, u32 val);
#endif
void CacheInvalidate(SH2_struct *context,u32 addr);

void DMAExec(SH2_struct *context);
void DMATransfer(SH2_struct *context, u32 *CHCR, u32 *SAR, u32 *DAR, u32 *TCR, u32 *VCRDMA);

u8 FASTCALL OnchipReadByte(SH2_struct *context, u32 addr);
u16 FASTCALL OnchipReadWord(SH2_struct *context, u32 addr);
u32 FASTCALL OnchipReadLong(SH2_struct *context, u32 addr);
void FASTCALL OnchipWriteByte(SH2_struct *context, u32 addr, u8 val);
void FASTCALL OnchipWriteWord(SH2_struct *context, u32 addr, u16 val);
void FASTCALL OnchipWriteLong(SH2_struct *context, u32 addr, u32 val);

u32 FASTCALL AddressArrayReadLong(SH2_struct *context, u32 addr);
void FASTCALL AddressArrayWriteLong(SH2_struct *context, u32 addr, u32 val);

u8 FASTCALL DataArrayReadByte(SH2_struct *context, u32 addr);
u16 FASTCALL DataArrayReadWord(SH2_struct *context, u32 addr);
u32 FASTCALL DataArrayReadLong(SH2_struct *context, u32 addr);
void FASTCALL DataArrayWriteByte(SH2_struct *context, u32 addr, u8 val);
void FASTCALL DataArrayWriteWord(SH2_struct *context, u32 addr, u16 val);
void FASTCALL DataArrayWriteLong(SH2_struct *context, u32 addr, u32 val);

void FASTCALL MSH2InputCaptureWriteWord(SH2_struct *context, UNUSED u8* mem, u32 addr, u16 data);
void FASTCALL SSH2InputCaptureWriteWord(SH2_struct *context, UNUSED u8* mem, u32 addr, u16 data);

int SH2SaveState(SH2_struct *context, FILE *fp);
int SH2LoadState(SH2_struct *context, FILE *fp, int version, int size);

extern SH2Interface_struct SH2Dyn;
extern SH2Interface_struct SH2DynDebug;

#if DYNAREC_KRONOS
extern SH2Interface_struct SH2KronosInterpreter;
#endif

#ifdef __cplusplus
}
#endif

#endif
