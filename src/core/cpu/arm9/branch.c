#include "arm.h"
#include "../../utils.h"

void ARM9_B_BL(struct ARM946E_S* ARM9)
{
    const u32 curinstr = ARM9->Instr[0].Data;
    const bool l = curinstr & (1<<24);

    if (l)
    {
        ARM9->LR = ARM9_GetReg(ARM9, 15) - 4;
    }

    u32 target = curinstr;
    target <<= 8;
    target = (s32)target >> 6;
    ARM9_Branch(ARM9, ARM9_GetReg(ARM9, 15) + (s32)target, false);
}

void ARM9_BX_BLXReg(struct ARM946E_S* ARM9)
{
    const u32 curinstr = ARM9->Instr[0].Data;
    const bool l = curinstr & (1<<5);

    if (l)
    {
        ARM9->LR = ARM9_GetReg(ARM9, 15) - 4;
    }

    ARM9_Branch(ARM9, ARM9_GetReg(ARM9, curinstr & 0xF), false);
}

void ARM9_BLX_Imm(struct ARM946E_S* ARM9)
{
    const u32 curinstr = ARM9->Instr[0].Data;
    u32 addr = curinstr & 0x00FFFFFF;

    addr = (s32)(addr << 8) >> 6;

    addr += ((curinstr >> 24) & 0x1) << 1;
    addr = (s32)addr + ARM9_GetReg(ARM9, 15);
    ARM9_WriteReg(ARM9, 14, ARM9_GetReg(ARM9, 15) - 4, false, false);

    ARM9_Branch(ARM9, addr | 1, false);
}

void THUMB9_CondB_SWI(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const u8 condition = (curinstr >> 8) & 0xF;

    if (condition == COND_AL) { ARM9_UndefinedInstruction(ARM9); return; }
    if (condition == COND_NV) { ARM9_SupervisorCall(ARM9); return; }

    if (CondLookup[condition] & (1<<ARM9->Flags))
    {
        s32 offset = ((s16)(s8)curinstr) << 1;
        u32 addr = ARM9_GetReg(ARM9, 15) + offset;
        ARM9_Branch(ARM9, addr | 1, false);
    }
}

void THUMB9_B(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;

    s32 offset = (s16)(curinstr << 5) >> 4;
    u32 addr = ARM9_GetReg(ARM9, 15) + offset;
    ARM9_Branch(ARM9, addr | 1, false);
}

void THUMB9_BL_BLX_LO(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;

    s32 offset = (s32)(curinstr << 21) >> 9;

    u32 addr = ARM9_GetReg(ARM9, 15) + offset;

    ARM9_WriteReg(ARM9, 14, addr, false, false);
}

void THUMB9_BL_BLX_HI(struct ARM946E_S* ARM9)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const bool x = !(curinstr & (1<<12));

    if (x && (curinstr & 0x1)) { ARM9_UndefinedInstruction(ARM9); return; }

    u32 addr = (u64)ARM9_GetReg(ARM9, 14) + ((curinstr & 0x7FF) << 1);

    u32 lr = (ARM9_GetReg(ARM9, 15) - 2) | 1;
    ARM9_WriteReg(ARM9, 14, lr, false, false);

    if (x) addr &= ~0x3;
    else addr |= 0x1;

    ARM9_Branch(ARM9, addr, false);
}

void THUMB9_BLX_BX_Reg(struct ARM946E_S* ARM9, const u32 addr)
{
    const u16 curinstr = ARM9->Instr[0].Data;
    const bool l = curinstr & (1<<7);

    if (l)
    {
        u32 lr = (ARM9_GetReg(ARM9, 15) - 2) | 1;
        ARM9_WriteReg(ARM9, 14, lr, false, false);
    }

    ARM9_Branch(ARM9, addr, false);
}
