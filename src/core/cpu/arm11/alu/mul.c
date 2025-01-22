#include "../../../utils.h"
#include "../arm.h"

void ARM11_MUL_MLA(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;
    const int rm = curinstr & 0xF;
    const int rs = (curinstr >> 8) & 0xF;
    const int rn = (curinstr >> 12) & 0xF;
    const int rd = (curinstr >> 16) & 0xF;
    const bool a = curinstr & (1<<21);
    const bool s = curinstr & (1<<20);

    u32 mul_out = ARM11_GetReg(ARM11, rm) * ARM11_GetReg(ARM11, rs); 
    if (a) mul_out += ARM11_GetReg(ARM11, rn);
    if (s)
    {
        ARM11->Negative = mul_out >> 31;
        ARM11->Zero = !mul_out;
    }
    ARM11_WriteReg(ARM11, rd, mul_out, false, false);
}

void THUMB11_MULS(struct ARM11MPCore* ARM11, const int rd, const u32 rdval, const u32 rmval)
{
    const u32 mul_out = rdval * rmval;
    ARM11->Negative = mul_out >> 31;
    ARM11->Zero = !mul_out;
    ARM11_WriteReg(ARM11, rd, mul_out, false, false);
}
