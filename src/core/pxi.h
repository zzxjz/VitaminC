#include "utils.h"
#include "emu.h"

void PXISync_WriteSend(struct Console* console, const u8 val, bool cpu);
void PXI9Sync_WriteIRQ(struct Console* console, const u8 val);
void PXI11Sync_WriteIRQ(struct Console* console, const u8 val);
