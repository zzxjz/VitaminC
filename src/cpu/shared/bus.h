#include "../../utils.h"

extern u8* SWRAM[2][8];
extern const u32 SWRAM_Size;
extern u8* AXI_WRAM;
extern const u32 AXI_WRAM_Size;
extern u8* FCRAM[2];
extern const u32 FCRAM_Size;
extern u8* VRAM;
extern const u32 VRAM_Size;

extern const u8 SOCInfo;

char* Bus_Init();
void Bus_Free();

void MapSWRAM(const bool data, const u8 bank, const u8 val);
u8* GetSWRAM(const u32 addr);