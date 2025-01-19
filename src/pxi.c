#include <stdio.h>
#include "utils.h"
#include "pxi.h"

u32 PXI_Sync[2];

void PXISync_WriteSend(const u8 val, bool cpu)
{
    PXI_Sync[!cpu] &= 0x80000000;
    PXI_Sync[!cpu] |= val;
}

void PXI9Sync_WriteIRQ(const u8 val)
{
    if (val & 0x20) printf("ARM9 SENDING IRQ 0x50 TO ARM11!!!\n");
    if (val & 0x40) printf("ARM9 SENDING IRQ 0x51 TO ARM11!!!\n");
    PXI_Sync[ARM9ID] &= 0xFF;
    PXI_Sync[ARM9ID] |= (val & 0x80) << 24;
}

void PXI11Sync_WriteIRQ(const u8 val)
{
    if (val & 0x40) printf("ARM11 SENDING IRQ 12 TO ARM9!!!\n");
    PXI_Sync[ARM9ID] &= 0xFF;
    PXI_Sync[ARM9ID] |= (val & 0x80) << 24;
}
