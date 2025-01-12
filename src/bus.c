#include <stdlib.h>
#include <stdio.h>
#include "cpu/arm11/interpreter.h"
#include "types.h"
#include "bus.h"

u8* Bios11;
const u32 Bios11_Size = 64 * 1024;
u8* WRAM;
const u32 WRAM_Size = 1 * 1024 * 1024;
u8* FCRAM[2];
const u32 FCRAM_Size = 128 * 1024 * 1024;
u8* VRAM;
const u32 VRAM_Size = 6 * 1024 * 1024;

char* Bus_Init()
{
    Bios11 = malloc(Bios11_Size);
    FILE* file = fopen("bios_ctr11.bin", "rb");
    if (file != NULL)
    {
        int num = fread(Bios11, 1, 0x10000, file);
        fclose(file);
        if (num != Bios11_Size)
        {
            return "bios_ctr11.bin is not a valid 3ds arm 11 bios.";
        }
    }
    else
    {
        return "bios_ctr11.bin not found.";
    }

    WRAM = calloc(1, WRAM_Size);
    FCRAM[0] = calloc(1, FCRAM_Size);
    FCRAM[1] = calloc(1, FCRAM_Size);
    VRAM = calloc(1, VRAM_Size);
    return NULL;
}

void Bus_Free()
{
    free(Bios11);
    free(WRAM);
    free(FCRAM[0]);
    free(FCRAM[1]);
    free(VRAM);
}

u32 Bus11_PageTableLoad32(const struct ARM11MPCore* ARM11, const u32 addr)
{
    u8* val = Bus11_GetPtr(addr & ~3, BusAccess_32Bit);
    if (val) return (ARM11->CP15.ExceptionEndian ? __builtin_bswap32(*(u32*)val) : *(u32*)val);
    return 0;
}

u32 Bus11_InstrLoad32(struct ARM11MPCore* ARM11, u32 addr)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, TLB_Instr | TLB_Read);

    u8* val = Bus11_GetPtr(addr & ~0x3, BusAccess_32Bit);
    if (val) return *(u32*)val;
    return 0;
}

u32 Bus11_Load32(struct ARM11MPCore* ARM11, u32 addr)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, TLB_Read);

    if (addr & 0x3) printf("UNALIGNED LOAD32\n");

    u8* val = Bus11_GetPtr(addr & ~0x3, BusAccess_32Bit);
    if (val) return *(u32*)val;
    return 0;
}

u16 Bus11_Load16(struct ARM11MPCore* ARM11, u32 addr)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, TLB_Read);

    if (addr & 0x1) printf("UNALIGNED LOAD16\n");

    u8* val = Bus11_GetPtr(addr & ~0x1, BusAccess_16Bit);
    if (val) return *(u16*)val;
    return 0;
}

u8 Bus11_Load8(struct ARM11MPCore* ARM11, u32 addr)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, TLB_Read);

    u8* val = Bus11_GetPtr(addr, BusAccess_8Bit);
    if (val) return *(u8*)val;
    return 0;
}

void Bus11_Store32(struct ARM11MPCore* ARM11, u32 addr, const u32 val)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, 0);

    if (addr & 0x3) printf("UNALIGNED STORE32\n");

    u32* ptr = (u32*)Bus11_GetPtr(addr & ~0x3, BusAccess_Store | BusAccess_32Bit);
    if (ptr) *ptr = val;
}

void Bus11_Store16(struct ARM11MPCore* ARM11, u32 addr, const u16 val)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, 0);

    if (addr & 0x1) printf("UNALIGNED STORE16\n");

    u16* ptr = (u16*)Bus11_GetPtr(addr & ~0x1, BusAccess_Store | BusAccess_16Bit);
    if (ptr) *ptr = val;
}

void Bus11_Store8(struct ARM11MPCore* ARM11, u32 addr, const u8 val)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, 0);

    u8* ptr = Bus11_GetPtr(addr, BusAccess_Store | BusAccess_8Bit);
    if (ptr) *ptr = val;
}

u8* Bus11_GetPtr(const u32 addr, const u8 accesstype)
{
    if (!(accesstype & BusAccess_Store) && ((addr < 0x20000) || (addr >= 0xFFFF0000)))
        return &Bios11[addr & (Bios11_Size-1)];

    if ((addr & 0xFFF00000) == 0x1FF00000)
        return &WRAM[addr & (WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return &FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return &FCRAM[1][addr & (FCRAM_Size-1)];

    printf("UNK ACCESS: %08X %X\n", addr, accesstype);
    return NULL;
}
