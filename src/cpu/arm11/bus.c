#include <stdlib.h>
#include <stdio.h>
#include "interpreter.h"
#include "../../utils.h"
#include "bus.h"
#include "../shared/bus.h"

u8* Bios11;
const u32 Bios11_Size = 64 * 1024;

// io

// mpcore priv rgn io
u16 SCUControlReg;
u8 IRQDistControl;
u8 IRQPriority[224];
u8 IRQTarget[224];

char* Bus11_Init()
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

    SCUControlReg = 0x1FFE;
    IRQDistControl = 0;

    return NULL;
}

void Bus11_Free()
{
    free(Bios11);
}

u8 Bus11_Load8_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch(addr & 0x1FFF)
    {
    case 0x0000: return SCUControlReg;
        case 0x0001: return SCUControlReg>>8;
        case 0x0002: return SCUControlReg>>16;
        case 0x0003: return SCUControlReg>>24;

    case 0x1400 ... 0x140F: return ARM11->PrivRgn.IRQPriority[addr & 0xF];
    case 0x1410 ... 0x141B: return 0;
    case 0x141C: return ARM11->PrivRgn.IRQPriority[16];
    case 0x141D: return ARM11->PrivRgn.IRQPriority[17];
    case 0x141E: return ARM11->PrivRgn.IRQPriority[18];
    case 0x141F: return ARM11->PrivRgn.IRQPriority[19];
    case 0x1420 ... 0x14FF: return IRQPriority[(addr & 0xFF) - 0x20];
    case 0x1800 ... 0x181B: return 0;
    case 0x181D ... 0x181F: return (1 << ARM11->CPUID);
    case 0x1820 ... 0x18FF:
        return IRQTarget[(addr & 0xFF) - 0x20] | IRQTarget[(addr & 0xFF) - 0x1F] << 8;

    default:
        printf("UNK MPCORE PRIV RGN LOAD8: %08X %08X\n", addr, ARM11->PC);
        return 0;
    }
}

u16 Bus11_Load16_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch(addr & 0x1FFE)
    {
    case 0x0000: return SCUControlReg;
        case 0x0002: return SCUControlReg>>16;


    case 0x1400 ... 0x140E: return (ARM11->PrivRgn.IRQPriority[(addr & 0xE) + 1] << 8) | ARM11->PrivRgn.IRQPriority[(addr & 0xE) + 0];
    case 0x1410 ... 0x141A: return 0;
    case 0x141C: return (ARM11->PrivRgn.IRQPriority[17] << 8) | ARM11->PrivRgn.IRQPriority[16];
    case 0x141E: return (ARM11->PrivRgn.IRQPriority[19] << 8) | ARM11->PrivRgn.IRQPriority[18];
    case 0x1420 ... 0x14FE: return (IRQPriority[(addr & 0xFE) - 0x1F] << 8) | IRQPriority[(addr & 0xFE) - 0x20];
    case 0x1800 ... 0x181A: return 0;
    case 0x181C: return (1 << (ARM11->CPUID + 8));
    case 0x181E: return (1 << (ARM11->CPUID)) | (1 << (ARM11->CPUID + 8));
    case 0x1820 ... 0x18FE:
        return IRQTarget[(addr & 0xFE) - 0x20] | IRQTarget[(addr & 0xFE) - 0x1F] << 8;

    default:
        printf("UNK MPCORE PRIV RGN LOAD16: %08X %08X\n", addr, ARM11->PC);
        return 0;
    }
}

