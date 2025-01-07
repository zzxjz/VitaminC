#include "interpreter.h"

void ARM11_B_BL(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;
    const bool l = curinstr & (1<<24);

    if (l)
    {
        ARM11->LR = ARM11->PC - 4;
    }

    u32 target = curinstr;
    target <<= 8;
    target = (s32)target >> 6;
    ARM11_Branch(ARM11, ARM11_GetReg(ARM11, 15) + (s32)target);
}