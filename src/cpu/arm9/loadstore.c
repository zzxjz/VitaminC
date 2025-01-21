#include <stdbit.h>
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
                offset = ROR32(offset, shiftimm);
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

    bool success;
    if (l)
    {
        const bool interwork = !ARM9->CP15.Control.TBitLoadDisable;
        if (b) // ldrb
        {
            u8 ret;
            if ((success = Bus9_Load8(ARM9, addr, &ret)))
                ARM9_WriteReg(ARM9, rd, ret, false, interwork);
        }
        else
        {
            u32 ret;
            if ((success = Bus9_Load32(ARM9, addr, &ret)))
            {
                ret = ROR32(ret, (addr&0x3)*8); // ARM9 always does a rotate right on unaligned 32 bit ldr
                ARM9_WriteReg(ARM9, rd, ret, false, interwork);
            }
        }
    }
    else
    {
        if (b)
        {
            u8 val = ARM9_GetReg(ARM9, rd);
            success = Bus9_Store8(ARM9, addr, val);
        }
        else
        {
            u32 val = ARM9_GetReg(ARM9, rd);
            success = Bus9_Store32(ARM9, addr, val);
        }
    }

    if (!p && w) ARM9->Mode = oldmode; // restore old mode

    if (!success) return ARM9_DataAbort(ARM9); // skip writeback if we data aborted

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
    const int rcount = stdc_count_ones(rlist);
    u32 wbbase;
    u32 base;

    if (!u) wbbase = base = rn - (rcount*4); // checkme: does this actually decrement now?
    else
    {
        base = rn;
        wbbase = rn + (rcount*4);
    }

    if (s && (!l || !r15)) ARM9_UpdateMode(ARM9, ARM9->Mode, 0x10); // swap to user mode regs

    bool success = true;
    while (rlist)
    {
        int reg = stdc_trailing_zeros(rlist);
        rlist &= ~1<<reg;

        if (p^u) base += 4;

        if (l)
        {
            u32 val;
            if ((success &= Bus9_Load32(ARM9, base, &val)))
                ARM9_WriteReg(ARM9, reg, val, s, !ARM9->CP15.Control.TBitLoadDisable);
        }
        else
        {
            success &= Bus9_Store32(ARM9, base, ARM9_GetReg(ARM9, reg));
        }

        if (!(p^u)) base += 4;
    }

    if (s && (!l || !r15)) ARM9_UpdateMode(ARM9, 0x10, ARM9->Mode); // restore actual mode's regs

    if (!success) return ARM9_DataAbort(ARM9);

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

    u32 val;
    if (Bus9_Load32(ARM9, addr, &val))
        ARM9_WriteReg(ARM9, rd, val, false, false);
    else ARM9_DataAbort(ARM9);
}

void THUMB9_LDR_STR_SPRel(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const u8 imm8 = curinstr & 0xFF;
    const u8 rd = (curinstr >> 8) & 0x7;
    const bool l = curinstr & (1<<11);

    u32 addr = ARM9_GetReg(ARM9, 13);
    addr += imm8 * 4;

    bool success;
    if (l)
    {
        u32 val;
        if ((success = Bus9_Load32(ARM9, addr, &val)))
        {
            val = ROR32(val, (addr&0x3)*8);
            ARM9_WriteReg(ARM9, rd, val, false, false);
        }
    }
    else success = Bus9_Store32(ARM9, addr, ARM9_GetReg(ARM9, rd));

    if (!success) ARM9_DataAbort(ARM9);
}

void THUMB9_LDR_STR_Reg(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const u8 rm = (curinstr >> 6) & 0x7;
    const u8 rn = (curinstr >> 3) & 0x7;
    const u8 rd = curinstr & 0x7;
    const u8 opcode = (curinstr >> 9) & 0x7;

    const u32 addr = ARM9_GetReg(ARM9, rm) + ARM9_GetReg(ARM9, rn);

    bool success;
    switch(opcode)
    {
    case 0x0: success = Bus9_Store32(ARM9, addr, ARM9_GetReg(ARM9, rd)); break; // str
    case 0x1: success = Bus9_Store16(ARM9, addr, ARM9_GetReg(ARM9, rd)); break; // strh
    case 0x2: success = Bus9_Store8 (ARM9, addr, ARM9_GetReg(ARM9, rd)); break; // strb
    case 0x3: { u8  val; if ((success = Bus9_Load8 (ARM9, addr, &val))) ARM9_WriteReg(ARM9, rd, (s32)(s8 )val, false, false); break; } // ldrsb
    case 0x4: { u32 val; if ((success = Bus9_Load32(ARM9, addr, &val))) ARM9_WriteReg(ARM9, rd, ROR32(val, (addr&0x3)*8), false, false); break; } // ldr
    case 0x5: { u16 val; if ((success = Bus9_Load16(ARM9, addr, &val))) ARM9_WriteReg(ARM9, rd, val, false, false); break; } // ldrh
    case 0x6: { u8  val; if ((success = Bus9_Load8 (ARM9, addr, &val))) ARM9_WriteReg(ARM9, rd, val, false, false); break; } // ldrb
    case 0x7: { u16 val; if ((success = Bus9_Load16(ARM9, addr, &val))) ARM9_WriteReg(ARM9, rd, (s32)(s16)val, false, false); break; } // ldrsh
    }

    if (!success) ARM9_DataAbort(ARM9);
}

