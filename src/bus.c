#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "bus.h"

u8* Bios11;

char* Bus_Init()
{
    Bios11 = malloc(0x10000);
    FILE* file = fopen("bios_ctr11.bin", "rb");
    if (file != NULL)
    {
        int num = fread(Bios11, 1, 0x10000, file);
        fclose(file);
        if (num != 0x10000)
        {
            return "bios_ctr11.bin is not a valid 3ds arm 11 bios.";
        }
    }
    else
    {
        return "bios_ctr11.bin not found.";
    }
    return NULL;
}

void Bus_Free()
{
    free(Bios11);
}

u32 Bus11_Load32(const u32 addr)
{
    u8* val = Bus11_GetPtr(addr & ~3);
    if (val) return *(u32*)val;
    return 0;
}

u8 Bus11_Load8(const u32 addr)
{
    u8* val = Bus11_GetPtr(addr & ~3);
    if (val) return *(u8*)val;
    return 0;
}

u8* Bus11_GetPtr(const u32 addr)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        return &Bios11[addr & 0xFFFF];

    printf("UNK ADDR: %08X", addr);
    return NULL;
}
