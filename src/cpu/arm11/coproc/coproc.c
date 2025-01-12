#include <stdio.h>
#include "../interpreter.h"
#include "../../../utils.h"

void ARM11_MCR_MRC(struct ARM11MPCore* ARM11) // WRITE
{
    const u32 curinstr = ARM11->Instr.Data;
    const u32 coproc = (curinstr >> 8) & 0xF;
    const u32 op1 = (curinstr >> 21) & 0x7;
    const u32 rd = (curinstr >> 12) & 0xF;
    const u32 crn = (curinstr >> 16) & 0xF;
    const u32 crm = curinstr & 0xF;
    const u32 op2 = (curinstr >> 5) & 0x7;
    const u32 load = curinstr & (1<<20);

    const u16 cmd = op1 << 12 | crn << 8 | crm << 4 | op2;

    switch(coproc)
    {
    case 15:
        if (load)
        {
            u32 val = ARM11_CP15_Load_Single(ARM11, cmd);
            if (rd == 15)
            {
                ARM11->Flags &= ~0xF0000000;
                ARM11->Flags |= val & 0xF0000000;
            }
            else ARM11->R[rd] = val;
        }
        else ARM11_CP15_Store_Single(ARM11, cmd, ARM11_GetReg(ARM11, rd));
        break;
    default:
        printf("UNIMPLEMENTED COPROC: %02X MCR: %08X\n", coproc, curinstr);
        break;
    }
}

void ARM11_MRC(struct ARM11MPCore* ARM11) // READ
{

}
