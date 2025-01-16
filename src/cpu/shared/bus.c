#include <stdlib.h>
#include "bus.h"
#include "../arm11/bus.h"


u8* WRAM;
const u32 WRAM_Size = 1 * 1024 * 1024;

u8* FCRAM[2];
const u32 FCRAM_Size = 128 * 1024 * 1024;

u8* VRAM;
const u32 VRAM_Size = 6 * 1024 * 1024;

char* Bus_Init()
{
    static char* error = NULL;
    WRAM = calloc(1, WRAM_Size);
    FCRAM[0] = calloc(1, FCRAM_Size);
    FCRAM[1] = calloc(1, FCRAM_Size);
    VRAM = calloc(1, VRAM_Size);

    if ((error = Bus11_Init())) return error;

    return NULL;
}

void Bus_Free()
{
    free(WRAM);
    free(FCRAM[0]);
    free(FCRAM[1]);
    free(VRAM);

    Bus11_Free();
}
