#include <stdio.h>
#include "../../../utils.h"
#include "../interpreter.h"

void ARM11_AND(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn & shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s, false);
}

void ARM11_EOR(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn ^ shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s, false);
}

void ARM11_SUB_RSB(struct ARM11MPCore* ARM11, const int rd, const u32 a, const u32 b, const bool s)
{
    u32 aluout = a - b;

    if (s && (rd != 15))
    {
        ARM11->Carry = a >= b;
        ARM11->Negative = aluout >> 31;
        ARM11->Overflow = ((a ^ b) & (aluout ^ a)) >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s, false);
}

void ARM11_ADD(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u64 aluout = rn + shifterout;

    if (s && (rd != 15))
    {
        ARM11->Carry = aluout >> 32;
        aluout &= 0xFFFFFFFF;
        ARM11->Overflow = (~(rn ^ shifterout) & (aluout ^ rn)) >> 31;
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s, false);
}

void ARM11_ADC(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u64 aluout = rn + shifterout + ARM11->Carry;

    if (s && (rd != 15))
    {
        ARM11->Carry = aluout >> 32;
        aluout &= 0xFFFFFFFF;
        ARM11->Overflow = (~(rn ^ shifterout) & (aluout ^ rn)) >> 31;
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s, false);
}

void ARM11_SBC_RSC(struct ARM11MPCore* ARM11, const int rd, const u32 a, const u32 b, const bool s)
{
    u32 aluout = a - b - !ARM11->Carry;

    if (s && (rd != 15))
    {
        ARM11->Carry = a >= ((u64)b + !ARM11->Carry);
        ARM11->Negative = aluout >> 31;
        ARM11->Overflow = ((a ^ b) & (aluout ^ a)) >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s, false);
}

void ARM11_TST(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout)
{
    u32 aluout = rn & shifterout;

    // todo: rd == 15?
    // ARM7TDMI & ARM946E-S have unique behavior for this edgecase, what about the arm 11?

    if (rd != 15)
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }
}

void ARM11_TEQ(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout)
{
    u32 aluout = rn ^ shifterout;

    // todo: rd == 15?
    // ARM7TDMI & ARM946E-S have unique behavior for this edgecase, what about the arm 11?

    if (rd != 15)
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }
}

void ARM11_CMP(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout)
{
    u32 aluout = rn - shifterout;

    // todo: rd == 15?
    // ARM7TDMI & ARM946E-S have unique behavior for this edgecase, what about the arm 11?

    if (rd != 15)
    {
        ARM11->Carry = rn >= shifterout;
        if (ARM11->PC == 0x134d8) printf("CARRRY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %i %i\n", ARM11->Carry, rn >= shifterout);
        ARM11->Negative = aluout >> 31;
        ARM11->Overflow = ((rn ^ shifterout) & (aluout ^ rn)) >> 31;
        ARM11->Zero = !aluout;
    }
}

void ARM11_CMN(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout)
{
    u64 aluout = rn + shifterout;

    // todo: rd == 15?
    // ARM7TDMI & ARM946E-S have unique behavior for this edgecase, what about the arm 11?

    if (rd != 15)
    {
        ARM11->Carry = aluout >> 32;
        aluout &= 0xFFFFFFFF;
        ARM11->Overflow = (~(rn ^ shifterout) & (aluout ^ rn)) >> 31;
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }
}

void ARM11_ORR(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn | shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s, false);
}

void ARM11_MOV(struct ARM11MPCore* ARM11, const int rd, const u32 shifterout, const bool s)
{
    u32 aluout = shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s, false);
}

void ARM11_BIC(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn & ~shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s, false);
}

void ARM11_MVN(struct ARM11MPCore* ARM11, const int rd, const u32 shifterout, const bool s)
{
    u32 aluout = ~shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s, false);
}

