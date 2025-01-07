#include <stdio.h>
#include "../interpreter.h"
#include "../../../types.h"

void ARM11_MCR(struct ARM11MPCore* ARM11) // WRITE
{
    const u32 curinstr = ARM11->Instr.Data;
    const u32 coproc = (curinstr >> 8) & 0xF;
    const u32 op1 = (curinstr >> 21) & 0x7;
    const u32 rd = (curinstr >> 12) & 0xF;
    const u32 crn = (curinstr >> 16) & 0xF;
    const u32 crm = curinstr & 0xF;
    const u32 op2 = (curinstr >> 5) & 0x7;

    const u16 cmd = op1 << 12 | crn << 8 | crm << 4 | op2;

    switch(coproc)
    {
    case 15:
        ARM11_CP15_Store(ARM11, cmd, ARM11_GetReg(ARM11, rd));
        break;
    default:
        printf("UNIMPLEMENTED COPROC: %02X MCR: %08X\n", coproc, curinstr);
        break;
    }
}

void ARM11_MRC(struct ARM11MPCore* ARM11) // READ
{

}
