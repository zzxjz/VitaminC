#include "arm.h"
#include "bus.h"

void ARM9_LDR_STR(struct ARM946E_S* ARM9)
{
    const u32 curinstr = ARM9->Instr[0].Data;
    const bool i = curinstr & (1<<25);
    const bool p = curinstr & (1<<24);
    const bool u = curinstr & (1<<23);
    const bool b = curinstr & (1<<22);
    const bool w = curinstr & (1<<21);
    const bool l = curinstr & (1<<20);
    const int rn = (curinstr >> 16) & 0xF;
    const int rd = (curinstr >> 12) & 0xF;

    u32 addr = ARM9_GetReg(ARM9, rn);
    u32 offset;
    if (!i) offset = curinstr & 0xFFF;
    else
    {
        const u8 shifttype = ((curinstr >> 5) & 0x3);
        u8 shiftimm = ((curinstr >> 7) & 0x1F);
        offset = ARM9_GetReg(ARM9, curinstr & 0xF);

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
                offset = ARM9_ROR32(offset, shiftimm);
            }
            else
            {
                offset = (ARM9->Carry << 31) | (offset >> 1);
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
        oldmode = ARM9->Mode;
        ARM9->Mode = 0x10;
    }

    if (b)
    {
        if (l) ARM9_WriteReg(ARM9, rd, Bus9_Load8(ARM9, addr), false, !ARM9->CP15.Control.TBitLoadDisable); // ldrb
        else Bus9_Store8(ARM9, addr, ARM9_GetReg(ARM9, rd)); // strb
    }
    else
    {
        if (l) ARM9_WriteReg(ARM9, rd, Bus9_Load32(ARM9, addr), false, !ARM9->CP15.Control.TBitLoadDisable); // ldr
        else Bus9_Store32(ARM9, addr, ARM9_GetReg(ARM9, rd)); // str
    }

    if (!p && w) ARM9->Mode = oldmode; // restore old mode

    if (w || !p)
    {
        ARM9_WriteReg(ARM9, rn, writeback, false, false);
    }
}

void ARM9_LDM_STM(struct ARM946E_S* ARM9)
{
    const u32 curinstr = ARM9->Instr[0].Data;
    const bool p = curinstr & (1<<24);
    const bool u = curinstr & (1<<23);
    const bool s = curinstr & (1<<22);
    const bool w = curinstr & (1<<21);
    const bool l = curinstr & (1<<20);
    const u32 rn = ARM9_GetReg(ARM9, (curinstr >> 16) & 0xF);
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

    if (s && !r15) ARM9_UpdateMode(ARM9, ARM9->Mode, 0x10); // swap to user mode regs

    while (rlist)
    {
        int reg = __builtin_ctz(rlist);
        rlist &= ~1<<reg;

        if (p^u) base += 4;

        if (l) ARM9_WriteReg(ARM9, reg, Bus9_Load32(ARM9, base), s, !ARM9->CP15.Control.TBitLoadDisable);
        else Bus9_Store32(ARM9, base, ARM9_GetReg(ARM9, reg));

        if (!(p^u)) base += 4;
    }

    if (s && !r15) ARM9_UpdateMode(ARM9, 0x10, ARM9->Mode); // restore actual mode's regs

    if (w)
    {
        ARM9_WriteReg(ARM9, (curinstr >> 16) & 0xF, wbbase, false, false);
    }
}

void ARM9_PLD(struct ARM946E_S* ARM9)
{
    // TODO: fun fact this literally does nothing but trigger interlocks p sure.
}

void THUMB9_LDRPCRel(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const u8 imm8 = curinstr & 0xFF;
    const int rd = (curinstr >> 8) & 0x7;

    u32 addr = ARM9_GetReg(ARM9, 15) & ~3;
    addr += imm8 * 4;

    ARM9_WriteReg(ARM9, rd, Bus9_Load32(ARM9, addr), false, false);
}

void THUMB9_LDR_STR_SPRel(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const u8 imm8 = curinstr & 0xFF;
    const u8 rd = (curinstr >> 8) & 0x7;
    const bool l = curinstr & (1<<11);

    u32 addr = ARM9_GetReg(ARM9, 13);
    addr += imm8 * 4;

    if (l) ARM9_WriteReg(ARM9, rd, Bus9_Load32(ARM9, addr), false, false);
    else Bus9_Store32(ARM9, addr, ARM9_GetReg(ARM9, rd));
}