void ARM11_ALU(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;
    const int opcode = (curinstr >> 21) & 0xF;
    const bool s = curinstr & (1<<20);
    const int rd = (curinstr >> 12) & 0xF;
    const u32 rn = ARM11_GetReg(ARM11, (curinstr >> 16) & 0xF);
    u32 shifterout;

    if (curinstr & (1<<25)) // immediate
    {
        const u8 imm8 = curinstr & 0xFF;
        const u8 rorimm = ((curinstr >> 8) & 0xF) * 2;

        shifterout = ARM11_ROR32(imm8, rorimm);
        
        if (s && rorimm)
            ARM11->Carry = shifterout >> 31;
    }
    else
    {
        u64 rm = ARM11_GetReg(ARM11, curinstr & 0xF);

        switch ((curinstr >> 4) & 0b111)
        {
        case 0: // reg / lsl imm
        {
            const u8 shiftimm = (curinstr >> 7) & 0x1F;
            rm <<= shiftimm;
            
            if (s && shiftimm)
                ARM11->Carry = (rm >> 32) & 0x1;

            shifterout = rm;
            break;
        }
        case 1: // lsl reg
        {
            const u8 rs = ARM11_GetReg(ARM11, (curinstr >> 8) & 0xF) & 0xFF;
            rm <<= rs;
            
            if (s && rs)
                ARM11->Carry = (rm >> 32) & 0x1;

            shifterout = rm;
            break;
        }
        case 2: // lsr imm
        {
            u8 shiftimm = (curinstr >> 7) & 0x1F;
            if (!shiftimm) shiftimm = 32;

            if (s)
                ARM11->Carry = (rm >> (shiftimm-1)) & 0x1;

            rm >>= shiftimm;

            shifterout = rm;
            break;
        }
        case 3: // lsr reg
        {
            const u8 rs = ARM11_GetReg(ARM11, (curinstr >> 8) & 0xF) & 0xFF;
            
            if (s && rs)
                ARM11->Carry = (rm >> (rs-1)) & 0x1;

            rm >>= rs;

            shifterout = rm;
            break;
        }
        case 4: // asr imm
        {
            u8 shiftimm = (curinstr >> 7) & 0x1F;
            if (!shiftimm) shiftimm = 32;

            if (s)
                ARM11->Carry = ((s32)rm >> (shiftimm-1)) & 0x1;

            rm = (s32)rm >> shiftimm;

            shifterout = rm;
            break;
        }
        case 5: // asr reg
        {
            const u8 rs = ARM11_GetReg(ARM11, (curinstr >> 8) & 0xF) & 0xFF;
            
            if (s && rs)
                ARM11->Carry = ((s32)rm >> (rs-1)) & 0x1;

            rm = (s32)rm >> rs;

            shifterout = rm;
            break;
        }
        case 6: // ror imm / rrx
        {
            const u8 rorimm = (curinstr >> 7) & 0x1F;

            if (rorimm) // ror imm
            {
                shifterout = ARM11_ROR32(rm, rorimm);

                if (s)
                    ARM11->Carry = shifterout >> 31;
            }
            else // rrx
            {
                shifterout = (ARM11->Carry << 31) | (rm >> 1);

                if (s)
                    ARM11->Carry = rm & 0x1;
            }
            break;
        }
        case 7: // ror reg
        {
            const u8 rs = ARM11_GetReg(ARM11, (curinstr >> 8) & 0xF) & 0xFF;

            shifterout = ARM11_ROR32(rm, rs);

            if (s && rs)
                ARM11->Carry = shifterout >> 31;

            break;
        }
        }
    }

    switch(opcode)
    {
    case 0:  ARM11_AND(ARM11, rd, rn, shifterout, s); break;
    case 1:  ARM11_EOR(ARM11, rd, rn, shifterout, s); break;
    case 2:  ARM11_SUB_RSB(ARM11, rd, rn, shifterout, s); break;
    case 3:  ARM11_SUB_RSB(ARM11, rd, shifterout, rn, s); break;
    case 4:  ARM11_ADD(ARM11, rd, rn, shifterout, s); break;
    case 5:  ARM11_ADC(ARM11, rd, rn, shifterout, s); break;
    case 6:  ARM11_SBC_RSC(ARM11, rd, rn, shifterout, s); break;
    case 7:  ARM11_SBC_RSC(ARM11, rd, shifterout, rn, s); break;
    case 8:  ARM11_TST(ARM11, rd, rn, shifterout); break;
    case 9:  ARM11_TEQ(ARM11, rd, rn, shifterout); break;
    case 10: ARM11_CMP(ARM11, rd, rn, shifterout); break;
    case 11: ARM11_CMN(ARM11, rd, rn, shifterout); break;
    case 12: ARM11_ORR(ARM11, rd, rn, shifterout, s); break;
    case 13: ARM11_MOV(ARM11, rd, shifterout, s); break;
    case 14: ARM11_BIC(ARM11, rd, rn, shifterout, s); break;
    case 15: ARM11_MVN(ARM11, rd, shifterout, s); break;
    }
}

