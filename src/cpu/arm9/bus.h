#pragma once

#include "../../utils.h"
#include "arm.h"

extern u8* Bios9;
extern const u32 Bios9_Size;
extern u8* ARM9WRAM;
extern const u32 ARM9WRAM_Size;
extern u8* ITCM;
extern const u32 ITCM_PhySize;
extern u8* DTCM;
extern const u32 DTCM_PhySize;

char* Bus9_Init();
void Bus9_Free();

u32 Bus9_PageTableLoad32(struct ARM946E_S* ARM9, const u32 addr);
struct Instruction Bus9_InstrLoad32(struct ARM946E_S* ARM9, u32 addr);
bool Bus9_Load32(struct ARM946E_S* ARM9, u32 addr, u32* ret);
bool Bus9_Load16(struct ARM946E_S* ARM9, u32 addr, u16* ret);
bool Bus9_Load8(struct ARM946E_S* ARM9, u32 addr, u8* ret);
bool Bus9_Store32(struct ARM946E_S* ARM9, u32 addr, const u32 val);
bool Bus9_Store16(struct ARM946E_S* ARM9, u32 addr, const u16 val);
bool Bus9_Store8(struct ARM946E_S* ARM9, u32 addr, const u8 val);
