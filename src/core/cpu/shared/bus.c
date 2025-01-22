#include <string.h>
#include "bus.h"
#include "../../emu.h"

void MapSWRAM(struct Console* console, const bool data, const u8 bank, const u8 val)
{
    // todo: figure out what happens when multiple banks are mapped to the same address? contents ORed together? One takes priority?

    console->CFG11_SWRAM[data][bank] = val;

    memset(console->SWRAM_Lookup, 0, sizeof(console->SWRAM_Lookup));
    for (int i = 0; i < 8; i++)
    {
        const u8 cfg = console->CFG11_SWRAM[data][i];
        const bool enable = cfg & 0x8;
        const bool DSP = cfg & 0x1; // TODO
        const u8 slot = (cfg >> 2) & 0x7;

        if (!enable) continue;

        console->SWRAM_Lookup[data][slot] = console->SWRAM[data][i];
    }
}

u8* GetSWRAM(struct Console* console, const u32 addr)
{
    const bool data = (addr >> 18) & 0x1;
    const u8 bank = (addr >> 15) & 0x7;

    return console->SWRAM_Lookup[data][bank];
}