void THUMB11_LSL(struct ARM11MPCore* ARM11, const u8 rd, const u32 rm, const u32 val)
{
    u64 aluout = rm;

    if (val)
    {
        aluout <<= val;
        ARM11->Carry = (aluout >> 32) & 0x1;
        aluout &= 0xFFFFFFFF;
    }
    // aluout should always be 32 bits by this point
    ARM11->Negative = aluout >> 31;
    ARM11->Zero = aluout;
    ARM11_WriteReg(ARM11, rd, aluout, false, false);
}

void THUMB11_LSR(struct ARM11MPCore* ARM11, const u8 rd, u32 rm, const u32 val)
{
    if (val)
    {
        ARM11->Carry = (rm >> (val-1)) & 0x1;
        rm >>= val;
    }
    ARM11->Negative = false; // always cleared :(
    ARM11->Zero = rm;
    ARM11_WriteReg(ARM11, rd, rm, false, false);
}

void THUMB11_ASR(struct ARM11MPCore* ARM11, const u8 rd, u32 rm, const u32 val)
{
    if (val)
    {
        ARM11->Carry = ((s32)rm >> (val-1)) & 0x1;
        rm = (s32)rm >> val;
    }
    ARM11->Negative = rm >> 31;
    ARM11->Zero = rm;
    ARM11_WriteReg(ARM11, rd, rm, false, false);
}

void THUMB11_ROR(struct ARM11MPCore* ARM11, const u8 rd, const u32 rdval, const u32 rs) // rick of rain is back!
{
    u64 rorout = rdval;
    if (rs)
    {
        rorout = ARM11_ROR32(rorout, rs);
        ARM11->Carry = rorout >> 31;
        rorout &= 0xFFFFFFFF;
    }

    ARM11->Negative = rorout >> 31;
    ARM11->Zero = rorout;
    ARM11_WriteReg(ARM11, rd, rorout, false, false);
}

void THUMB11_ShiftImm(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const u8 opcode = ((curinstr >> 11) & 0x3);
    const u8 rd = curinstr & 0x7;
    const u32 rm = ARM11_GetReg(ARM11, (curinstr >> 3) & 0x7);
    const u8 imm5 = (curinstr >> 6) & 0x1F;

    switch(opcode)
    {
    case 0x0: THUMB11_LSL(ARM11, rd, rm, imm5); break;
    case 0x1: THUMB11_LSR(ARM11, rd, rm, imm5 ? imm5 : 32); break;
    case 0x2: THUMB11_ASR(ARM11, rd, rm, imm5 ? imm5 : 32); break;
    default: __builtin_unreachable();
    }
}

void THUMB11_ADD_SUB_Reg_Imm3(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const bool imm = curinstr & (1<<10);
    const bool sub = curinstr & (1<<9);
    const u8 bits = (curinstr >> 6) & 0x7;
    const u32 val = (imm ? bits : ARM11_GetReg(ARM11, bits));
    const u8 rn = (curinstr >> 3) & 0x7;
    const u32 rnval = ARM11_GetReg(ARM11, rn);
    const u8 rd = curinstr & 0x7;

    if (sub) ARM11_SUB_RSB(ARM11, rd, rnval, val, true);
    else ARM11_ADD(ARM11, rd, rnval, val, true);
}

