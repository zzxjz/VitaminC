#include <stdlib.h>
#include <string.h>
#include "bus.h"
#include "../arm11/bus.h"


u8* SWRAM[2][8];
const u32 SWRAM_Size = 32 * 1024;
u8* SWRAM_Lookup[2][8];

u8* AXI_WRAM;
const u32 AXI_WRAM_Size = 512 * 1024;

u8* FCRAM[2];
const u32 FCRAM_Size = 128 * 1024 * 1024;

u8* VRAM;
const u32 VRAM_Size = 6 * 1024 * 1024;

const u8 SOCInfo = 7;

char* Bus_Init()
{
    static char* error = NULL;
    AXI_WRAM = calloc(1, AXI_WRAM_Size);
    FCRAM[0] = calloc(1, FCRAM_Size);
    FCRAM[1] = calloc(1, FCRAM_Size);
    VRAM = calloc(1, VRAM_Size);
    for (int s = 0; s < 2; s++)
        for (int b = 0; b < 8; b++)
            SWRAM[s][b] = calloc(1, SWRAM_Size);

    if ((error = Bus11_Init())) return error;

    return NULL;
}

void Bus_Free()
{
    free(AXI_WRAM);
    free(FCRAM[0]);
    free(FCRAM[1]);
    free(VRAM);

    Bus11_Free();
}

void MapSWRAM(const bool data, const u8 bank, const u8 val)
{
    // todo: figure out what happens when multiple banks are mapped to the same address? contents ORed together? One takes priority?

    CFG11_SWRAM[data][bank] = val;

    memset(SWRAM_Lookup, 0, sizeof(SWRAM_Lookup));
    for (int i = 0; i < 8; i++)
    {
        const u8 cfg = CFG11_SWRAM[data][i];
        const bool enable = cfg & 0x8;
        const bool DSP = cfg & 0x1; // TODO
        const u8 slot = (cfg >> 2) & 0x7;

        if (!enable) continue;

        SWRAM_Lookup[data][slot] = SWRAM[data][i];
    }
}

u8* GetSWRAM(const u32 addr)
{
    const bool data = (addr >> 18) & 0x1;
    const u8 bank = (addr >> 15) & 0x7;

    return SWRAM_Lookup[data][bank];
}
