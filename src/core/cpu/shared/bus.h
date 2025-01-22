#pragma once

#include "../../utils.h"

enum : u8
{
    BusAccess_8Bit  = 0x01,
    BusAccess_16Bit = 0x02,
    BusAccess_32Bit = 0x04,
    BusAccess_Store = 0x10,
};

#define SWRAM_Size      (32 * 1024)
#define AXIWRAM_Size    (512 * 1024)
#define FCRAM_Size      (128 * 1024 * 1024)
#define VRAM_Size       (6 * 1024 * 1024)

#include "../../emu.h"

void MapSWRAM(struct Console* console, const bool data, const u8 bank, const u8 val);
u8* GetSWRAM(struct Console* console, const u32 addr);
