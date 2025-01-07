#include <stdio.h>
#include "../interpreter.h"
#include "../../../types.h"

void ARM11_CP15_Store(struct ARM11MPCore* ARM11, u16 cmd, u32 val)
{
    switch(cmd)
    {
    case 0x0100:
    {
        const u32 changemask = 0x3AC0BB03;
        ARM11->CP15.Control &= ~changemask;
        ARM11->CP15.Control |= val & changemask;
        printf("CP15 CONTROL REG WRITE!!!\n");
        break;
    }
    case 0x0101:
    {
        ARM11->CP15.AuxControl = val & 0x7F;
        printf("CP15 AUX CONTROL REG WRITE!!!\n");
        break;
    }
    case 0x0102:
    {
        ARM11->CP15.CoprocAccessControl = val & (0xF<<20); // checkme?
        printf("CP15 COPROCESSOR ACCESS CONTROL REG WRITE!!!\n");
        break;
    }
    }
}