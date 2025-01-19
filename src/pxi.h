#include "utils.h"

extern u32 PXI_Sync[2];

void PXISync_WriteSend(const u8 val, bool cpu);
void PXI9Sync_WriteIRQ(const u8 val);
void PXI11Sync_WriteIRQ(const u8 val);
