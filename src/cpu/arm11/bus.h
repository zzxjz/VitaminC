#pragma once

#include "../../utils.h"
#include "arm.h"

enum
{
    BusAccess_8Bit  = 0x01,
    BusAccess_16Bit = 0x02,
    BusAccess_32Bit = 0x04,
    BusAccess_Store = 0x10,
};
struct CFG11
{
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
};
struct GPUIO
{
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
};
extern u8* Bios11;

extern u8 CFG11_SWRAM[2][8];
extern struct CFG11 CFG11;
extern struct GPUIO GPUIO;

extern u16 SCUControlReg;
extern u8 IRQDistControl;
extern u8 IRQEnable[30];
extern u8 IRQPriority[224];
extern u8 IRQTarget[224];

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
