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

    u32 addr = ARM11_GetReg(ARM11, rn);
    u32 offset;
    if (!i) offset = curinstr & 0xFFF;
    else
    {
        const u8 shifttype = ((curinstr >> 5) & 0x3);
        u8 shiftimm = ((curinstr >> 7) & 0x1F);
        offset = ARM11_GetReg(ARM11, curinstr & 0xF);

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
    u32 writeback;

    if (!u)
    {
        writeback = addr - offset;
        if (p) addr -= offset;
    }
    else
    {
        writeback = addr + offset;
        if (p) addr += offset;
    }

    if (!p && w)
        ; // todo: translate

    if (b)
    {
        if (l) ARM11_WriteReg(ARM11, rd, Bus11_Load8(ARM11, addr), false); // ldrb
        else Bus11_Store8(ARM11, addr, ARM11_GetReg(ARM11, rd)); // strb
    }
    else
    {
        if (l) ARM11_WriteReg(ARM11, rd, Bus11_Load32(ARM11, addr), false); // ldr
        else Bus11_Store32(ARM11, addr, ARM11_GetReg(ARM11, rd)); // str
    }

    if (w || !p)
    {
        ARM11_WriteReg(ARM11, rn, writeback, false);
    }
}

void ARM11_LDM_STM(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;
    const bool p = curinstr & (1<<24);
    const bool u = curinstr & (1<<23);
    const bool s = curinstr & (1<<22);
    const bool w = curinstr & (1<<21);
    const bool l = curinstr & (1<<20);
    const u32 rn = ARM11_GetReg(ARM11, (curinstr >> 16) & 0xF);
    const int r15 = curinstr & (1<<15);
    u16 rlist = curinstr & 0xFFFF;
    const int rcount = __builtin_popcount(rlist);
    u32 wbbase;
    u32 base;

    if (!u) wbbase = base = rn - (rcount*4); // checkme: does this actually decrement now?
    else
    {
        base = rn;
        wbbase = rn + (rcount*4);
    }

    if (l && s && ! r15) printf("WARNING: USER MODE LDM UNIMPLEMENTED\n"); // todo: user mode regs

    while (rlist)
    {
        printf("%04X\n", rlist);
        int reg = __builtin_ctz(rlist);
        rlist &= ~1<<reg;

        if (p^u) base += 4;

        if (l) ARM11_WriteReg(ARM11, reg, Bus11_Load32(ARM11, base), s);
        else Bus11_Store32(ARM11, base, ARM11_GetReg(ARM11, reg)); // todo: stores

        if (!(p^u)) base += 4;
    }

    if (w)
    {
        ARM11_WriteReg(ARM11, (curinstr >> 16) & 0xF, wbbase, false);
    }
}
