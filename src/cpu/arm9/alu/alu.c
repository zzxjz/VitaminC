#include <stdio.h>
#include "../../../utils.h"
#include "../arm.h"

void ARM9_AND(struct ARM946E_S* ARM9, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn & shifterout;

    if (s && (rd != 15))
    {
        ARM9->Negative = aluout >> 31;
        ARM9->Zero = !aluout;
    }

    ARM9_WriteReg(ARM9, rd, aluout, s, false);
}

void ARM9_EOR(struct ARM946E_S* ARM9, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn ^ shifterout;

    if (s && (rd != 15))
    {
        ARM9->Negative = aluout >> 31;
        ARM9->Zero = !aluout;
    }

    ARM9_WriteReg(ARM9, rd, aluout, s, false);
}

void ARM9_SUB_RSB(struct ARM946E_S* ARM9, const int rd, const u32 a, const u32 b, const bool s)
{
    u32 aluout = a - b;

    if (s && (rd != 15))
    {
        ARM9->Carry = a >= b;
        ARM9->Negative = aluout >> 31;
        ARM9->Overflow = ((a ^ b) & (aluout ^ a)) >> 31;
        ARM9->Zero = !aluout;
    }

    ARM9_WriteReg(ARM9, rd, aluout, s, false);
}

void ARM9_ADD(struct ARM946E_S* ARM9, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u64 aluout = rn + shifterout;

    if (s && (rd != 15))
    {
        ARM9->Carry = aluout >> 32;
        aluout &= 0xFFFFFFFF;
        ARM9->Overflow = (~(rn ^ shifterout) & (aluout ^ rn)) >> 31;
        ARM9->Negative = aluout >> 31;
        ARM9->Zero = !aluout;
    }

    ARM9_WriteReg(ARM9, rd, aluout, s, false);
}

void ARM9_ADC(struct ARM946E_S* ARM9, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u64 aluout = rn + shifterout + ARM9->Carry;

    if (s && (rd != 15))
    {
        ARM9->Carry = aluout >> 32;
        aluout &= 0xFFFFFFFF;
        ARM9->Overflow = (~(rn ^ shifterout) & (aluout ^ rn)) >> 31;
        ARM9->Negative = aluout >> 31;
        ARM9->Zero = !aluout;
    }

    ARM9_WriteReg(ARM9, rd, aluout, s, false);
}

void ARM9_SBC_RSC(struct ARM946E_S* ARM9, const int rd, const u32 a, const u32 b, const bool s)
{
    u32 aluout = a - b - !ARM9->Carry;

    if (s && (rd != 15))
    {
        ARM9->Carry = a >= ((u64)b + !ARM9->Carry);
        ARM9->Negative = aluout >> 31;
        ARM9->Overflow = ((a ^ b) & (aluout ^ a)) >> 31;
        ARM9->Zero = !aluout;
    }

    ARM9_WriteReg(ARM9, rd, aluout, s, false);
}

void ARM9_TST(struct ARM946E_S* ARM9, const int rd, const u32 rn, const u32 shifterout)
{
    u32 aluout = rn & shifterout;

    // todo: rd == 15?
    // ARM7TDMI & ARM946E-S have unique behavior for this edgecase, what about the arm 11?

    if (rd != 15)
    {
        ARM9->Negative = aluout >> 31;
        ARM9->Zero = !aluout;
    }
}

void ARM9_TEQ(struct ARM946E_S* ARM9, const int rd, const u32 rn, const u32 shifterout)
{
    u32 aluout = rn ^ shifterout;

    // todo: rd == 15?
    // ARM7TDMI & ARM946E-S have unique behavior for this edgecase, what about the arm 11?

    if (rd != 15)
    {
        ARM9->Negative = aluout >> 31;
        ARM9->Zero = !aluout;
    }
}

void ARM9_CMP(struct ARM946E_S* ARM9, const int rd, const u32 rn, const u32 shifterout)
{
    u32 aluout = rn - shifterout;

    // todo: rd == 15?
    // ARM7TDMI & ARM946E-S have unique behavior for this edgecase, what about the arm 11?

    if (rd != 15)
    {
        ARM9->Carry = rn >= shifterout;
        if (ARM9->PC == 0x134d8) printf("CARRRY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %i %i\n", ARM9->Carry, rn >= shifterout);
        ARM9->Negative = aluout >> 31;
        ARM9->Overflow = ((rn ^ shifterout) & (aluout ^ rn)) >> 31;
        ARM9->Zero = !aluout;
    }
}

