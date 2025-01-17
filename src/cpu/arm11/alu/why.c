#include "../arm.h"
#include "../../../utils.h"

void ARM11_SXTH(struct ARM11MPCore* ARM11, const int rd, const u32 rm)
{
    ARM11_WriteReg(ARM11, rd, (s32)(s16)rm, false, false);
}

void ARM11_SXTB(struct ARM11MPCore* ARM11, const int rd, const u32 rm)
{
    ARM11_WriteReg(ARM11, rd, (s32)(s8)rm, false, false);
}

void ARM11_UXTH(struct ARM11MPCore* ARM11, const int rd, const u32 rm)
{
    ARM11_WriteReg(ARM11, rd, rm & 0xFFFF, false, false);
}

void ARM11_UXTB(struct ARM11MPCore* ARM11, const int rd, const u32 rm)
{
    ARM11_WriteReg(ARM11, rd, rm & 0xFF, false, false);
}

void THUMB11_Extend(struct ARM11MPCore* ARM11)
{
    u16 curinstr = ARM11->Instr.Data;
    const int rd = curinstr & 0x7;
    const u32 rmval = ARM11_GetReg(ARM11, (curinstr >> 3) & 0x7);
    const u8 opcode = (curinstr >> 6) & 0x3;

    switch(opcode)
    {
    case 0: ARM11_SXTH(ARM11, rd, rmval); break;
    case 1: ARM11_SXTB(ARM11, rd, rmval); break;
    case 2: ARM11_UXTH(ARM11, rd, rmval); break;
    case 3: ARM11_UXTB(ARM11, rd, rmval); break;
    }
}
