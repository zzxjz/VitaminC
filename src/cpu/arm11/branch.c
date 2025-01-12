#include <stdio.h>
#include "interpreter.h"
#include "../../utils.h"

void ARM11_B_BL(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;
    const bool l = curinstr & (1<<24);

    if (l)
    {
        ARM11->LR = ARM11_GetReg(ARM11, 15) - 4;
    }

    u32 target = curinstr;
    target <<= 8;
    target = (s32)target >> 6;
    ARM11_Branch(ARM11, ARM11_GetReg(ARM11, 15) + (s32)target, false);
}

void ARM11_BX_BLXReg(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;
    const bool l = curinstr & (1<<5);

    if (l)
    {
        ARM11->LR = ARM11_GetReg(ARM11, 15) - 4;
    }

    ARM11_Branch(ARM11, ARM11_GetReg(ARM11, curinstr & 0xF), false);
}

void THUMB11_CondB_SWI(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const u8 condition = (curinstr >> 8) & 0xF;

    if (condition == COND_AL) { printf("UNIMPLEMENTED T BAL!!!\n"); return; }
    if (condition == COND_NV) { printf("UNIMPLEMENTED T SWI\n"); return; }

	if (CondLookup[condition] & (1<<ARM11->Flags))
    {
        s32 offset = ((s16)(s8)curinstr) << 1;
        u32 addr = ARM11_GetReg(ARM11, 15) + offset;
        ARM11_Branch(ARM11, addr | 1, false);
    }
}

void THUMB11_B(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;

    s32 offset = ((s16)curinstr << 5) >> 4;
    u32 addr = ARM11_GetReg(ARM11, 15) + offset;
    ARM11_Branch(ARM11, addr | 1, false);
}

void THUMB11_BL_BLX_LO(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;

    s32 offset = ((s32)curinstr << 21) >> 9;

    u32 addr = ARM11_GetReg(ARM11, 15) + offset;

    ARM11_WriteReg(ARM11, 14, addr, false, false);
}

void THUMB11_BL_BLX_HI(struct ARM11MPCore* ARM11)
{
    const u16 curinstr = ARM11->Instr.Data;
    const bool x = curinstr & (1<<12);

    if (!x && (curinstr & 0x1)) { printf("INVALID BL HI!!!!\n"); return; }

    s32 offset = ((s16) curinstr << 5) >> 4;
    offset += ARM11_GetReg(ARM11, 14);

    u32 lr = (ARM11_GetReg(ARM11, 15) - 4) | 1;
    ARM11_WriteReg(ARM11, 14, lr, false, false);

    u32 addr = offset;
    if (x) addr &= ~0x3;
    else addr |= 0x1;

    ARM11_Branch(ARM11, addr, false);
}

void THUMB11_BLX_BX_Reg(struct ARM11MPCore* ARM11, const u32 addr)
{
    const u16 curinstr = ARM11->Instr.Data;
    const bool l = curinstr & (1<<7);

    if (l)
    {
        u32 lr = (ARM11_GetReg(ARM11, 15) - 4) | 1;
        ARM11_WriteReg(ARM11, 14, lr, false, false);
    }

    ARM11_Branch(ARM11, addr, false);
}