void THUMB11_ADD_SUB_CMP_MOV_Imm8(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const u8 opcode = (curinstr >> 11) & 0x3;
    const u8 rd = (curinstr >> 8) & 0x3;
    const u8 rdval = ARM11_GetReg(ARM11, rd);
    const u8 imm8 = curinstr & 0xFF;

    switch(opcode)
    {
    case 0x0: ARM11_MOV(ARM11, rd, imm8, true); break;
    case 0x1: ARM11_CMP(ARM11, rd, rdval, imm8); break;
    case 0x2: ARM11_ADD(ARM11, rd, rdval, imm8, true); break;
    case 0x3: ARM11_SUB_RSB(ARM11, rd, rdval, imm8, true); break;
    }
}

void THUMB11_ALU(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const u8 opcode = (curinstr >> 6) & 0xF;
    const u8 rd = curinstr & 0x7;
    const u32 rm = ARM11_GetReg(ARM11, (curinstr >> 0x3) & 0x7);
    const u32 rdval = ARM11_GetReg(ARM11, rd);

    switch(opcode)
    {
    case 0x0: ARM11_AND(ARM11, rd, rdval, rm, true); break;
    case 0x1: ARM11_EOR(ARM11, rd, rdval, rm, true); break;
    case 0x2: THUMB11_LSL(ARM11, rd, rdval, rm); break;
    case 0x3: THUMB11_LSR(ARM11, rd, rdval, rm); break;
    case 0x4: THUMB11_ASR(ARM11, rd, rdval, rm); break;
    case 0x5: ARM11_ADC(ARM11, rd, rdval, rm, true); break;
    case 0x6: ARM11_SBC_RSC(ARM11, rd, rdval, rm, true); break;
    case 0x7: THUMB11_ROR(ARM11, rd, rdval, rm); break;
    case 0x8: ARM11_TST(ARM11, rd, rdval, rm); break;
    case 0x9: ARM11_SUB_RSB(ARM11, rd, 0, rm, true); break; // NEG
    case 0xA: ARM11_CMP(ARM11, rd, rdval, rm); break;
    case 0xB: ARM11_CMN(ARM11, rd, rdval, rm); break;
    case 0xC: ARM11_ORR(ARM11, rd, rdval, rm, true); break;
    case 0xD: THUMB11_MULS(ARM11, rd, rdval, rm); break;
    case 0xE: ARM11_BIC(ARM11, rd, rdval, rm, true); break;
    case 0xF: ARM11_MVN(ARM11, rd, rm, true); break;
    }
}

void THUMB11_ALU_HI(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const u8 opcode = (curinstr >> 8) & 0x3;
    const u8 rd = ((curinstr >> 4) & 0x8) | (curinstr & 0x7);
    const u8 rm = ((curinstr >> 3) & 0x8) | ((curinstr >> 3) & 0x7);
    const u32 rdval = ARM11_GetReg(ARM11, rd);
    const u32 rmval = ARM11_GetReg(ARM11, rm);

    switch(opcode)
    {
    case 0b00: ARM11_ADD(ARM11, rd, rdval, rmval, false); break;
    case 0b01: ARM11_CMP(ARM11, rd, rdval, rmval); break;
    case 0b10: ARM11_MOV(ARM11, rd, rmval, false); break;
    case 0b11: THUMB11_BLX_BX_Reg(ARM11, rmval); break;
    }
}

void THUMB11_ADD_SP_PCRel(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const u8 rd = (curinstr >> 8) & 0x7;
    const u8 imm8 = curinstr;
    const bool sp = curinstr & (1<<11);

    u32 val = (sp ? ARM11_GetReg(ARM11, 13) : (ARM11_GetReg(ARM11, 15) & ~0x3));
    val += imm8 * 4;

    ARM11_WriteReg(ARM11, rd, val, false, false);
}

void THUMB11_ADD_SUB_SP(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    s8 imm7 = curinstr & 0x7F;
    const bool sub = curinstr & (1<<7);

    if (sub) imm7 = -imm7;

    u32 spval = ARM11_GetReg(ARM11, 13);
    spval += imm7 * 4;
    ARM11_WriteReg(ARM11, 13, spval, false, false);
}
