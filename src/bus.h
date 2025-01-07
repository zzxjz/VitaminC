#pragma once

#include "types.h"

extern u8* Bios11;

char* Bus_Init();
void Bus_Free();

u32 Bus11_Load32(const u32 addr);
u8 Bus11_Load8(const u32 addr);
u8* Bus11_GetPtr(const u32 addr);
