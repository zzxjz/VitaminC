#pragma once

#include "../../utils.h"
#include "interpreter.h"

extern u8* Bios11;
extern u16 SCUControlReg;
extern u8 IRQDistControl;
extern u8 CFG11_SWRAM[2][8];

enum
{
    BusAccess_8Bit  = 0x01,
    BusAccess_16Bit = 0x02,
    BusAccess_32Bit = 0x04,
    BusAccess_Store = 0x10,
};

char* Bus11_Init();
void Bus11_Free();

u32 Bus11_PageTableLoad32(struct ARM11MPCore* ARM11, const u32 addr);
u32 Bus11_InstrLoad32(struct ARM11MPCore* ARM11, u32 addr);
u32 Bus11_Load32(struct ARM11MPCore* ARM11, u32 addr);
u16 Bus11_Load16(struct ARM11MPCore* ARM11, u32 addr);
u8 Bus11_Load8(struct ARM11MPCore* ARM11, u32 addr);
void Bus11_Store32(struct ARM11MPCore* ARM11, u32 addr, const u32 val);
void Bus11_Store16(struct ARM11MPCore* ARM11, u32 addr, const u16 val);
void Bus11_Store8(struct ARM11MPCore* ARM11, u32 addr, const u8 val);
