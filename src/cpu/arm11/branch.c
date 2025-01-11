#include "interpreter.h"
#include "../../types.h"

void ARM11_B_BL(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;
    const bool l = curinstr & (1<<24);

    if (l)
    {
        ARM11->LR = ARM11->PC;
    }

    u32 target = curinstr;
    target <<= 8;
    target = (s32)target >> 6;
    ARM11_Branch(ARM11, ARM11_GetReg(ARM11, 15) + (s32)target, false);
}

void ARM_BX(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;

    ARM11_Branch(ARM11, ARM11_GetReg(ARM11, curinstr & 0xF), false);
}