void THUMB9_LDR_STR_Imm5(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const int rd = curinstr & 0x7;
    const int rn = (curinstr >> 3) & 0x7;
    const u8 imm5 = (curinstr >> 6) & 0x1F;
    const u8 opcode = (curinstr >> 11) & 0x3;

    const u32 addr = ARM9_GetReg(ARM9, rn);

    bool success;
    switch(opcode)
    {
    case 0b00: success = Bus9_Store32(ARM9, addr + (imm5*4), ARM9_GetReg(ARM9, rd)); break; // str
    case 0b01: { u32 val; if ((success = Bus9_Load32(ARM9, addr + (imm5*4), &val))) ARM9_WriteReg(ARM9, rd, ROR32(val, (addr + (imm5*4))*8), false, false); break; } // ldr
    case 0b10: success = Bus9_Store8 (ARM9, addr + (imm5  ), ARM9_GetReg(ARM9, rd)); break; // strb
    case 0b11: { u8  val; if ((success = Bus9_Load8 (ARM9, addr + (imm5  ), &val))) ARM9_WriteReg(ARM9, rd, val, false, false); break; } // ldrb
    }

    if (!success) ARM9_DataAbort(ARM9);
}

void THUMB9_LDRH_STRH_Imm5(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const int rd = curinstr & 0x7;
    const int rn = (curinstr >> 3) & 0x7;
    const u8 imm5 = (curinstr >> 6) & 0x1F;
    const bool l = curinstr & (1<<11);

    const u32 addr = ARM9_GetReg(ARM9, rn) + (imm5 * 2);

    bool success;
    if (l)
    {
        u16 val;
        if ((success = Bus9_Load16(ARM9, addr, &val)))
            ARM9_WriteReg(ARM9, rd, ROR32(val, (addr&0x3)*8), false, false);
    }
    else success = Bus9_Store16(ARM9, addr, ARM9_GetReg(ARM9, rd));

    if (!success) ARM9_DataAbort(ARM9);
}

void THUMB9_LDMIA_STMIA(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const int rn = (curinstr >> 8) & 0x7;
    u8 rlist = curinstr & 0xFF;
    const bool l = curinstr & (1<<11);
    u32 base = ARM9_GetReg(ARM9, rn);

    bool success = true;
    while (rlist)
    {
        int reg = stdc_trailing_zeros(rlist);
        rlist &= ~1<<reg;

        if (l)
        {
            u32 val;
            if ((success &= Bus9_Load32(ARM9, base, &val)))
                ARM9_WriteReg(ARM9, reg, val, false, false);
        }
        else success &= Bus9_Store32(ARM9, base, ARM9_GetReg(ARM9, reg));

        base += 4;
    }

    if (!success) return ARM9_DataAbort(ARM9);

    ARM9_WriteReg(ARM9, rn, base, false, false);
}

void THUMB9_PUSH(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    u32 base = ARM9_GetReg(ARM9, 13);
    const int numregs = stdc_count_ones(curinstr & 0x1FF);
    u8 rlist = curinstr & 0xFF;
    const bool r = curinstr & (1<<8);
    const u32 wbbase = base -= (4*numregs);

    bool success = true;
    while (rlist)
    {
        int reg = stdc_trailing_zeros(rlist);
        rlist &= ~1<<reg;

        success &= Bus9_Store32(ARM9, base, ARM9_GetReg(ARM9, reg));

        base += 4;
    }

    if (r)
    {
        int reg = 14;

        success &= Bus9_Store32(ARM9, base, ARM9_GetReg(ARM9, reg));

        base += 4;
    }

    if (!success) return ARM9_DataAbort(ARM9);

    ARM9_WriteReg(ARM9, 13, wbbase, false, false);
}

void THUMB9_POP(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    u32 base = ARM9_GetReg(ARM9, 13);
    u8 rlist = curinstr & 0xFF;
    const bool r = curinstr & (1<<8);

    bool success = true;
    while (rlist)
    {
        int reg = stdc_trailing_zeros(rlist);
        rlist &= ~1<<reg;

        u32 val;
        if ((success &= Bus9_Load32(ARM9, base, &val)))
            ARM9_WriteReg(ARM9, reg, val, false, false);

        base += 4;
    }

    if (r)
    {
        int reg = 15;
        
        u32 val;
        if ((success &= Bus9_Load32(ARM9, base, &val)))
            ARM9_WriteReg(ARM9, reg, val, false, true);

        base += 4;
    }

    if (!success) return ARM9_DataAbort(ARM9);

    ARM9_WriteReg(ARM9, 13, base, false, false);
}
