#pragma once

#include "../../utils.h"
#include "arm.h"

#define Bios9_Size      (64 * 1024)
#define ARM9WRAM_Size   (1 * 1024 * 1024)

u32 Bus9_PageTableLoad32(struct ARM946E_S* ARM9, const u32 addr);
struct Instruction Bus9_InstrLoad32(struct ARM946E_S* ARM9, u32 addr);
bool Bus9_Load32(struct ARM946E_S* ARM9, u32 addr, u32* ret);
bool Bus9_Load16(struct ARM946E_S* ARM9, u32 addr, u16* ret);
bool Bus9_Load8(struct ARM946E_S* ARM9, u32 addr, u8* ret);
bool Bus9_Store32(struct ARM946E_S* ARM9, u32 addr, const u32 val);
bool Bus9_Store16(struct ARM946E_S* ARM9, u32 addr, const u16 val);
bool Bus9_Store8(struct ARM946E_S* ARM9, u32 addr, const u8 val);
