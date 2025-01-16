#include "../../utils.h"

extern u8* WRAM;
extern const u32 WRAM_Size;
extern u8* FCRAM[2];
extern const u32 FCRAM_Size;
extern u8* VRAM;
extern const u32 VRAM_Size;

char* Bus_Init();
void Bus_Free();
