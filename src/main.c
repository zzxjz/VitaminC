#include <stdio.h>
//#include "main.h"
#include "cpu/arm11/interpreter.h"
#include "bus.h"

char* Emu_Init()
{
    for (u16 i = 0; i < 0x1000; i++) ARM11_InstrLUT[i] = ARM11_InitInstrLUT(i);
    static char* error = NULL;
    if ((error = ARM11_Init())) return error;
    if ((error = Bus_Init())) return error;
    return NULL;
}

void Emu_Free()
{
    Bus_Free();
}

int main()
{
    char* error;
    if ((error = Emu_Init()))
    {
        printf("INIT ERROR: %s\n", error);
        return 0;
    }

    u64 target = 8;
    while (true)
    {
        printf("CPU0 TIME!\n");
        ARM11_RunInterpreter(&ARM11[0], target);
        /*printf("CPU1 TIME!\n");
        ARM11_RunInterpreter(&ARM11[1], target);
        printf("CPU2 TIME!\n");
        ARM11_RunInterpreter(&ARM11[2], target);
        printf("CPU3 TIME!\n");
        ARM11_RunInterpreter(&ARM11[3], target);
        */target += 8;
    }

    Emu_Free();
}