void ARM9_CMN(struct ARM946E_S* ARM9, const int rd, const u32 rn, const u32 shifterout)
{
    u64 aluout = rn + shifterout;

    // todo: rd == 15?
    // ARM7TDMI & ARM946E-S have unique behavior for this edgecase, what about the arm 11?

    if (rd != 15)
    {
        ARM9->Carry = aluout >> 32;
        aluout &= 0xFFFFFFFF;
        ARM9->Overflow = (~(rn ^ shifterout) & (aluout ^ rn)) >> 31;
        ARM9->Negative = aluout >> 31;
        ARM9->Zero = !aluout;
    }
}

void ARM9_ORR(struct ARM946E_S* ARM9, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn | shifterout;

    if (s && (rd != 15))
    {
        ARM9->Negative = aluout >> 31;
        ARM9->Zero = !aluout;
    }

    ARM9_WriteReg(ARM9, rd, aluout, s, false);
}

void ARM9_MOV(struct ARM946E_S* ARM9, const int rd, const u32 shifterout, const bool s)
{
    u32 aluout = shifterout;

    if (s && (rd != 15))
    {
        ARM9->Negative = aluout >> 31;
        ARM9->Zero = !aluout;
    }

    ARM9_WriteReg(ARM9, rd, aluout, s, false);
}

void ARM9_BIC(struct ARM946E_S* ARM9, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn & ~shifterout;

    if (s && (rd != 15))
    {
        ARM9->Negative = aluout >> 31;
        ARM9->Zero = !aluout;
    }

    ARM9_WriteReg(ARM9, rd, aluout, s, false);
}

void ARM9_MVN(struct ARM946E_S* ARM9, const int rd, const u32 shifterout, const bool s)
{
    u32 aluout = ~shifterout;

    if (s && (rd != 15))
    {
        ARM9->Negative = aluout >> 31;
        ARM9->Zero = !aluout;
    }

    ARM9_WriteReg(ARM9, rd, aluout, s, false);
}

void ARM9_ALU(struct ARM946E_S* ARM9)
{
    const u32 curinstr = ARM9->Instr[0].Data;
    const int opcode = (curinstr >> 21) & 0xF;
    const bool s = curinstr & (1<<20);
    const int rd = (curinstr >> 12) & 0xF;
    const u32 rn = ARM9_GetReg(ARM9, (curinstr >> 16) & 0xF);
    u32 shifterout;

    if (curinstr & (1<<25)) // immediate
    {
        const u8 imm8 = curinstr & 0xFF;
        const u8 rorimm = ((curinstr >> 8) & 0xF) * 2;

        shifterout = ARM9_ROR32(imm8, rorimm);
        
        if (s && rorimm)
            ARM9->Carry = shifterout >> 31;
    }
    else
    {
        u64 rm = ARM9_GetReg(ARM9, curinstr & 0xF);

        switch ((curinstr >> 4) & 0b111)
        {
        case 0: // reg / lsl imm
        {
            const u8 shiftimm = (curinstr >> 7) & 0x1F;
            rm <<= shiftimm;
            
            if (s && shiftimm)
                ARM9->Carry = (rm >> 32) & 0x1;

            shifterout = rm;
            break;
        }
        case 1: // lsl reg
        {
            const u8 rs = ARM9_GetReg(ARM9, (curinstr >> 8) & 0xF) & 0xFF;
            rm <<= rs;
            
            if (s && rs)
                ARM9->Carry = (rm >> 32) & 0x1;

            shifterout = rm;
            break;
        }
        case 2: // lsr imm
        {
            u8 shiftimm = (curinstr >> 7) & 0x1F;
            if (!shiftimm) shiftimm = 32;

            if (s)
                ARM9->Carry = (rm >> (shiftimm-1)) & 0x1;

            rm >>= shiftimm;

            shifterout = rm;
            break;
        }
        case 3: // lsr reg
        {
            const u8 rs = ARM9_GetReg(ARM9, (curinstr >> 8) & 0xF) & 0xFF;
            
            if (s && rs)
                ARM9->Carry = (rm >> (rs-1)) & 0x1;

            rm >>= rs;

            shifterout = rm;
            break;
        }
        case 4: // asr imm
        {
            u8 shiftimm = (curinstr >> 7) & 0x1F;
            if (!shiftimm) shiftimm = 32;

            if (s)
                ARM9->Carry = ((s32)rm >> (shiftimm-1)) & 0x1;

            rm = (s32)rm >> shiftimm;

            shifterout = rm;
            break;
        }
        case 5: // asr reg
        {
            const u8 rs = ARM9_GetReg(ARM9, (curinstr >> 8) & 0xF) & 0xFF;
            
            if (s && rs)
                ARM9->Carry = ((s32)rm >> (rs-1)) & 0x1;

            rm = (s32)rm >> rs;

            shifterout = rm;
            break;
        }
        case 6: // ror imm / rrx
        {
            const u8 rorimm = (curinstr >> 7) & 0x1F;

            if (rorimm) // ror imm
            {
                shifterout = ARM9_ROR32(rm, rorimm);

                if (s)
                    ARM9->Carry = shifterout >> 31;
            }
            else // rrx
            {
                shifterout = (ARM9->Carry << 31) | (rm >> 1);

                if (s)
                    ARM9->Carry = rm & 0x1;
            }
            break;
        }
        case 7: // ror reg
        {
            const u8 rs = ARM9_GetReg(ARM9, (curinstr >> 8) & 0xF) & 0xFF;

            shifterout = ARM9_ROR32(rm, rs);

            if (s && rs)
                ARM9->Carry = shifterout >> 31;

            break;
        }
        }
    }

    switch(opcode)
    {
    case 0:  ARM9_AND(ARM9, rd, rn, shifterout, s); break;
    case 1:  ARM9_EOR(ARM9, rd, rn, shifterout, s); break;
    case 2:  ARM9_SUB_RSB(ARM9, rd, rn, shifterout, s); break;
    case 3:  ARM9_SUB_RSB(ARM9, rd, shifterout, rn, s); break;
    case 4:  ARM9_ADD(ARM9, rd, rn, shifterout, s); break;
    case 5:  ARM9_ADC(ARM9, rd, rn, shifterout, s); break;
    case 6:  ARM9_SBC_RSC(ARM9, rd, rn, shifterout, s); break;
    case 7:  ARM9_SBC_RSC(ARM9, rd, shifterout, rn, s); break;
    case 8:  ARM9_TST(ARM9, rd, rn, shifterout); break;
    case 9:  ARM9_TEQ(ARM9, rd, rn, shifterout); break;
    case 10: ARM9_CMP(ARM9, rd, rn, shifterout); break;
    case 11: ARM9_CMN(ARM9, rd, rn, shifterout); break;
    case 12: ARM9_ORR(ARM9, rd, rn, shifterout, s); break;
    case 13: ARM9_MOV(ARM9, rd, shifterout, s); break;
    case 14: ARM9_BIC(ARM9, rd, rn, shifterout, s); break;
    case 15: ARM9_MVN(ARM9, rd, shifterout, s); break;
    }
}