u32 Bus11_Load32_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch(addr & 0x1FFC)
    {
    case 0x0000: return SCUControlReg;


    case 0x0100: return ARM11->PrivRgn.IRQControl;
    case 0x010C: return 0x3FF;

    case 0x0200: if ((SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { return _ARM11[0].PrivRgn.IRQControl; } else return 0;

    case 0x0300: if ((SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { return _ARM11[1].PrivRgn.IRQControl; } else return 0;

    case 0x0400: if ((SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { return _ARM11[2].PrivRgn.IRQControl; } else return 0;

    case 0x0500: if ((SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { return _ARM11[3].PrivRgn.IRQControl; } else return 0;


    case 0x0608: return ARM11->PrivRgn.TimerControl;
    case 0x060C: return ARM11->PrivRgn.TimerIRQStat;
    case 0x0628: return ARM11->PrivRgn.WatchdogControl;
    case 0x062C: return ARM11->PrivRgn.WatchdogIRQStat;
    case 0x0630: return ARM11->PrivRgn.WatchdogResetStat;

    case 0x0708: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return _ARM11[0].PrivRgn.TimerControl; } else return 0;
    case 0x070C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return _ARM11[0].PrivRgn.TimerIRQStat; } else return 0;

    case 0x0728: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[0].PrivRgn.WatchdogControl; } else return 0;
    case 0x072C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[0].PrivRgn.WatchdogIRQStat; } else return 0;
    case 0x0730: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[0].PrivRgn.WatchdogResetStat; } else return 0;

    case 0x0808: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return _ARM11[1].PrivRgn.TimerControl; } else return 0;
    case 0x080C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return _ARM11[1].PrivRgn.TimerIRQStat; } else return 0;

    case 0x0828: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[1].PrivRgn.WatchdogControl; } else return 0;
    case 0x082C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[1].PrivRgn.WatchdogIRQStat; } else return 0;
    case 0x0830: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[1].PrivRgn.WatchdogResetStat; } else return 0;

    case 0x0908: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return _ARM11[2].PrivRgn.TimerControl; } else return 0;
    case 0x090C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return _ARM11[2].PrivRgn.TimerIRQStat; } else return 0;

    case 0x0928: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[2].PrivRgn.WatchdogControl; } else return 0;
    case 0x092C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[2].PrivRgn.WatchdogIRQStat; } else return 0;
    case 0x0930: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[2].PrivRgn.WatchdogResetStat; } else return 0;

    case 0x0A08: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return _ARM11[3].PrivRgn.TimerControl; } else return 0;
    case 0x0A0C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return _ARM11[3].PrivRgn.TimerIRQStat; } else return 0;

    case 0x0A28: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[3].PrivRgn.WatchdogControl; } else return 0;
    case 0x0A2C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[3].PrivRgn.WatchdogIRQStat; } else return 0;
    case 0x0A30: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[3].PrivRgn.WatchdogResetStat; } else return 0;

    case 0x1400 ... 0x140C:
        return ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 0] | ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 1] << 8 |
            ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 2] << 16 | ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 3] << 24;
    case 0x1410 ... 0x1418: return 0;
    case 0x141C:
        return ARM11->PrivRgn.IRQPriority[16] | ARM11->PrivRgn.IRQPriority[17] << 8 |
            ARM11->PrivRgn.IRQPriority[18] << 16 | ARM11->PrivRgn.IRQPriority[19] << 24;
    case 0x1420 ... 0x14FC:
        return IRQPriority[(addr & 0xFC) - 0x20] | IRQPriority[(addr & 0xFC) - 0x1F] << 8 |
            IRQPriority[(addr & 0xFC) - 0x1E] << 16 | IRQPriority[(addr & 0xFC) - 0x1D] << 24;
    case 0x1800 ... 0x1818: return 0;
    case 0x181C: return (1 << (ARM11->CPUID + 8)) | (1 << (ARM11->CPUID + 16)) | (1 << (ARM11->CPUID + 24));
    case 0x1820 ... 0x18FC:
        return IRQTarget[(addr & 0xFC) - 0x20] | IRQTarget[(addr & 0xFC) - 0x1F] << 8 |
        IRQTarget[(addr & 0xFC) - 0x1E] << 16 | IRQTarget[(addr & 0xFC) - 0x1D] << 24;


    default: 
        printf("UNK MPCORE PRIV RGN LOAD32: %08X %08X\n", addr, ARM11->PC);
        return 0;
    }
}

