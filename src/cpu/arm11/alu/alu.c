#include "../../../types.h"
#include "../interpreter.h"

void ARM11_AND(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn & shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s);
}

void ARM11_EOR(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn ^ shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s);
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

    ARM11_WriteReg(ARM11, rd, aluout, s);
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

    ARM11_WriteReg(ARM11, rd, aluout, s);
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

    ARM11_WriteReg(ARM11, rd, aluout, s);
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

    ARM11_WriteReg(ARM11, rd, aluout, s);
}

void ARM11_TST(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn & shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }
}

void ARM11_TEQ(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn ^ shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }
}

void ARM11_CMP(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn - shifterout;

    // todo: rd == 15?
    // ARM7TDMI & ARM946E-S have unique behavior for this edgecase, what about the arm 11?

    if (s)
    {
        ARM11->Carry = rn >= shifterout;
        ARM11->Negative = aluout >> 31;
        ARM11->Overflow = ((rn ^ shifterout) & (aluout ^ rn)) >> 31;
        ARM11->Zero = !aluout;
    }
}

void ARM11_CMN(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u64 aluout = rn + shifterout;

    // todo: rd == 15?
    // ARM7TDMI & ARM946E-S have unique behavior for this edgecase, what about the arm 11?

    if (s)
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

    ARM11_WriteReg(ARM11, rd, aluout, s);
}

void ARM11_MOV(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s);
}

void ARM11_BIC(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = rn & ~shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s);
}

void ARM11_MVN(struct ARM11MPCore* ARM11, const int rd, const u32 rn, const u32 shifterout, const bool s)
{
    u32 aluout = ~shifterout;

    if (s && (rd != 15))
    {
        ARM11->Negative = aluout >> 31;
        ARM11->Zero = !aluout;
    }

    ARM11_WriteReg(ARM11, rd, aluout, s);
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
    case 2:  ARM11_SUB_RSB(ARM11, rd, shifterout, rn, s); break;
    case 3:  ARM11_SUB_RSB(ARM11, rd, rn, shifterout, s); break;
    case 4:  ARM11_ADD(ARM11, rd, rn, shifterout, s); break;
    case 5:  ARM11_ADC(ARM11, rd, rn, shifterout, s); break;
    case 6:  ARM11_SBC_RSC(ARM11, rd, rn, shifterout, s); break;
    case 7:  ARM11_SBC_RSC(ARM11, rd, shifterout, rn, s); break;
    case 8:  ARM11_TST(ARM11, rd, rn, shifterout, s); break;
    case 9:  ARM11_TEQ(ARM11, rd, rn, shifterout, s); break;
    case 10: ARM11_CMP(ARM11, rd, rn, shifterout, s); break;
    case 11: ARM11_CMN(ARM11, rd, rn, shifterout, s); break;
    case 12: ARM11_ORR(ARM11, rd, rn, shifterout, s); break;
    case 13: ARM11_MOV(ARM11, rd, rn, shifterout, s); break;
    case 14: ARM11_BIC(ARM11, rd, rn, shifterout, s); break;
    case 15: ARM11_MVN(ARM11, rd, rn, shifterout, s); break;
    }
}
