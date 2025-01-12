#include "interpreter.h"
#include "bus.h"

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

    u8 oldmode;
    if (!p && w) // translate: switch mode to emulate switching to user perms
    {
        oldmode = ARM11->Mode;
        ARM11->Mode = 0x10;
    }

    if (b)
    {
        if (l) ARM11_WriteReg(ARM11, rd, Bus11_Load8(ARM11, addr), false, !ARM11->CP15.ARMv4Thingy); // ldrb
        else Bus11_Store8(ARM11, addr, ARM11_GetReg(ARM11, rd)); // strb
    }
    else
    {
        if (l) ARM11_WriteReg(ARM11, rd, Bus11_Load32(ARM11, addr), false, !ARM11->CP15.ARMv4Thingy); // ldr
        else Bus11_Store32(ARM11, addr, ARM11_GetReg(ARM11, rd)); // str
    }

    if (!p && w) ARM11->Mode = oldmode; // restore old mode

    if (w || !p)
    {
        ARM11_WriteReg(ARM11, rn, writeback, false, false);
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

    if (s && !r15) ARM11_UpdateMode(ARM11, ARM11->Mode, 0x10); // swap to user mode regs

    while (rlist)
    {
        int reg = __builtin_ctz(rlist);
        rlist &= ~1<<reg;

        if (p^u) base += 4;

        if (l) ARM11_WriteReg(ARM11, reg, Bus11_Load32(ARM11, base), s, !ARM11->CP15.ARMv4Thingy);
        else Bus11_Store32(ARM11, base, ARM11_GetReg(ARM11, reg)); // todo: stores

        if (!(p^u)) base += 4;
    }

    if (s && !r15) ARM11_UpdateMode(ARM11, 0x10, ARM11->Mode); // restore actual mode's regs

    if (w)
    {
        ARM11_WriteReg(ARM11, (curinstr >> 16) & 0xF, wbbase, false, false);
    }
}

void THUMB11_LDRPCRel(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const u8 imm8 = curinstr;
    const u8 rd = (curinstr >> 8) & 0x7;

    u32 addr = ARM11_GetReg(ARM11, 15) & ~3;
    addr += imm8 * 4;

    ARM11_WriteReg(ARM11, rd, Bus11_Load32(ARM11, addr), false, false);
}

void THUMB11_LDR_STR_SPRel(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const u8 imm8 = curinstr;
    const u8 rd = (curinstr >> 8) & 0x7;
    const bool l = curinstr & (1<<11);

    u32 addr = ARM11_GetReg(ARM11, 13);
    addr += imm8 * 4;

    if (l) ARM11_WriteReg(ARM11, rd, Bus11_Load32(ARM11, addr), false, false);
    else Bus11_Store32(ARM11, addr, ARM11_GetReg(ARM11, rd));
}

void THUMB11_LDR_STR_Reg(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const u8 rm = (curinstr >> 6) & 0x7;
    const u8 rn = (curinstr >> 3) & 0x7;
    const u8 rd = curinstr & 0x7;
    const u8 opcode = (curinstr >> 9) & 0x7;
    
    const u32 addr = ARM11_GetReg(ARM11, rm) + ARM11_GetReg(ARM11, rn);

    switch(opcode)
    {
    case 0x0: Bus11_Store32(ARM11, addr, ARM11_GetReg(ARM11, rd)); break; // ----------------------------------------- str
    case 0x1: Bus11_Store16(ARM11, addr, ARM11_GetReg(ARM11, rd)); break; // ----------------------------------------- strh
    case 0x2: Bus11_Store8(ARM11, addr, ARM11_GetReg(ARM11, rd)); break; // ------------------------------------------ strb
    case 0x3: ARM11_WriteReg(ARM11, rd, (s32)(s8)Bus11_Load8(ARM11, addr), false, false); break; // - ldrsb
    case 0x4: ARM11_WriteReg(ARM11, rd, Bus11_Load32(ARM11, addr), false, false); break; // --------- ldr
    case 0x5: ARM11_WriteReg(ARM11, rd, Bus11_Load16(ARM11, addr), false, false); break; // --------- ldrh
    case 0x6: ARM11_WriteReg(ARM11, rd, Bus11_Load8(ARM11, addr), false, false); break; // ---------- ldrb
    case 0x7: ARM11_WriteReg(ARM11, rd, (s32)(s16)Bus11_Load16(ARM11, addr), false, false); break; // ldrsh
    }
}

void THUMB11_LDR_STR_Imm5(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const u8 rd = curinstr & 0x7;
    const u8 rn = (curinstr >> 3) & 0x7;
    const u8 imm5 = (curinstr >> 6) & 0x1F;
    const u8 opcode = (curinstr >> 11) & 0x3;

    const u32 addr = ARM11_GetReg(ARM11, rn);

    switch(opcode)
    {
    case 0b00: Bus11_Store32(ARM11, addr + (imm5*4), ARM11_GetReg(ARM11, rd)); break; // ------------------------------- str
    case 0b01: ARM11_WriteReg(ARM11, rd, Bus11_Load32(ARM11, addr + (imm5*4)), false, false); break; // ldr
    case 0b10: Bus11_Store8(ARM11, addr + imm5, ARM11_GetReg(ARM11, rd)); break; // ------------------------------------ strb
    case 0b11: ARM11_WriteReg(ARM11, rd, Bus11_Load8(ARM11, addr + imm5), false, false); break; // ---- ldrb
    }
}

void THUMB11_LDRH_STRH_Imm5(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const u8 rd = curinstr & 0x7;
    const u8 rn = (curinstr >> 3) & 0x7;
    const u8 imm5 = (curinstr >> 6) & 0x1F;
    const u8 l = curinstr & (1<<11);

    const u32 addr = ARM11_GetReg(ARM11, rn) + (imm5 * 2);

    if (l) ARM11_WriteReg(ARM11, rd, Bus11_Load16(ARM11, addr), false, false);
    else Bus11_Store16(ARM11, addr, ARM11_GetReg(ARM11, rd));
}

void THUMB11_PUSH(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    u32 base = ARM11_GetReg(ARM11, 13);
    const int numregs = __builtin_popcount(curinstr & 0x1FF);
    u8 rlist = curinstr;
    const bool r = curinstr & (1<<8);
    const u32 wbbase = base -= (4*numregs);

    while (rlist)
    {
        int reg = __builtin_ctz(rlist);
        rlist &= ~1<<reg;

        Bus11_Store32(ARM11, base, ARM11_GetReg(ARM11, reg));

        base += 4;        
    }

    if (r)
    {
        int reg = 14;

        Bus11_Store32(ARM11, base, ARM11_GetReg(ARM11, reg));

        base += 4;
    }

    ARM11_WriteReg(ARM11, 13, base, false, false);
}

void THUMB11_POP(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    u32 base = ARM11_GetReg(ARM11, 13);
    const int numregs = __builtin_popcount(curinstr & 0x1FF);
    u8 rlist = curinstr;
    const bool r = curinstr & (1<<8);
    const u32 wbbase = base += (4*numregs);

    while (rlist)
    {
        int reg = __builtin_ctz(rlist);
        rlist &= ~1<<reg;

        ARM11_WriteReg(ARM11, reg, Bus11_Load32(ARM11, base), false, false);

        base += 4;    
    }

    if (r)
    {
        int reg = 15;

        ARM11_WriteReg(ARM11, reg, Bus11_Load32(ARM11, base), false, true);

        base += 4;
    }

    ARM11_WriteReg(ARM11, 13, base, false, false);
}
