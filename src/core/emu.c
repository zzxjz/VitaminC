#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "utils.h"
#include "emu.h"
#include "cpu/arm9/arm.h"
#include "cpu/arm11/arm.h"
#include "cpu/arm11/bus.h"

u8* Bios11;
u8* Bios9;

char* Emu_Init()
{
    // calculate instruction lookup tables
    for (u16 i = 0; i < 0x1000; i++) ARM11_InstrLUT[i] = ARM11_InitARMInstrLUT(i);
    for (u16 i = 0; i < 0x40; i++) THUMB11_InstrLUT[i] = ARM11_InitTHUMBInstrLUT(i);

    for (u16 i = 0; i < 0x1000; i++) ARM9_InstrLUT[i] = ARM9_InitARMInstrLUT(i);
    for (u16 i = 0; i < 0x40; i++) THUMB9_InstrLUT[i] = ARM9_InitTHUMBInstrLUT(i);

    // allocate and load bios files
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

    Bios9 = malloc(Bios9_Size);
    file = fopen("bios_ctr9.bin", "rb");
    if (file != NULL)
    {
        int num = fread(Bios9, 1, 0x10000, file);
        fclose(file);
        if (num != Bios9_Size)
        {
            return "bios_ctr9.bin is not a valid 3ds arm 9 bios.";
        }
    }
    else
    {
        return "bios_ctr9.bin not found.";
    }

    return NULL;
}

void Console_HardReset(struct Console* console)
{
    memset(console, 0, sizeof(*console));

    console->SOCInfo = 0x7;
    console->MPCore.SCUControlReg = 0x1FFE;
    console->VRAMPower.Data = 0xFFFFFFFF;

    ARM11_HardReset(console);
    ARM9_HardReset(console);
}

void Console_MainLoop(struct Console* console)
{
    while (true)
    {
        console->SystemTimestamp += 1;
        //printf("CPU0 TIME!\n");
        ARM11_RunInterpreter(&console->ARM11[0], console->SystemTimestamp);
        /*printf("CPU1 TIME!\n");
        ARM11_RunInterpreter(&console->ARM11[1], target);
        printf("CPU2 TIME!\n");
        ARM11_RunInterpreter(&console->ARM11[2], target);
        printf("CPU3 TIME!\n");
        ARM11_RunInterpreter(&console->ARM11[3], target);
        printf("CPU9 TIME!\n");
        */
        ARM9_RunInterpreter(&console->ARM9, console->SystemTimestamp);
    };
}