void THUMB9_LSL(struct ARM946E_S* ARM9, const u8 rd, const u32 rm, const u32 val)
{
    u64 aluout = rm;

    if (val)
    {
        aluout <<= val;
        ARM9->Carry = (aluout >> 32) & 0x1;
        aluout &= 0xFFFFFFFF;
    }
    // aluout should always be 32 bits by this point
    ARM9->Negative = aluout >> 31;
    ARM9->Zero = aluout;
    ARM9_WriteReg(ARM9, rd, aluout, false, false);
}

void THUMB9_LSR(struct ARM946E_S* ARM9, const u8 rd, u32 rm, const u32 val)
{
    if (val)
    {
        ARM9->Carry = (rm >> (val-1)) & 0x1;
        rm >>= val;
    }
    ARM9->Negative = false; // always cleared :(
    ARM9->Zero = rm;
    ARM9_WriteReg(ARM9, rd, rm, false, false);
}

void THUMB9_ASR(struct ARM946E_S* ARM9, const u8 rd, u32 rm, const u32 val)
{
    if (val)
    {
        ARM9->Carry = ((s32)rm >> (val-1)) & 0x1;
        rm = (s32)rm >> val;
    }
    ARM9->Negative = rm >> 31;
    ARM9->Zero = rm;
    ARM9_WriteReg(ARM9, rd, rm, false, false);
}

void THUMB9_ROR(struct ARM946E_S* ARM9, const u8 rd, const u32 rdval, const u32 rs) // rick of rain is back!
{
    u64 rorout = rdval;
    if (rs)
    {
        rorout = ARM9_ROR32(rorout, rs);
        ARM9->Carry = rorout >> 31;
        rorout &= 0xFFFFFFFF;
    }

    ARM9->Negative = rorout >> 31;
    ARM9->Zero = rorout;
    ARM9_WriteReg(ARM9, rd, rorout, false, false);
}

void THUMB9_ShiftImm(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const u8 opcode = ((curinstr >> 11) & 0x3);
    const u8 rd = curinstr & 0x7;
    const u32 rm = ARM9_GetReg(ARM9, (curinstr >> 3) & 0x7);
    const u8 imm5 = (curinstr >> 6) & 0x1F;

    switch(opcode)
    {
    case 0x0: THUMB9_LSL(ARM9, rd, rm, imm5); break;
    case 0x1: THUMB9_LSR(ARM9, rd, rm, imm5 ? imm5 : 32); break;
    case 0x2: THUMB9_ASR(ARM9, rd, rm, imm5 ? imm5 : 32); break;
    default: __builtin_unreachable();
    }
}

