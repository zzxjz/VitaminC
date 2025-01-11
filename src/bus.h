#pragma once

#include "types.h"
#include "cpu/arm11/interpreter.h"

extern u8* Bios11;
extern u8* WRAM;
extern u8* FCRAM[2];
extern u8* VRAM;

char* Bus_Init();
void Bus_Free();

u32 Bus11_PageTableLoad32(const struct ARM11MPCore* ARM11, const u32 addr);
u32 Bus11_Load32(const u32 addr);
u8 Bus11_Load8(const u32 addr);
void Bus11_Store32(const u32 addr, const u32 val);
void Bus11_Store8(const u32 addr, const u8 val);
u8* Bus11_GetPtrWrite(const u32 addr);
u8* Bus11_GetPtrLoad(const u32 addr);
