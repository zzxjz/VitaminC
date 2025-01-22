#pragma once

#include "../../utils.h"
#include "arm.h"

#define Bios11_Size (64 * 1024)

u32 Bus11_PageTableLoad32(struct ARM11MPCore* ARM11, const u32 addr);
u32 Bus11_InstrLoad32(struct ARM11MPCore* ARM11, u32 addr);
u32 Bus11_Load32(struct ARM11MPCore* ARM11, u32 addr);
u16 Bus11_Load16(struct ARM11MPCore* ARM11, u32 addr);
u8 Bus11_Load8(struct ARM11MPCore* ARM11, u32 addr);
void Bus11_Store32(struct ARM11MPCore* ARM11, u32 addr, const u32 val);
void Bus11_Store16(struct ARM11MPCore* ARM11, u32 addr, const u16 val);
void Bus11_Store8(struct ARM11MPCore* ARM11, u32 addr, const u8 val);