u8 Bus11_Load8_Main(struct ARM11MPCore* ARM11, const u32 addr)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        return Bios11[addr & (Bios11_Size-1)];

    if ((addr & 0xFFFFE000) == 0x17E00000)
        return Bus11_Load8_MPCorePriv(ARM11, addr);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
        return VRAM[addr - 0x18000000];

    if ((addr & 0xFFF00000) == 0x1FF00000)
        return WRAM[addr & (WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return FCRAM[1][addr & (FCRAM_Size-1)];

    printf("UNK LOAD8: %08X %08X\n", addr, ARM11->PC);
    return 0;
}

u16 Bus11_Load16_Main(struct ARM11MPCore* ARM11, const u32 addr)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        return *(u16*)&Bios11[addr & (Bios11_Size-1)];

    if ((addr & 0xFFFFE000) == 0x17E00000)
        return Bus11_Load16_MPCorePriv(ARM11, addr);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
        return *(u16*)&VRAM[addr - 0x18000000];

    if ((addr & 0xFFF00000) == 0x1FF00000)
        return *(u16*)&WRAM[addr & (WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return *(u16*)&FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return *(u16*)&FCRAM[1][addr & (FCRAM_Size-1)];

    printf("UNK LOAD16: %08X %08X\n", addr, ARM11->PC);
    return 0;
}

u32 Bus11_Load32_Main(struct ARM11MPCore* ARM11, const u32 addr)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        return *(u32*)&Bios11[addr & (Bios11_Size-1)];

    if ((addr & 0xFFFFE000) == 0x17E00000)
        return Bus11_Load32_MPCorePriv(ARM11, addr);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
        return *(u32*)&VRAM[addr - 0x18000000];

    if ((addr & 0xFFF00000) == 0x1FF00000)
        return *(u32*)&WRAM[addr & (WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return *(u32*)&FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return *(u32*)&FCRAM[1][addr & (FCRAM_Size-1)];

    printf("UNK LOAD32: %08X %08X\n", addr, ARM11->PC);
    while (true)
        ;
    return 0;
}

void Bus11_Store8_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr, const u8 val)
{
    switch(addr & 0x1FFF)
    {
    case 0x0000: SCUControlReg &= ~0xFF; SCUControlReg |= val; break;
        case 0x0001: SCUControlReg &= ~0xFF00; SCUControlReg |= (val & 0x3F) << 8; break;
        case 0x0002:
        case 0x0003: break;


    case 0x1000: IRQDistControl = val & 0x1; break;
        case 0x1001:
        case 0x1002:
        case 0x1003: break;


    case 0x1400 ... 0x140F: ARM11->PrivRgn.IRQPriority[addr & 0xF] = val & 0xF0; break;
    case 0x1410 ... 0x141B: break;
    case 0x141C: ARM11->PrivRgn.IRQPriority[16] = val & 0xF0; break;
    case 0x141D: ARM11->PrivRgn.IRQPriority[17] = val & 0xF0; break;
    case 0x141E: ARM11->PrivRgn.IRQPriority[18] = val & 0xF0; break;
    case 0x141F: ARM11->PrivRgn.IRQPriority[19] = val & 0xF0; break;
    case 0x1420 ... 0x14FF: IRQPriority[(addr & 0xFF) - 0x20] = val & 0xF0; break;

    case 0x1800 ... 0x181F: break;
    case 0x1820 ... 0x18FF:
        IRQTarget[(addr & 0xFF) - 0x20] = val & 0x0F;
        break;

    case 0x1C00 ... 0x1C03: ARM11->PrivRgn.IRQConfig[addr & 0x3] = val | 0xAA; break;
    case 0x1C04 ... 0x1C07: break;
    case 0x1C08 ... 0x1C3F: ARM11->PrivRgn.IRQConfig[(addr & 0x3C) - 4] = val; break;

    default: printf("UNK MPCORE PRIV RGN STORE8: %08X %08X %08X\n", addr, val, ARM11->PC); break;
    }
}

void Bus11_Store16_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr, u16 val)
{
    switch(addr & 0x1FFE)
    {
    case 0x0000: SCUControlReg &= ~0xFFFF; SCUControlReg |= val & 0x3FFF; break;
        case 0x0002: break;


    case 0x1000: IRQDistControl = val & 0x1; break;
        case 0x1002: break;


    case 0x1400 ... 0x140E:
        val &= 0xF0F0;
        ARM11->PrivRgn.IRQPriority[(addr & 0xE) + 0] = val;
        ARM11->PrivRgn.IRQPriority[(addr & 0xE) + 1] = val >> 8;
        break;
    case 0x1410 ... 0x141A: break;
    case 0x141C:
        val &= 0xF0F0;
        ARM11->PrivRgn.IRQPriority[16] = val;
        ARM11->PrivRgn.IRQPriority[17] = val >> 8;
        break;
    case 0x141E:
        val &= 0xF0F0;
        ARM11->PrivRgn.IRQPriority[18] = val;
        ARM11->PrivRgn.IRQPriority[19] = val >> 8;
        break;
    case 0x1420 ... 0x14FE:
        val &= 0xF0F0;
        IRQPriority[(addr & 0xFE) - 0x20] = val;
        IRQPriority[(addr & 0xFE) - 0x1F] = val >> 8;
        break;

    case 0x1800 ... 0x181F: break;
    case 0x1820 ... 0x18FE:
        val &= 0x0F0F;
        IRQTarget[(addr & 0xFE) - 0x20] = val;
        IRQTarget[(addr & 0xFE) - 0x1F] = val >> 8;
        break;

    case 0x1C00:
        ARM11->PrivRgn.IRQConfig[0] = val | 0xAA;
        ARM11->PrivRgn.IRQConfig[1] = (val>>8) | 0xAA;
        break;
    case 0x1C02:
        ARM11->PrivRgn.IRQConfig[2] = val | 0xAA;
        ARM11->PrivRgn.IRQConfig[3] = (val>>8) | 0xAA;
        break;
    case 0x1C04:
    case 0x1C06: break;
    case 0x1C08 ... 0x1C3E:
        ARM11->PrivRgn.IRQConfig[(addr & 0x3C) - 4] = val;
        ARM11->PrivRgn.IRQConfig[(addr & 0x3C) - 3] = val >> 8;
        break;

    default: printf("UNK MPCORE PRIV RGN STORE16: %08X %08X %08X\n", addr, val, ARM11->PC); break;
    }
}

void Bus11_Store32_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr, u32 val)
{
    switch(addr & 0x1FFC)
    {
    case 0x0000: SCUControlReg = val & 0x3FFF; break;


    case 0x0100: ARM11->PrivRgn.IRQControl = val & 0x1; break;

    case 0x0200: if ((SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { _ARM11[0].PrivRgn.IRQControl = val & 0x1; } break;

    case 0x0300: if ((SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { _ARM11[1].PrivRgn.IRQControl = val & 0x1; } break;

    case 0x0400: if ((SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { _ARM11[2].PrivRgn.IRQControl = val & 0x1; } break;

    case 0x0500: if ((SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { _ARM11[3].PrivRgn.IRQControl = val & 0x1; } break;


    case 0x0608: ARM11->PrivRgn.TimerControl = val & 0xFF07; break;
    case 0x060C: ARM11->PrivRgn.TimerIRQStat &= ~val; break;
    case 0x0628: ARM11->PrivRgn.WatchdogControl = (val & 0xFF0F) | (ARM11->PrivRgn.WatchdogControl & 0x8); break;
    case 0x062C: ARM11->PrivRgn.WatchdogIRQStat &= ~val; break;
    case 0x0630: ARM11->PrivRgn.WatchdogResetStat &= ~val; break;
    case 0x0634:
        if ((ARM11->PrivRgn.WatchdogDisable == 0x12345678) && (val == 0x87654321)) { ARM11->PrivRgn.WatchdogControl &= ~0x8; }
        ARM11->PrivRgn.WatchdogDisable = val;
        break;

    case 0x0708: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[0].PrivRgn.TimerControl = val & 0xFF07; } break;
    case 0x070C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[0].PrivRgn.TimerIRQStat &= ~val; } break;
    case 0x0728: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[0].PrivRgn.WatchdogControl = (val & 0xFF0F) | (_ARM11[0].PrivRgn.WatchdogControl & 0x8); } break;
    case 0x072C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[0].PrivRgn.WatchdogIRQStat &= ~val;}  break;
    case 0x0730: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[0].PrivRgn.WatchdogResetStat &= ~val; } break;
    case 0x0734: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { 
            if ((_ARM11[0].PrivRgn.WatchdogDisable == 0x12345678) && (val == 0x87654321)) { _ARM11[0].PrivRgn.WatchdogControl &= ~0x8; }
            _ARM11[0].PrivRgn.WatchdogDisable = val;
        }
        break;

    case 0x0808: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[1].PrivRgn.TimerControl = val & 0xFF07; } break;
    case 0x080C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[1].PrivRgn.TimerIRQStat &= ~val; } break;
    case 0x0828: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[1].PrivRgn.WatchdogControl = (val & 0xFF0F) | (_ARM11[0].PrivRgn.WatchdogControl & 0x8); } break;
    case 0x082C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[1].PrivRgn.WatchdogIRQStat &= ~val;}  break;
    case 0x0830: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[1].PrivRgn.WatchdogResetStat &= ~val; } break;
    case 0x0834: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { 
            if ((_ARM11[1].PrivRgn.WatchdogDisable == 0x12345678) && (val == 0x87654321)) { _ARM11[1].PrivRgn.WatchdogControl &= ~0x8; }
            _ARM11[1].PrivRgn.WatchdogDisable = val;
        }
        break;

    case 0x0908: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[2].PrivRgn.TimerControl = val & 0xFF07; } break;
    case 0x090C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[2].PrivRgn.TimerIRQStat &= ~val; } break;
    case 0x0928: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[2].PrivRgn.WatchdogControl = (val & 0xFF0F) | (_ARM11[2].PrivRgn.WatchdogControl & 0x8); } break;
    case 0x092C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[2].PrivRgn.WatchdogIRQStat &= ~val;}  break;
    case 0x0930: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[2].PrivRgn.WatchdogResetStat &= ~val; } break;
    case 0x0934: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { 
            if ((_ARM11[2].PrivRgn.WatchdogDisable == 0x12345678) && (val == 0x87654321)) { _ARM11[2].PrivRgn.WatchdogControl &= ~0x8; }
            _ARM11[2].PrivRgn.WatchdogDisable = val;
        }
        break;

    case 0x0A08: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[3].PrivRgn.TimerControl = val & 0xFF07; } break;
    case 0x0A0C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[3].PrivRgn.TimerIRQStat &= ~val; } break;
    case 0x0A28: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[3].PrivRgn.WatchdogControl = (val & 0xFF0F) | (_ARM11[3].PrivRgn.WatchdogControl & 0x8); } break;
    case 0x0A2C: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[3].PrivRgn.WatchdogIRQStat &= ~val;}  break;
    case 0x0A30: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { _ARM11[3].PrivRgn.WatchdogResetStat &= ~val; } break;
    case 0x0A34: if ((SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { 
            if ((_ARM11[3].PrivRgn.WatchdogDisable == 0x12345678) && (val == 0x87654321)) { _ARM11[3].PrivRgn.WatchdogControl &= ~0x8; }
            _ARM11[3].PrivRgn.WatchdogDisable = val;
        }
        break;
    
    case 0x1000: IRQDistControl = val & 0x1; break;

    case 0x1400 ... 0x140C:
        val &= 0xF0F0F0F0;
        ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 0] = val;
        ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 1] = val >> 8;
        ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 2] = val >> 16;
        ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 3] = val >> 24;
        break;
    case 0x1410 ... 0x1418: break;
    case 0x141C:
        val &= 0xF0F0F0F0;
        ARM11->PrivRgn.IRQPriority[16] = val;
        ARM11->PrivRgn.IRQPriority[17] = val >> 8;
        ARM11->PrivRgn.IRQPriority[18] = val >> 16;
        ARM11->PrivRgn.IRQPriority[19] = val >> 24;
        break;
    case 0x1420 ... 0x14FC:
        val &= 0xF0F0F0F0;
        IRQPriority[(addr & 0xFC) - 0x20] = val;
        IRQPriority[(addr & 0xFC) - 0x1F] = val >> 8;
        IRQPriority[(addr & 0xFC) - 0x1E] = val >> 16;
        IRQPriority[(addr & 0xFC) - 0x1D] = val >> 24;
        break;

    case 0x1800 ... 0x181F: break;
    case 0x1820 ... 0x18FC:
        val &= 0x0F0F0F0F;
        IRQTarget[(addr & 0xFC) - 0x20] = val;
        IRQTarget[(addr & 0xFC) - 0x1F] = val >> 8;
        IRQTarget[(addr & 0xFC) - 0x1E] = val >> 16;
        IRQTarget[(addr & 0xFC) - 0x1D] = val >> 24;
        break;

    case 0x1C00:
        ARM11->PrivRgn.IRQConfig[0] = val | 0xAA;
        ARM11->PrivRgn.IRQConfig[1] = (val>>8) | 0xAA;
        ARM11->PrivRgn.IRQConfig[2] = (val>>16) | 0xAA;
        ARM11->PrivRgn.IRQConfig[3] = (val>>24) | 0xAA;
        break;
    case 0x1C04: break;
    case 0x1C08 ... 0x1C3C:
        ARM11->PrivRgn.IRQConfig[(addr & 0x3C) - 4] = val;
        ARM11->PrivRgn.IRQConfig[(addr & 0x3C) - 3] = val >> 8;
        ARM11->PrivRgn.IRQConfig[(addr & 0x3C) - 2] = val >> 16;
        ARM11->PrivRgn.IRQConfig[(addr & 0x3C) - 1] = val >> 24;
        break;

    default: printf("UNK MPCORE PRIV RGN STORE32: %08X %08X %08X\n", addr, val, ARM11->PC); break;
    }
}

