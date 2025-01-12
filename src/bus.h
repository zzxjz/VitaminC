#pragma once

#include "types.h"
#include "cpu/arm11/interpreter.h"

extern u8* Bios11;
extern u8* WRAM;
extern u8* FCRAM[2];
extern u8* VRAM;

enum
{
    BusAccess_8Bit  = 0x01,
    BusAccess_16Bit = 0x02,
    BusAccess_32Bit = 0x04,
    BusAccess_Store = 0x10,
};

char* Bus_Init();
void Bus_Free();

u32 Bus11_PageTableLoad32(const struct ARM11MPCore* ARM11, const u32 addr);
u32 Bus11_InstrLoad32(struct ARM11MPCore* ARM11, u32 addr);
u32 Bus11_Load32(struct ARM11MPCore* ARM11, u32 addr);
u16 Bus11_Load16(struct ARM11MPCore* ARM11, u32 addr);
u8 Bus11_Load8(struct ARM11MPCore* ARM11, u32 addr);
void Bus11_Store32(struct ARM11MPCore* ARM11, u32 addr, const u32 val);
void Bus11_Store16(struct ARM11MPCore* ARM11, u32 addr, const u16 val);
void Bus11_Store8(struct ARM11MPCore* ARM11, u32 addr, const u8 val);
u8* Bus11_GetPtr(const u32 addr, const u8 accesstype);
