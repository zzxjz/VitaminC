#include <stdio.h>
#include <stdlib.h>
//#include "main.h"
#include "core/emu.h"

int main()
{
    char* error;
    if ((error = Emu_Init()))
    {
        printf("INIT ERROR: %s\n", error);
        return 0;
    }

    struct Console* console = malloc(sizeof(*console));
    if (!console)
    {
        printf("INIT ERROR: Could not allocate memory for core.\n");
        return 0;
    }

    Console_HardReset(console);

    Console_MainLoop(console);
}