void Bus11_Store8_Main(struct ARM11MPCore* ARM11, const u32 addr, const u8 val)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        Bios11[addr & (Bios11_Size-1)] = val;

    else if ((addr & 0xFFFFE000) == 0x17E00000)
        Bus11_Store8_MPCorePriv(ARM11, addr, val);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
        VRAM[addr - 0x18000000] = val;

    else if ((addr & 0xFFF00000) == 0x1FF00000)
        WRAM[addr & (WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("UNK STORE8: %08X %08X %08X\n", addr, val, ARM11->PC);
}

void Bus11_Store16_Main(struct ARM11MPCore* ARM11, const u32 addr, const u16 val)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        *(u16*)&Bios11[addr & (Bios11_Size-1)] = val;

    else if ((addr & 0xFFFFE000) == 0x17E00000)
        Bus11_Store16_MPCorePriv(ARM11, addr, val);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
        *(u16*)&VRAM[addr - 0x18000000] = val;

    else if ((addr & 0xFFF00000) == 0x1FF00000)
        *(u16*)&WRAM[addr & (WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        *(u16*)&FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        *(u16*)&FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("UNK STORE16: %08X %08X %08X\n", addr, val, ARM11->PC);
}

void Bus11_Store32_Main(struct ARM11MPCore* ARM11, const u32 addr, const u32 val)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        *(u32*)&Bios11[addr & (Bios11_Size-1)] = val;

    else if ((addr & 0xFFFFE000) == 0x17E00000)
        Bus11_Store32_MPCorePriv(ARM11, addr, val);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
        *(u32*)&VRAM[addr - 0x18000000] = val;

    else if ((addr & 0xFFF00000) == 0x1FF00000)
        *(u32*)&WRAM[addr & (WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        *(u32*)&FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        *(u32*)&FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("UNK STORE32: %08X %08X %08X\n", addr, val, ARM11->PC);
}

u32 Bus11_PageTableLoad32(struct ARM11MPCore* ARM11, u32 addr)
{
    if (addr & 0x3) printf("UNALIGNED PAGE TABLE FETCH??? %08X %08X\n", addr, ARM11->PC);
    addr &= ~0x3;

    u32 val = Bus11_Load32_Main(ARM11, addr & ~3);

    return ARM11->CP15.ExceptionEndian ? __builtin_bswap32(val) : val;
}

u32 Bus11_InstrLoad32(struct ARM11MPCore* ARM11, u32 addr)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, TLB_Instr | TLB_Read);

    addr &= ~0x3;

    return Bus11_Load32_Main(ARM11, addr);
}

u32 Bus11_Load32(struct ARM11MPCore* ARM11, u32 addr)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, TLB_Read);

    if (addr & 0x3) printf("UNALIGNED LOAD32 %08X %08X\n", addr, ARM11->PC);
    addr &= ~0x3;

    return Bus11_Load32_Main(ARM11, addr);
}

