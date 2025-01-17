#include "../../../utils.h"
#include "../arm.h"

void ARM9_MUL_MLA(struct ARM946E_S* ARM9)
{
    const u32 curinstr = ARM9->Instr[0].Data;
    const int rm = curinstr & 0xF;
    const int rs = (curinstr >> 8) & 0xF;
    const int rn = (curinstr >> 12) & 0xF;
    const int rd = (curinstr >> 16) & 0xF;
    const bool a = curinstr & (1<<21);
    const bool s = curinstr & (1<<20);

    u32 mul_out = ARM9_GetReg(ARM9, rm) * ARM9_GetReg(ARM9, rs); 
    if (a) mul_out += ARM9_GetReg(ARM9, rn);
    if (s)
    {
        ARM9->Negative = mul_out >> 31;
        ARM9->Zero = !mul_out;
    }
    ARM9_WriteReg(ARM9, rd, mul_out, false, false);
}

void THUMB9_MULS(struct ARM946E_S* ARM9, const int rd, const u32 rdval, const u32 rmval)
{
    const u32 mul_out = rdval * rmval;
    ARM9->Negative = mul_out >> 31;
    ARM9->Zero = !mul_out;
    ARM9_WriteReg(ARM9, rd, mul_out, false, false);
}