void THUMB9_ADD_SUB_Reg_Imm3(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const bool imm = curinstr & (1<<10);
    const bool sub = curinstr & (1<<9);
    const u8 bits = (curinstr >> 6) & 0x7;
    const u32 val = (imm ? bits : ARM9_GetReg(ARM9, bits));
    const u8 rn = (curinstr >> 3) & 0x7;
    const u32 rnval = ARM9_GetReg(ARM9, rn);
    const u8 rd = curinstr & 0x7;

    if (sub) ARM9_SUB_RSB(ARM9, rd, rnval, val, true);
    else ARM9_ADD(ARM9, rd, rnval, val, true);
}

void THUMB9_ADD_SUB_CMP_MOV_Imm8(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const u8 opcode = (curinstr >> 11) & 0x3;
    const int rd = (curinstr >> 8) & 0x7;
    const u32 rdval = ARM9_GetReg(ARM9, rd);
    const u8 imm8 = curinstr & 0xFF;

    switch(opcode)
    {
    case 0x0: ARM9_MOV(ARM9, rd, imm8, true); break;
    case 0x1: ARM9_CMP(ARM9, rd, rdval, imm8); break;
    case 0x2: ARM9_ADD(ARM9, rd, rdval, imm8, true); break;
    case 0x3: ARM9_SUB_RSB(ARM9, rd, rdval, imm8, true); break;
    }
}

void THUMB9_ALU(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const u8 opcode = (curinstr >> 6) & 0xF;
    const u8 rd = curinstr & 0x7;
    const u32 rm = ARM9_GetReg(ARM9, (curinstr >> 0x3) & 0x7);
    const u32 rdval = ARM9_GetReg(ARM9, rd);

    switch(opcode)
    {
    case 0x0: ARM9_AND(ARM9, rd, rdval, rm, true); break;
    case 0x1: ARM9_EOR(ARM9, rd, rdval, rm, true); break;
    case 0x2: THUMB9_LSL(ARM9, rd, rdval, rm); break;
    case 0x3: THUMB9_LSR(ARM9, rd, rdval, rm); break;
    case 0x4: THUMB9_ASR(ARM9, rd, rdval, rm); break;
    case 0x5: ARM9_ADC(ARM9, rd, rdval, rm, true); break;
    case 0x6: ARM9_SBC_RSC(ARM9, rd, rdval, rm, true); break;
    case 0x7: THUMB9_ROR(ARM9, rd, rdval, rm); break;
    case 0x8: ARM9_TST(ARM9, rd, rdval, rm); break;
    case 0x9: ARM9_SUB_RSB(ARM9, rd, 0, rm, true); break; // NEG
    case 0xA: ARM9_CMP(ARM9, rd, rdval, rm); break;
    case 0xB: ARM9_CMN(ARM9, rd, rdval, rm); break;
    case 0xC: ARM9_ORR(ARM9, rd, rdval, rm, true); break;
    case 0xD: THUMB9_MULS(ARM9, rd, rdval, rm); break;
    case 0xE: ARM9_BIC(ARM9, rd, rdval, rm, true); break;
    case 0xF: ARM9_MVN(ARM9, rd, rm, true); break;
    }
}

void THUMB9_ALU_HI(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const u8 opcode = (curinstr >> 8) & 0x3;
    const u8 rd = ((curinstr >> 4) & 0x8) | (curinstr & 0x7);
    const u8 rm = ((curinstr >> 3) & 0x8) | ((curinstr >> 3) & 0x7);
    const u32 rdval = ARM9_GetReg(ARM9, rd);
    const u32 rmval = ARM9_GetReg(ARM9, rm);

    switch(opcode)
    {
    case 0b00: ARM9_ADD(ARM9, rd, rdval, rmval, false); break;
    case 0b01: ARM9_CMP(ARM9, rd, rdval, rmval); break;
    case 0b10: ARM9_MOV(ARM9, rd, rmval, false); break;
    case 0b11: THUMB9_BLX_BX_Reg(ARM9, rmval); break;
    }
}

void THUMB9_ADD_SP_PCRel(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const u8 rd = (curinstr >> 8) & 0x7;
    const u8 imm8 = curinstr & 0xFF;
    const bool sp = curinstr & (1<<11);

    u32 val = (sp ? ARM9_GetReg(ARM9, 13) : (ARM9_GetReg(ARM9, 15) & ~0x3));
    val += imm8 * 4;

    ARM9_WriteReg(ARM9, rd, val, false, false);
}

void THUMB9_ADD_SUB_SP(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    s8 imm7 = curinstr & 0x7F;
    const bool sub = curinstr & (1<<7);

    if (sub) imm7 = -imm7;

    u32 spval = ARM9_GetReg(ARM9, 13);
    spval += imm7 * 4;
    ARM9_WriteReg(ARM9, 13, spval, false, false);
}