void THUMB9_LDR_STR_Reg(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const u8 rm = (curinstr >> 6) & 0x7;
    const u8 rn = (curinstr >> 3) & 0x7;
    const u8 rd = curinstr & 0x7;
    const u8 opcode = (curinstr >> 9) & 0x7;
    
    const u32 addr = ARM9_GetReg(ARM9, rm) + ARM9_GetReg(ARM9, rn);

    switch(opcode)
    {
    case 0x0: Bus9_Store32(ARM9, addr, ARM9_GetReg(ARM9, rd)); break; // ----------------------------------------- str
    case 0x1: Bus9_Store16(ARM9, addr, ARM9_GetReg(ARM9, rd)); break; // ----------------------------------------- strh
    case 0x2: Bus9_Store8(ARM9, addr, ARM9_GetReg(ARM9, rd)); break; // ------------------------------------------ strb
    case 0x3: ARM9_WriteReg(ARM9, rd, (s32)(s8)Bus9_Load8(ARM9, addr), false, false); break; // - ldrsb
    case 0x4: ARM9_WriteReg(ARM9, rd, Bus9_Load32(ARM9, addr), false, false); break; // --------- ldr
    case 0x5: ARM9_WriteReg(ARM9, rd, Bus9_Load16(ARM9, addr), false, false); break; // --------- ldrh
    case 0x6: ARM9_WriteReg(ARM9, rd, Bus9_Load8(ARM9, addr), false, false); break; // ---------- ldrb
    case 0x7: ARM9_WriteReg(ARM9, rd, (s32)(s16)Bus9_Load16(ARM9, addr), false, false); break; // ldrsh
    }
}

void THUMB9_LDR_STR_Imm5(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const int rd = curinstr & 0x7;
    const int rn = (curinstr >> 3) & 0x7;
    const u8 imm5 = (curinstr >> 6) & 0x1F;
    const u8 opcode = (curinstr >> 11) & 0x3;

    const u32 addr = ARM9_GetReg(ARM9, rn);

    switch(opcode)
    {
    case 0b00: Bus9_Store32(ARM9, addr + (imm5*4), ARM9_GetReg(ARM9, rd)); break; // ------------------------------- str
    case 0b01: ARM9_WriteReg(ARM9, rd, Bus9_Load32(ARM9, addr + (imm5*4)), false, false); break; // ldr
    case 0b10: Bus9_Store8(ARM9, addr + imm5, ARM9_GetReg(ARM9, rd)); break; // ------------------------------------ strb
    case 0b11: ARM9_WriteReg(ARM9, rd, Bus9_Load8(ARM9, addr + imm5), false, false); break; // ---- ldrb
    }
}

void THUMB9_LDRH_STRH_Imm5(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const int rd = curinstr & 0x7;
    const int rn = (curinstr >> 3) & 0x7;
    const u8 imm5 = (curinstr >> 6) & 0x1F;
    const bool l = curinstr & (1<<11);

    const u32 addr = ARM9_GetReg(ARM9, rn) + (imm5 * 2);

    if (l) ARM9_WriteReg(ARM9, rd, Bus9_Load16(ARM9, addr), false, false);
    else Bus9_Store16(ARM9, addr, ARM9_GetReg(ARM9, rd));
}

void THUMB9_LDMIA_STMIA(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const int rn = (curinstr >> 8) & 0x7;
    u8 rlist = curinstr & 0xFF;
    const bool l = curinstr & (1<<11);
    u32 base = ARM9_GetReg(ARM9, rn);

    while (rlist)
    {
        int reg = __builtin_ctz(rlist);
        rlist &= ~1<<reg;

        if (l) ARM9_WriteReg(ARM9, reg, Bus9_Load32(ARM9, base), false, false);
        else Bus9_Store32(ARM9, base, ARM9_GetReg(ARM9, reg));

        base += 4;        
    }

    ARM9_WriteReg(ARM9, rn, base, false, false);
}

void THUMB9_PUSH(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    u32 base = ARM9_GetReg(ARM9, 13);
    const int numregs = __builtin_popcount(curinstr & 0x1FF);
    u8 rlist = curinstr & 0xFF;
    const bool r = curinstr & (1<<8);
    base -= (4*numregs);

    ARM9_WriteReg(ARM9, 13, base, false, false);
    
    while (rlist)
    {
        int reg = __builtin_ctz(rlist);
        rlist &= ~1<<reg;

        Bus9_Store32(ARM9, base, ARM9_GetReg(ARM9, reg));

        base += 4;        
    }

    if (r)
    {
        int reg = 14;

        Bus9_Store32(ARM9, base, ARM9_GetReg(ARM9, reg));

        base += 4;
    }
}

void THUMB9_POP(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    u32 base = ARM9_GetReg(ARM9, 13);
    u8 rlist = curinstr & 0xFF;
    const bool r = curinstr & (1<<8);

    while (rlist)
    {
        int reg = __builtin_ctz(rlist);
        rlist &= ~1<<reg;

        ARM9_WriteReg(ARM9, reg, Bus9_Load32(ARM9, base), false, false);

        base += 4;    
    }

    if (r)
    {
        int reg = 15;

        ARM9_WriteReg(ARM9, reg, Bus9_Load32(ARM9, base), false, true);

        base += 4;
    }

    ARM9_WriteReg(ARM9, 13, base, false, false);
}