u16 Bus11_Load16(struct ARM11MPCore* ARM11, u32 addr)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, TLB_Read);

    if (addr & 0x1) printf("UNALIGNED LOAD16 %08X %08X\n", addr, ARM11->PC);
    addr &= ~0x1;

    return Bus11_Load16_Main(ARM11, addr);
}

u8 Bus11_Load8(struct ARM11MPCore* ARM11, u32 addr)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, TLB_Read);

    return Bus11_Load8_Main(ARM11, addr);
}

void Bus11_Store32(struct ARM11MPCore* ARM11, u32 addr, const u32 val)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, 0);

    if (addr & 0x3) printf("UNALIGNED STORE32 %08X %08X\n", addr, ARM11->PC);
    addr &= ~0x3;

    Bus11_Store32_Main(ARM11, addr, val);
}

void Bus11_Store16(struct ARM11MPCore* ARM11, u32 addr, const u16 val)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, 0);

    if (addr & 0x1) printf("UNALIGNED STORE16 %08X %08X\n", addr, ARM11->PC);
    addr &= ~0x1;

    Bus11_Store16_Main(ARM11, addr, val);
}

void Bus11_Store8(struct ARM11MPCore* ARM11, u32 addr, const u8 val)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, 0);

    Bus11_Store8_Main(ARM11, addr, val);
}
