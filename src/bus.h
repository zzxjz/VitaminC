#pragma once

#include "types.h"

extern u8* Bios11;

char* Bus_Init();
void Bus_Free();

u8* Bus_GetPtr(const u32 addr);
