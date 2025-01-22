#pragma once

#include "cpu/arm11/arm.h"
#include "cpu/arm9/arm.h"
#include "cpu/shared/bus.h"
#include "cpu/arm9/bus.h"

struct Console
{
    // main components
    struct ARM11MPCore ARM11[4];
    struct ARM946E_S ARM9;

    // sys
    u64 SystemTimestamp;

    // io
    u8 SOCInfo;
    union
    {
        u32 Data;
        struct
        {
            bool GPURegVRAM_Enable : 1;
            bool Memfill_Enable : 1;
            bool InternalReg_Enable0 : 1;
            bool InternalReg_Enable1 : 1;
            bool Memcpy_Enable : 1;
            bool Hang_Disable : 1;
            bool LCD_Enable : 1;
            u32 : 9;
            bool Backlight_Enable : 1;
        } Sub;
    } GPUCnt;

    union
    {
        u32 Data;
        struct
        {
            u32 : 8;
            bool VRAMA_Lo_Disable : 1;
            bool VRAMA_Hi_Disable : 1;
            bool VRAMB_Lo_Disable : 1;
            bool VRAMB_Hi_Disable : 1;
        } Sub;
    } VRAMPower;

    u32 PXI_Sync[2];

    u8 CFG11_SWRAM[2][8];

    struct
    {
        u16 SCUControlReg;
        u8 IRQDistControl;
        u8 IRQEnable[30];
        u8 IRQPriority[224];
        u8 IRQTarget[224];
    } MPCore;

    u8* SWRAM_Lookup[2][8];

    alignas(64)
    u8 ARM9WRAM[ARM9WRAM_Size];
    u8 SWRAM[2][8][SWRAM_Size];
    u8 AXIWRAM[AXIWRAM_Size];
    u8 FCRAM[2][FCRAM_Size];
    u8 VRAM[VRAM_Size];
};

extern u8* Bios11;
extern u8* Bios9;

char* Emu_Init();

void Console_HardReset(struct Console* console);
void Console_MainLoop(struct Console* console);
