#include <stdio.h>
#include "interpreter.h"
#include "../../bus.h"

void ARM11_LDR_STR(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;
    const bool i = curinstr & (1<<25);
    const bool p = curinstr & (1<<24);
    const bool u = curinstr & (1<<23);
    const bool b = curinstr & (1<<22);
    const bool w = curinstr & (1<<21);
    const bool l = curinstr & (1<<20);
    const int rn = (curinstr >> 16) & 0xF;
    const int rd = (curinstr >> 12) & 0xF;

    u32 addr = ARM11->R[rn];
    u32 offset;
    if (i) offset = curinstr & 0xFFF;
    else
    {
        const u8 shifttype = ((curinstr >> 5) & 0x3);
        u8 shiftimm = ((curinstr >> 7) & 0x1F);
        offset = ARM11->R[curinstr & 0xF];

        switch(shifttype)
        {
        case 0: // lsl
        {
            offset <<= shiftimm;
            break;
        }
        case 1: // lsr
        {
            if (!shiftimm) shiftimm = 32;
            offset >>= shiftimm;
            break;
        }
        case 2: // asr
        {
            if (!shiftimm) shiftimm = 32;
            offset = (s32)offset >> shiftimm;
            break;
        }
        case 3: // ror/rrx
        {
            if (shiftimm) // ror
            {
                offset = ARM11_ROR32(offset, shiftimm);
            }
            else
            {
                offset = (ARM11->Carry << 31) | (offset >> 1);
            }
            break;
        }
        }
    }
    if (!u) offset = -offset;
    const u32 writeback = addr + offset;
    if (p) addr += offset;
    else if (w)
        ; // todo: translate

    if (b)
    {
        if (l) // ldrb
        {
            ARM11_WriteReg(ARM11, rd, *Bus_GetPtr(addr), false);
        }
        else printf("WARNING: STRB UNIMPLEMENTED\n"); // strb
    }
    else
    {
        if (l) // ldr
        {
            ARM11_WriteReg(ARM11, rd, *(u32*)Bus_GetPtr(addr), false);
        }
        else printf("WARNING: STR UNIMPLEMENTED\n"); // str
    }

    if (w || !p)
    {
        ARM11_WriteReg(ARM11, rn, writeback, false);
    }
}
