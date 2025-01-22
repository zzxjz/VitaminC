#include <stdio.h>
#include "../arm.h"
#include "../../../utils.h"

void ARM9_MCR_MRC(struct ARM946E_S* ARM9) // WRITE
{
    const u32 curinstr = ARM9->Instr[0].Data;
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
            u32 val = ARM9_CP15_Load_Single(ARM9, cmd);
            if (rd == 15)
            {
                ARM9->Flags &= ~0xF0000000;
                ARM9->Flags |= val & 0xF0000000;
            }
            else ARM9->R[rd] = val;
        }
        else ARM9_CP15_Store_Single(ARM9, cmd, ARM9_GetReg(ARM9, rd));
        break;
    default:
        printf("UNIMPLEMENTED COPROC: %02X MCR: %08X\n", coproc, curinstr);
        break;
    }
}

void ARM9_MRC(struct ARM946E_S* ARM9) // READ
{

}
