/*  Copyright 2020 Francois Caron

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
#include "vdp1_interface.h"
#include "vdp1.h"
#include "yabause.h"
#include <stdlib.h>

extern Vdp1 * Vdp1Regs;
YabEventQueue *vdp1_q;

#define LOG
#define FRAMELOG

//////////////////////////////////////////////////////////////////////////////

void FASTCALL Vdp1FrameBufferWriteByte(SH2_struct *context, u8* mem, u32 addr, u8 val) {
   addr &= 0x7FFFF;
   vdp1Command_struct *p = (vdp1Command_struct*)malloc(sizeof(vdp1Command_struct));
   p->cmd = VDP1FB_WRITE;
   p->msg = malloc(3*sizeof(int));
   ((int*)(p->msg))[0]=0; //Byte
   ((int*)(p->msg))[1]=addr; //Addr
   ((int*)(p->msg))[2]=val; //Val
   YabAddEventQueue(vdp1_q,p);
}

//////////////////////////////////////////////////////////////////////////////

void FASTCALL Vdp1FrameBufferWriteWord(SH2_struct *context, u8* mem, u32 addr, u16 val) {
  addr &= 0x7FFFF;
  vdp1Command_struct *p = (vdp1Command_struct*)malloc(sizeof(vdp1Command_struct));
  p->cmd = VDP1FB_WRITE;
  p->msg = malloc(3*sizeof(int));
  ((int*)(p->msg))[0]=1; //Byte
  ((int*)(p->msg))[1]=addr; //Addr
  ((int*)(p->msg))[2]=val; //Val
  YabAddEventQueue(vdp1_q,p);
}

//////////////////////////////////////////////////////////////////////////////

void FASTCALL Vdp1FrameBufferWriteLong(SH2_struct *context, u8* mem, u32 addr, u32 val) {
  addr &= 0x7FFFF;
  vdp1Command_struct *p = (vdp1Command_struct*)malloc(sizeof(vdp1Command_struct));
  p->cmd = VDP1FB_WRITE;
  p->msg = malloc(3*sizeof(int));
  ((int*)(p->msg))[0]=2; //Byte
  ((int*)(p->msg))[1]=addr; //Addr
  ((int*)(p->msg))[2]=val; //Val
  YabAddEventQueue(vdp1_q,p);
}

//////////////////////////////////////////////////////////////////////////////

void FASTCALL Vdp1WriteByte(SH2_struct *context, u8* mem, u32 addr, UNUSED u8 val) {
   addr &= 0xFF;
   LOG("trying to byte-write a Vdp1 register - %08X\n", addr);
}

//////////////////////////////////////////////////////////////////////////////

void FASTCALL Vdp1WriteWord(SH2_struct *context, u8* mem, u32 addr, u16 val) {
  addr &= 0xFF;
  vdp1Command_struct *p = (vdp1Command_struct*)malloc(sizeof(vdp1Command_struct));
  p->msg = malloc(3*sizeof(int));
  ((int*)(p->msg))[0]=1; //Word
  ((int*)(p->msg))[1]=addr; //Addr
  ((int*)(p->msg))[2]=val; //Val
  p->cmd = VDP1REG_UPDATE;
  switch(addr) {
    case 0x0:
      if ((Vdp1Regs->FBCR & 3) != 3) val = (val & (~0x4));
      Vdp1Regs->TVMR = val;
      YabAddEventQueue(vdp1_q,p);
      FRAMELOG("TVMR => Write VBE=%d FCM=%d FCT=%d line = %d\n", (Vdp1Regs->TVMR >> 3) & 0x01, (Vdp1Regs->FBCR & 0x02) >> 1, (Vdp1Regs->FBCR & 0x01),  yabsys.LineCount);
    break;
    case 0x2:
      Vdp1Regs->FBCR = val;
      YabAddEventQueue(vdp1_q,p);
      FRAMELOG("FBCR => Write VBE=%d FCM=%d FCT=%d line = %d\n", (Vdp1Regs->TVMR >> 3) & 0x01, (Vdp1Regs->FBCR & 0x02) >> 1, (Vdp1Regs->FBCR & 0x01),  yabsys.LineCount);
      break;
    case 0x4:
      FRAMELOG("Write PTMR %X line = %d %d\n", val, yabsys.LineCount, yabsys.VBlankLineCount);
      if ((val & 0x3)==0x3) {
        //Skeleton warriors is writing 0xFFF to PTMR. It looks like the behavior is 0x2
          val = 0x2;
      }
      Vdp1Regs->PTMR = val;
      YabAddEventQueue(vdp1_q,p);
      break;
      case 0x6:
         Vdp1Regs->EWDR = val;
         break;
      case 0x8:
         Vdp1Regs->EWLR = val;
         break;
      case 0xA:
         Vdp1Regs->EWRR = val;
         break;
      case 0xC:
         Vdp1Regs->ENDR = val;
         break;
      default:
         LOG("trying to write a Vdp1 read-only register - %08X\n", addr);
   }
}

//////////////////////////////////////////////////////////////////////////////

void FASTCALL Vdp1WriteLong(SH2_struct *context, u8* mem, u32 addr, UNUSED u32 val) {
   addr &= 0xFF;
   LOG("trying to long-write a Vdp1 register - %08X\n", addr);
}

/////////////////////////////////////////////////////////////////////////////

void Vdp1HBlankIN(void)
{
  vdp1Command_struct *p = (vdp1Command_struct*)malloc(sizeof(vdp1Command_struct));
  p->cmd = VDP1_HBLANKIN;
  p->msg = NULL;
  YabAddEventQueue(vdp1_q,p);
}

//////////////////////////////////////////////////////////////////////////////

void Vdp1HBlankOUT(void)
{
  vdp1Command_struct *p = (vdp1Command_struct*)malloc(sizeof(vdp1Command_struct));
  p->cmd = VDP1_HBLANKOUT;
  p->msg = NULL;
  YabAddEventQueue(vdp1_q,p);
}

//////////////////////////////////////////////////////////////////////////////

void Vdp1VBlankIN(void)
{
}

//////////////////////////////////////////////////////////////////////////////

void Vdp1VBlankOUT(void)
{
  vdp1Command_struct *p = (vdp1Command_struct*)malloc(sizeof(vdp1Command_struct));
  p->cmd = VDP1_VBLANKOUT;
  p->msg = NULL;
  YabAddEventQueue(vdp1_q,p);
}
