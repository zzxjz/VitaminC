#include <stdio.h>
#include "arm.h"
#include "../../utils.h"
#include "bus.h"
#include "../shared/bus.h"
#include "../../pxi.h"

#define c3ds ARM11->console

u8 Bus11_Load8_IO(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch (addr)
    {
    case 0x10163000: return c3ds->PXI_Sync[ARM11ID] & 0xFF;
    case 0x10163001:
    case 0x10163002: return 0;
    case 0x10163003: return c3ds->PXI_Sync[ARM11ID] >> 24;

    default: printf("ARM11 - UNK IO LOAD8: %08X %08X\n", addr, ARM11->PC); return 0;
    }
}

u16 Bus11_Load16_IO(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch(addr)
    {
    case 0x10140FFC: return c3ds->SOCInfo;

    case 0x10163000: return c3ds->PXI_Sync[ARM11ID] & 0xFFFF;
    case 0x10163002: return c3ds->PXI_Sync[ARM11ID] >> 16;

    default: printf("ARM11 - UNK IO LOAD16: %08X %08X\n", addr, ARM11->PC); return 0;
    }
}

u32 Bus11_Load32_IO(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch(addr)
    {
    case 0x10141200: return c3ds->GPUCnt.Data;

    case 0x10163000: return c3ds->PXI_Sync[ARM11ID];

    case 0x10400030: return c3ds->VRAMPower.Data;

    default: printf("ARM11 - UNK IO LOAD32: %08X %08X\n", addr, ARM11->PC); return 0;
    }
}

u8 Bus11_Load8_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch(addr & 0x1FFF)
    {
    case 0x0000: return c3ds->MPCore.SCUControlReg;
        case 0x0001: return c3ds->MPCore.SCUControlReg>>8;
        case 0x0002: return c3ds->MPCore.SCUControlReg>>16;
        case 0x0003: return c3ds->MPCore.SCUControlReg>>24;

    case 0x1100 ... 0x1101:
        case 0x1180 ... 0x1181: return 0xFF;
        case 0x1102 ... 0x111F:
        case 0x1182 ... 0x119F: return c3ds->MPCore.IRQEnable[(addr & 0x1F)-2];
    case 0x1400 ... 0x140F: return ARM11->PrivRgn.IRQPriority[addr & 0xF];
        case 0x1410 ... 0x141B: return 0;
        case 0x141C: return ARM11->PrivRgn.IRQPriority[16];
        case 0x141D: return ARM11->PrivRgn.IRQPriority[17];
        case 0x141E: return ARM11->PrivRgn.IRQPriority[18];
        case 0x141F: return ARM11->PrivRgn.IRQPriority[19];
        case 0x1420 ... 0x14FF: return c3ds->MPCore.IRQPriority[(addr & 0xFF) - 0x20];
    case 0x1800 ... 0x181B: return 0;
        case 0x181D ... 0x181F: return (1 << ARM11->CPUID);
        case 0x1820 ... 0x18FF: return c3ds->MPCore.IRQTarget[(addr & 0xFF) - 0x20] | c3ds->MPCore.IRQTarget[(addr & 0xFF) - 0x1F] << 8;

    default:
        printf("ARM11 - UNK MPCORE PRIV RGN LOAD8: %08X %08X\n", addr, ARM11->PC);
        return 0;
    }
}

u16 Bus11_Load16_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch(addr & 0x1FFE)
    {
    case 0x0000: return c3ds->MPCore.SCUControlReg;
        case 0x0002: return c3ds->MPCore.SCUControlReg>>16;


    case 0x1100:
        case 0x1180: return 0xFFFF;
        case 0x1102 ... 0x111E:
        case 0x1182 ... 0x119E: return c3ds->MPCore.IRQEnable[(addr & 0x1E)-2] | c3ds->MPCore.IRQEnable[(addr & 0x1E)-1] << 8;
    case 0x1400 ... 0x140E: return (ARM11->PrivRgn.IRQPriority[(addr & 0xE) + 1] << 8) | ARM11->PrivRgn.IRQPriority[(addr & 0xE) + 0];
        case 0x1410 ... 0x141A: return 0;
        case 0x141C: return (ARM11->PrivRgn.IRQPriority[17] << 8) | ARM11->PrivRgn.IRQPriority[16];
        case 0x141E: return (ARM11->PrivRgn.IRQPriority[19] << 8) | ARM11->PrivRgn.IRQPriority[18];
        case 0x1420 ... 0x14FE: return (c3ds->MPCore.IRQPriority[(addr & 0xFE) - 0x1F] << 8) | c3ds->MPCore.IRQPriority[(addr & 0xFE) - 0x20];
    case 0x1800 ... 0x181A: return 0;
        case 0x181C: return (1 << (ARM11->CPUID + 8));
        case 0x181E: return (1 << (ARM11->CPUID)) | (1 << (ARM11->CPUID + 8));
        case 0x1820 ... 0x18FE: return c3ds->MPCore.IRQTarget[(addr & 0xFE) - 0x20] | c3ds->MPCore.IRQTarget[(addr & 0xFE) - 0x1F] << 8;

    default:
        printf("ARM11 - UNK MPCORE PRIV RGN LOAD16: %08X %08X\n", addr, ARM11->PC);
        return 0;
    }
}

u32 Bus11_Load32_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch(addr & 0x1FFC)
    {
    case 0x0000: return c3ds->MPCore.SCUControlReg;


    case 0x0100: return ARM11->PrivRgn.IRQControl;
    case 0x010C: return 0x3FF;

    case 0x0200: if ((c3ds->MPCore.SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[0].PrivRgn.IRQControl; } else return 0;

    case 0x0300: if ((c3ds->MPCore.SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[1].PrivRgn.IRQControl; } else return 0;

    case 0x0400: if ((c3ds->MPCore.SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[2].PrivRgn.IRQControl; } else return 0;

    case 0x0500: if ((c3ds->MPCore.SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[3].PrivRgn.IRQControl; } else return 0;


    case 0x0608: return ARM11->PrivRgn.TimerControl;
    case 0x060C: return ARM11->PrivRgn.TimerIRQStat;
    case 0x0628: return ARM11->PrivRgn.WatchdogControl;
    case 0x062C: return ARM11->PrivRgn.WatchdogIRQStat;
    case 0x0630: return ARM11->PrivRgn.WatchdogResetStat;

    case 0x0708: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[0].PrivRgn.TimerControl; } else return 0;
    case 0x070C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[0].PrivRgn.TimerIRQStat; } else return 0;

    case 0x0728: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[0].PrivRgn.WatchdogControl; } else return 0;
    case 0x072C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[0].PrivRgn.WatchdogIRQStat; } else return 0;
    case 0x0730: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[0].PrivRgn.WatchdogResetStat; } else return 0;

    case 0x0808: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[1].PrivRgn.TimerControl; } else return 0;
    case 0x080C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[1].PrivRgn.TimerIRQStat; } else return 0;

    case 0x0828: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[1].PrivRgn.WatchdogControl; } else return 0;
    case 0x082C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[1].PrivRgn.WatchdogIRQStat; } else return 0;
    case 0x0830: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[1].PrivRgn.WatchdogResetStat; } else return 0;

    case 0x0908: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[2].PrivRgn.TimerControl; } else return 0;
    case 0x090C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[2].PrivRgn.TimerIRQStat; } else return 0;

    case 0x0928: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[2].PrivRgn.WatchdogControl; } else return 0;
    case 0x092C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[2].PrivRgn.WatchdogIRQStat; } else return 0;
    case 0x0930: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[2].PrivRgn.WatchdogResetStat; } else return 0;

    case 0x0A08: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[3].PrivRgn.TimerControl; } else return 0;
    case 0x0A0C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return c3ds->ARM11[3].PrivRgn.TimerIRQStat; } else return 0;

    case 0x0A28: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[3].PrivRgn.WatchdogControl; } else return 0;
    case 0x0A2C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[3].PrivRgn.WatchdogIRQStat; } else return 0;
    case 0x0A30: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { return ARM11[3].PrivRgn.WatchdogResetStat; } else return 0;


    case 0x1100:
        case 0x1180: return 0xFFFF | c3ds->MPCore.IRQEnable[0] << 16 | c3ds->MPCore.IRQEnable[1] << 24;
        case 0x1104 ... 0x111C:
        case 0x1184 ... 0x119C:
            return c3ds->MPCore.IRQEnable[(addr & 0x1C)-2] | c3ds->MPCore.IRQEnable[(addr & 0x1C)-1] << 8 |
                c3ds->MPCore.IRQEnable[(addr & 0x1C)] << 16 | c3ds->MPCore.IRQEnable[(addr & 0x1C)+1] << 24;
    case 0x1400 ... 0x140C:
        return ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 0] | ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 1] << 8 |
            ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 2] << 16 | ARM11->PrivRgn.IRQPriority[(addr & 0xC) + 3] << 24;
    case 0x1410 ... 0x1418: return 0;
    case 0x141C:
        return ARM11->PrivRgn.IRQPriority[16] | ARM11->PrivRgn.IRQPriority[17] << 8 |
            ARM11->PrivRgn.IRQPriority[18] << 16 | ARM11->PrivRgn.IRQPriority[19] << 24;
    case 0x1420 ... 0x14FC:
        return c3ds->MPCore.IRQPriority[(addr & 0xFC) - 0x20] | c3ds->MPCore.IRQPriority[(addr & 0xFC) - 0x1F] << 8 |
            c3ds->MPCore.IRQPriority[(addr & 0xFC) - 0x1E] << 16 | c3ds->MPCore.IRQPriority[(addr & 0xFC) - 0x1D] << 24;
    case 0x1800 ... 0x1818: return 0;
    case 0x181C: return (1 << (ARM11->CPUID + 8)) | (1 << (ARM11->CPUID + 16)) | (1 << (ARM11->CPUID + 24));
    case 0x1820 ... 0x18FC:
        return c3ds->MPCore.IRQTarget[(addr & 0xFC) - 0x20] | c3ds->MPCore.IRQTarget[(addr & 0xFC) - 0x1F] << 8 |
        c3ds->MPCore.IRQTarget[(addr & 0xFC) - 0x1E] << 16 | c3ds->MPCore.IRQTarget[(addr & 0xFC) - 0x1D] << 24;


    default: 
        printf("ARM11 - UNK MPCORE PRIV RGN LOAD32: %08X %08X\n", addr, ARM11->PC);
        return 0;
    }
}

u8 Bus11_Load8_Main(struct ARM11MPCore* ARM11, const u32 addr)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        return Bios11[addr & (Bios11_Size-1)];

    if ((addr >= 0x10100000) && addr < 0x17E00000) // checkme?
        return Bus11_Load8_IO(ARM11, addr);

    if ((addr & 0xFFFFE000) == 0x17E00000)
        return Bus11_Load8_MPCorePriv(ARM11, addr);

    if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (c3ds->GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000)) // VRAM A
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMA_Hi_Disable : c3ds->VRAMPower.Sub.VRAMA_Lo_Disable))
                    return c3ds->VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }

            if ((addr >= 0x18300000) && (addr < 0x18600000)) // VRAM B
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMB_Hi_Disable : c3ds->VRAMPower.Sub.VRAMB_Lo_Disable))
                    return c3ds->VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }
        }
    }

    if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(c3ds, addr);
        if (bank) return bank[addr&(SWRAM_Size-1)];
        else { printf("ARM11 - ACCESSING UNALLOCATED SWRAM "); }
    }

    if ((addr & 0xFFF80000) == 0x1FF80000)
        return c3ds->AXIWRAM[addr & (AXIWRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return c3ds->FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return c3ds->FCRAM[1][addr & (FCRAM_Size-1)];

    printf("ARM11 - UNK LOAD8: %08X %08X\n", addr, ARM11->PC);
    return 0;
}

u16 Bus11_Load16_Main(struct ARM11MPCore* ARM11, const u32 addr)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        return *(u16*)&Bios11[addr & (Bios11_Size-1)];

    if ((addr >= 0x10100000) && addr < 0x17E00000) // checkme?
        return Bus11_Load16_IO(ARM11, addr);

    if ((addr & 0xFFFFE000) == 0x17E00000)
        return Bus11_Load16_MPCorePriv(ARM11, addr);

    if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (c3ds->GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000)) // VRAM A
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMA_Hi_Disable : c3ds->VRAMPower.Sub.VRAMA_Lo_Disable))
                    return *(u16*)&c3ds->VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }

            if ((addr >= 0x18300000) && (addr < 0x18600000)) // VRAM B
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMB_Hi_Disable : c3ds->VRAMPower.Sub.VRAMB_Lo_Disable))
                    return *(u16*)&c3ds->VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }
        }
    }

    if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(c3ds, addr);
        if (bank) return *(u16*)&bank[addr&(SWRAM_Size-1)];
        else { printf("ARM11 - ACCESSING UNALLOCATED SWRAM "); }
    }

    if ((addr & 0xFFF80000) == 0x1FF80000)
        return *(u16*)&c3ds->AXIWRAM[addr & (AXIWRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return *(u16*)&c3ds->FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return *(u16*)&c3ds->FCRAM[1][addr & (FCRAM_Size-1)];

    printf("ARM11 - UNK LOAD16: %08X %08X\n", addr, ARM11->PC);
    return 0;
}

u32 Bus11_Load32_Main(struct ARM11MPCore* ARM11, const u32 addr)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        return *(u32*)&Bios11[addr & (Bios11_Size-1)];

    if ((addr >= 0x10100000) && addr < 0x17E00000) // checkme?
        return Bus11_Load32_IO(ARM11, addr);

    if ((addr & 0xFFFFE000) == 0x17E00000)
        return Bus11_Load32_MPCorePriv(ARM11, addr);

    if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (c3ds->GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000)) // VRAM A
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMA_Hi_Disable : c3ds->VRAMPower.Sub.VRAMA_Lo_Disable))
                    return *(u32*)&c3ds->VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }

            if ((addr >= 0x18300000) && (addr < 0x18600000)) // VRAM B
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMB_Hi_Disable : c3ds->VRAMPower.Sub.VRAMB_Lo_Disable))
                    return *(u32*)&c3ds->VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }
        }
    }

    if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(c3ds, addr);
        if (bank) return *(u32*)&bank[addr&(SWRAM_Size-1)];
        else { printf("ARM11 - ACCESSING UNALLOCATED SWRAM "); }
    }

    if ((addr & 0xFFF80000) == 0x1FF80000)
        return *(u32*)&c3ds->AXIWRAM[addr & (AXIWRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return *(u32*)&c3ds->FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return *(u32*)&c3ds->FCRAM[1][addr & (FCRAM_Size-1)];

    printf("ARM11 - UNK LOAD32: %08X %08X\n", addr, ARM11->PC);
    return 0;
}

void Bus11_Store8_IO(struct ARM11MPCore* ARM11, const u32 addr, const u8 val)
{
    switch(addr)
    {
        case 0x10140000 ... 0x1014000F: MapSWRAM(c3ds, addr & 0x8, addr & 0x7, val); break;

        case 0x10163000: break;
        case 0x10163001: PXISync_WriteSend(c3ds, val, ARM11ID); break;
        case 0x10163002: break;
        case 0x10163003: PXI11Sync_WriteIRQ(c3ds, val); break;

        default: printf("ARM11 - UNK IO STORE8 %08X %02X %08X\n", addr, val, ARM11->PC); break;
    }
}

void Bus11_Store16_IO(struct ARM11MPCore* ARM11, const u32 addr, const u16 val)
{
    switch(addr)
    {
        case 0x10140000 ... 0x1014000F: MapSWRAM(c3ds, addr & 0x8, addr & 0x7, val); break;

        case 0x10163000: PXISync_WriteSend(c3ds, val >> 8, ARM11ID); break;
        case 0x10163002: PXI11Sync_WriteIRQ(c3ds, val >> 8); break;

        default: printf("ARM11 - UNK IO STORE16 %08X %04X %08X\n", addr, val, ARM11->PC); break;
    }
}

void Bus11_Store32_IO(struct ARM11MPCore* ARM11, const u32 addr, const u32 val)
{
    switch (addr)
    {
        case 0x10141200: c3ds->GPUCnt.Data = val & 0x10075; break;

        case 0x10163000:
            PXISync_WriteSend(c3ds, (val >> 8 & 0xFF), ARM11ID);
            PXI11Sync_WriteIRQ(c3ds, val >> 24);
            break;

        case 0x10400030: c3ds->VRAMPower.Data = val | 0xFFFFF0FF; break;

        default: printf("ARM11 - UNK IO STORE32 %08X %08X %08X\n", addr, val, ARM11->PC); break;
    }
}

void Bus11_Store8_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr, const u8 val)
{
    switch(addr & 0x1FFF)
    {
    case 0x0000: c3ds->MPCore.SCUControlReg &= ~0xFF; c3ds->MPCore.SCUControlReg |= val; break;
        case 0x0001: c3ds->MPCore.SCUControlReg &= ~0xFF00; c3ds->MPCore.SCUControlReg |= (val & 0x3F) << 8; break;
        case 0x0002:
        case 0x0003: break;


    case 0x1000: c3ds->MPCore.IRQDistControl = val & 0x1; break;
        case 0x1001:
        case 0x1002:
        case 0x1003: break;

    // irq enable set
    case 0x1102 ... 0x111F:
        c3ds->MPCore.IRQEnable[(addr & 0x1F)-2] |= val;
        printf("ARM11 - IRQ MASK: ");
        for (int i = 29; i > 0; i--) printf("%02X", c3ds->MPCore.IRQEnable[i]);
        printf("FFFF\n");
        break;

    // irq enable clear    
    case 0x1182 ... 0x119F:
        c3ds->MPCore.IRQEnable[(addr & 0x1F)-2] &= ~val;
        printf("ARM11 - IRQ MASK: ");
        for (int i = 29; i > 0; i--) printf("%02X", c3ds->MPCore.IRQEnable[i]);
        printf("FFFF\n");
        break;

    case 0x1400 ... 0x140F: ARM11->PrivRgn.IRQPriority[addr & 0xF] = val & 0xF0; break;
    case 0x1410 ... 0x141B: break;
    case 0x141C: ARM11->PrivRgn.IRQPriority[16] = val & 0xF0; break;
    case 0x141D: ARM11->PrivRgn.IRQPriority[17] = val & 0xF0; break;
    case 0x141E: ARM11->PrivRgn.IRQPriority[18] = val & 0xF0; break;
    case 0x141F: ARM11->PrivRgn.IRQPriority[19] = val & 0xF0; break;
    case 0x1420 ... 0x14FF: c3ds->MPCore.IRQPriority[(addr & 0xFF) - 0x20] = val & 0xF0; break;

    case 0x1800 ... 0x181F: break;
    case 0x1820 ... 0x18FF:
        c3ds->MPCore.IRQTarget[(addr & 0xFF) - 0x20] = val & 0x0F;
        break;

    case 0x1C00 ... 0x1C03: ARM11->PrivRgn.IRQConfig[addr & 0x3] = val | 0xAA; break;
    case 0x1C04 ... 0x1C07: break;
    case 0x1C08 ... 0x1C3F: ARM11->PrivRgn.IRQConfig[(addr & 0x3C) - 4] = val; break;

    default: printf("ARM11 - UNK MPCORE PRIV RGN STORE8: %08X %02X %08X\n", addr, val, ARM11->PC); break;
    }
}

void Bus11_Store16_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr, u16 val)
{
    switch(addr & 0x1FFE)
    {
    case 0x0000: c3ds->MPCore.SCUControlReg &= ~0xFFFF; c3ds->MPCore.SCUControlReg |= val & 0x3FFF; break;
        case 0x0002: break;


    case 0x1000: c3ds->MPCore.IRQDistControl = val & 0x1; break;
        case 0x1002: break;

    // irq enable set
    case 0x1102 ... 0x111E:
        c3ds->MPCore.IRQEnable[(addr & 0x1E)-2] |= val;
        c3ds->MPCore.IRQEnable[(addr & 0x1E)-1] |= val >> 8;
        printf("ARM11 - IRQ MASK: ");
        for (int i = 29; i > 0; i--) printf("%02X", c3ds->MPCore.IRQEnable[i]);
        printf("FFFF\n");
        break;

    // irq enable clear    
    case 0x1182 ... 0x119E:
        c3ds->MPCore.IRQEnable[(addr & 0x1E)-2] &= ~val;
        c3ds->MPCore.IRQEnable[(addr & 0x1E)-1] &= ~val >> 8;
        printf("ARM11 - IRQ MASK: ");
        for (int i = 29; i > 0; i--) printf("%02X", c3ds->MPCore.IRQEnable[i]);
        printf("FFFF\n");
        break;

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
        c3ds->MPCore.IRQPriority[(addr & 0xFE) - 0x20] = val;
        c3ds->MPCore.IRQPriority[(addr & 0xFE) - 0x1F] = val >> 8;
        break;

    case 0x1800 ... 0x181F: break;
    case 0x1820 ... 0x18FE:
        val &= 0x0F0F;
        c3ds->MPCore.IRQTarget[(addr & 0xFE) - 0x20] = val;
        c3ds->MPCore.IRQTarget[(addr & 0xFE) - 0x1F] = val >> 8;
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

    default: printf("ARM11 - UNK MPCORE PRIV RGN STORE16: %08X %04X %08X\n", addr, val, ARM11->PC); break;
    }
}

void Bus11_Store32_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr, u32 val)
{
    switch(addr & 0x1FFC)
    {
    case 0x0000: c3ds->MPCore.SCUControlReg = val & 0x3FFF; break;


    case 0x0100: ARM11->PrivRgn.IRQControl = val & 0x1; break;

    case 0x0200: if ((c3ds->MPCore.SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[0].PrivRgn.IRQControl = val & 0x1; } break;

    case 0x0300: if ((c3ds->MPCore.SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[1].PrivRgn.IRQControl = val & 0x1; } break;

    case 0x0400: if ((c3ds->MPCore.SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[2].PrivRgn.IRQControl = val & 0x1; } break;

    case 0x0500: if ((c3ds->MPCore.SCUControlReg >> (5 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[3].PrivRgn.IRQControl = val & 0x1; } break;


    case 0x0608: ARM11->PrivRgn.TimerControl = val & 0xFF07; break;
    case 0x060C: ARM11->PrivRgn.TimerIRQStat &= ~val; break;
    case 0x0628: ARM11->PrivRgn.WatchdogControl = (val & 0xFF0F) | (ARM11->PrivRgn.WatchdogControl & 0x8); break;
    case 0x062C: ARM11->PrivRgn.WatchdogIRQStat &= ~val; break;
    case 0x0630: ARM11->PrivRgn.WatchdogResetStat &= ~val; break;
    case 0x0634:
        if ((ARM11->PrivRgn.WatchdogDisable == 0x12345678) && (val == 0x87654321)) { ARM11->PrivRgn.WatchdogControl &= ~0x8; }
        ARM11->PrivRgn.WatchdogDisable = val;
        break;

    case 0x0708: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[0].PrivRgn.TimerControl = val & 0xFF07; } break;
    case 0x070C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[0].PrivRgn.TimerIRQStat &= ~val; } break;
    case 0x0728: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[0].PrivRgn.WatchdogControl = (val & 0xFF0F) | (c3ds->ARM11[0].PrivRgn.WatchdogControl & 0x8); } break;
    case 0x072C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[0].PrivRgn.WatchdogIRQStat &= ~val;}  break;
    case 0x0730: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[0].PrivRgn.WatchdogResetStat &= ~val; } break;
    case 0x0734: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { 
            if ((c3ds->ARM11[0].PrivRgn.WatchdogDisable == 0x12345678) && (val == 0x87654321)) { c3ds->ARM11[0].PrivRgn.WatchdogControl &= ~0x8; }
            c3ds->ARM11[0].PrivRgn.WatchdogDisable = val;
        }
        break;

    case 0x0808: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[1].PrivRgn.TimerControl = val & 0xFF07; } break;
    case 0x080C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[1].PrivRgn.TimerIRQStat &= ~val; } break;
    case 0x0828: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[1].PrivRgn.WatchdogControl = (val & 0xFF0F) | (c3ds->ARM11[0].PrivRgn.WatchdogControl & 0x8); } break;
    case 0x082C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[1].PrivRgn.WatchdogIRQStat &= ~val;}  break;
    case 0x0830: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[1].PrivRgn.WatchdogResetStat &= ~val; } break;
    case 0x0834: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { 
            if ((c3ds->ARM11[1].PrivRgn.WatchdogDisable == 0x12345678) && (val == 0x87654321)) { c3ds->ARM11[1].PrivRgn.WatchdogControl &= ~0x8; }
            c3ds->ARM11[1].PrivRgn.WatchdogDisable = val;
        }
        break;

    case 0x0908: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[2].PrivRgn.TimerControl = val & 0xFF07; } break;
    case 0x090C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[2].PrivRgn.TimerIRQStat &= ~val; } break;
    case 0x0928: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[2].PrivRgn.WatchdogControl = (val & 0xFF0F) | (c3ds->ARM11[2].PrivRgn.WatchdogControl & 0x8); } break;
    case 0x092C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[2].PrivRgn.WatchdogIRQStat &= ~val;}  break;
    case 0x0930: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[2].PrivRgn.WatchdogResetStat &= ~val; } break;
    case 0x0934: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { 
            if ((c3ds->ARM11[2].PrivRgn.WatchdogDisable == 0x12345678) && (val == 0x87654321)) { c3ds->ARM11[2].PrivRgn.WatchdogControl &= ~0x8; }
            c3ds->ARM11[2].PrivRgn.WatchdogDisable = val;
        }
        break;

    case 0x0A08: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[3].PrivRgn.TimerControl = val & 0xFF07; } break;
    case 0x0A0C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[3].PrivRgn.TimerIRQStat &= ~val; } break;
    case 0x0A28: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[3].PrivRgn.WatchdogControl = (val & 0xFF0F) | (c3ds->ARM11[3].PrivRgn.WatchdogControl & 0x8); } break;
    case 0x0A2C: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[3].PrivRgn.WatchdogIRQStat &= ~val;}  break;
    case 0x0A30: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { c3ds->ARM11[3].PrivRgn.WatchdogResetStat &= ~val; } break;
    case 0x0A34: if ((c3ds->MPCore.SCUControlReg >> (9 + ARM11->CPUID)) & 0x1) { 
            if ((c3ds->ARM11[3].PrivRgn.WatchdogDisable == 0x12345678) && (val == 0x87654321)) { c3ds->ARM11[3].PrivRgn.WatchdogControl &= ~0x8; }
            c3ds->ARM11[3].PrivRgn.WatchdogDisable = val;
        }
        break;
    
    case 0x1000: c3ds->MPCore.IRQDistControl = val & 0x1; break;

    // irq enable set
    case 0x1100:
        c3ds->MPCore.IRQEnable[0] |= val >> 16;
        c3ds->MPCore.IRQEnable[1] |= val >> 24;
        printf("ARM11 - IRQ MASK: ");
        for (int i = 29; i > 0; i--) printf("%02X", c3ds->MPCore.IRQEnable[i]);
        printf("FFFF\n");
        break;
    case 0x1104 ... 0x111C:
        c3ds->MPCore.IRQEnable[(addr & 0x1C)-2] |= val;
        c3ds->MPCore.IRQEnable[(addr & 0x1C)-1] |= val >> 8;
        c3ds->MPCore.IRQEnable[(addr & 0x1C)+0] |= val >> 16;
        c3ds->MPCore.IRQEnable[(addr & 0x1C)+1] |= val >> 24;
        printf("ARM11 - IRQ MASK: ");
        for (int i = 29; i > 0; i--) printf("%02X", c3ds->MPCore.IRQEnable[i]);
        printf("FFFF\n");
        break;
    // irq enable clear    
    case 0x1180:
        c3ds->MPCore.IRQEnable[0] &= ~val >> 16;
        c3ds->MPCore.IRQEnable[1] &= ~val >> 24;
        printf("ARM11 - IRQ MASK: ");
        for (int i = 29; i > 0; i--) printf("%02X", c3ds->MPCore.IRQEnable[i]);
        printf("FFFF\n");
        break;
    case 0x1184 ... 0x119C:
        c3ds->MPCore.IRQEnable[(addr & 0x1C)-2] &= ~val;
        c3ds->MPCore.IRQEnable[(addr & 0x1C)-1] &= ~val >> 8;
        c3ds->MPCore.IRQEnable[(addr & 0x1C)+0] &= ~val >> 16;
        c3ds->MPCore.IRQEnable[(addr & 0x1C)+1] &= ~val >> 24;
        printf("ARM11 - IRQ MASK: ");
        for (int i = 29; i > 0; i--) printf("%02X", c3ds->MPCore.IRQEnable[i]);
        printf("FFFF\n");
        break;

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
        c3ds->MPCore.IRQPriority[(addr & 0xFC) - 0x20] = val;
        c3ds->MPCore.IRQPriority[(addr & 0xFC) - 0x1F] = val >> 8;
        c3ds->MPCore.IRQPriority[(addr & 0xFC) - 0x1E] = val >> 16;
        c3ds->MPCore.IRQPriority[(addr & 0xFC) - 0x1D] = val >> 24;
        break;

    case 0x1800 ... 0x181F: break;
    case 0x1820 ... 0x18FC:
        val &= 0x0F0F0F0F;
        c3ds->MPCore.IRQTarget[(addr & 0xFC) - 0x20] = val;
        c3ds->MPCore.IRQTarget[(addr & 0xFC) - 0x1F] = val >> 8;
        c3ds->MPCore.IRQTarget[(addr & 0xFC) - 0x1E] = val >> 16;
        c3ds->MPCore.IRQTarget[(addr & 0xFC) - 0x1D] = val >> 24;
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

    default: printf("ARM11 - UNK MPCORE PRIV RGN STORE32: %08X %08X %08X\n", addr, val, ARM11->PC); break;
    }
}

void Bus11_Store8_Main(struct ARM11MPCore* ARM11, const u32 addr, const u8 val)
{
    if ((addr >= 0x10100000) && addr < 0x17E00000) // checkme?
        Bus11_Store8_IO(ARM11, addr, val);

    else if ((addr & 0xFFFFE000) == 0x17E00000)
        Bus11_Store8_MPCorePriv(ARM11, addr, val);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (c3ds->GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000))
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMA_Hi_Disable : c3ds->VRAMPower.Sub.VRAMA_Lo_Disable))
                    c3ds->VRAM[addr - 0x18000000] = val;
            }

            else if ((addr >= 0x18300000) && (addr < 0x18600000))
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMB_Hi_Disable : c3ds->VRAMPower.Sub.VRAMB_Lo_Disable))
                    c3ds->VRAM[addr - 0x18000000] = val;
            }
        }
    }

    else if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(c3ds, addr);
        if (bank) bank[addr&(SWRAM_Size-1)] = val;
        else { printf("ARM11 - UNALLOCATED SWRAM STORE8!! %08X %02X %08X\n", addr, val, ARM11->PC); }
    }

    else if ((addr & 0xFFF80000) == 0x1FF80000)
        c3ds->AXIWRAM[addr & (AXIWRAM_Size-1)] = val;

    else if ((addr & 0xF8000000) == 0x20000000)
        c3ds->FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        c3ds->FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("ARM11 - UNK STORE8: %08X %02X %08X\n", addr, val, ARM11->PC);
}

void Bus11_Store16_Main(struct ARM11MPCore* ARM11, const u32 addr, const u16 val)
{
    if ((addr >= 0x10100000) && addr < 0x17E00000) // checkme?
        Bus11_Store16_IO(ARM11, addr, val);

    else if ((addr & 0xFFFFE000) == 0x17E00000)
        Bus11_Store16_MPCorePriv(ARM11, addr, val);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (c3ds->GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000))
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMA_Hi_Disable : c3ds->VRAMPower.Sub.VRAMA_Lo_Disable))
                    *(u16*)&c3ds->VRAM[addr - 0x18000000] = val;
            }

            else if ((addr >= 0x18300000) && (addr < 0x18600000))
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMB_Hi_Disable : c3ds->VRAMPower.Sub.VRAMB_Lo_Disable))
                    *(u16*)&c3ds->VRAM[addr - 0x18000000] = val;
            }
        }
    }

    else if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(c3ds, addr);
        if (bank) *(u16*)&bank[addr&(SWRAM_Size-1)] = val;
        else { printf("ARM11 - UNALLOCATED SWRAM STORE16!! %08X %04X %08X\n", addr, val, ARM11->PC); }
    }

    else if ((addr & 0xFFF80000) == 0x1FF80000)
        *(u16*)&c3ds->AXIWRAM[addr & (AXIWRAM_Size-1)] = val;

    else if ((addr & 0xF8000000) == 0x20000000)
        *(u16*)&c3ds->FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        *(u16*)&c3ds->FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("ARM11 - UNK STORE16: %08X %04X %08X\n", addr, val, ARM11->PC);
}

void Bus11_Store32_Main(struct ARM11MPCore* ARM11, const u32 addr, const u32 val)
{
    if ((addr >= 0x10100000) && addr < 0x17E00000) // checkme?
        Bus11_Store32_IO(ARM11, addr, val);

    else if ((addr & 0xFFFFE000) == 0x17E00000)
        Bus11_Store32_MPCorePriv(ARM11, addr, val);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (c3ds->GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000))
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMA_Hi_Disable : c3ds->VRAMPower.Sub.VRAMA_Lo_Disable))
                    *(u32*)&c3ds->VRAM[addr - 0x18000000] = val;
            }

            else if ((addr >= 0x18300000) && (addr < 0x18600000))
            {
                if (!((addr & 0x8) ? c3ds->VRAMPower.Sub.VRAMB_Hi_Disable : c3ds->VRAMPower.Sub.VRAMB_Lo_Disable))
                    *(u32*)&c3ds->VRAM[addr - 0x18000000] = val;
            }
        }
    }

    else if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(c3ds, addr);
        if (bank) *(u32*)&bank[addr&(SWRAM_Size-1)] = val;
        else { printf("ARM11 - UNALLOCATED SWRAM STORE32!! %08X %08X %08X\n", addr, val, ARM11->PC); }
    }

    else if ((addr & 0xFFF80000) == 0x1FF80000)
        *(u32*)&c3ds->AXIWRAM[addr & (AXIWRAM_Size-1)] = val;

    else if ((addr & 0xF8000000) == 0x20000000)
        *(u32*)&c3ds->FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        *(u32*)&c3ds->FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("ARM11 - UNK STORE32: %08X %08X %08X\n", addr, val, ARM11->PC);
}

u32 Bus11_PageTableLoad32(struct ARM11MPCore* ARM11, u32 addr)
{
    if (addr & 0x3) printf("ARM11 - UNALIGNED PAGE TABLE FETCH??? %08X %08X\n", addr, ARM11->PC);
    addr &= ~0x3;

    u32 val = Bus11_Load32_Main(ARM11, addr & ~3);

    return ARM11->CP15.ExceptionEndian ? bswap(val) : val;
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

    if (addr & 0x3) printf("ARM11 - UNALIGNED LOAD32 %08X %08X\n", addr, ARM11->PC);
    addr &= ~0x3;

    return Bus11_Load32_Main(ARM11, addr);
}

u16 Bus11_Load16(struct ARM11MPCore* ARM11, u32 addr)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, TLB_Read);

    if (addr & 0x1) printf("ARM11 - UNALIGNED LOAD16 %08X %08X\n", addr, ARM11->PC);
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

    if (addr & 0x3) printf("ARM11 - UNALIGNED STORE32 %08X %08X\n", addr, ARM11->PC);
    addr &= ~0x3;

    Bus11_Store32_Main(ARM11, addr, val);
}

void Bus11_Store16(struct ARM11MPCore* ARM11, u32 addr, const u16 val)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, 0);

    if (addr & 0x1) printf("ARM11 - UNALIGNED STORE16 %08X %08X\n", addr, ARM11->PC);
    addr &= ~0x1;

    Bus11_Store16_Main(ARM11, addr, val);
}

void Bus11_Store8(struct ARM11MPCore* ARM11, u32 addr, const u8 val)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, 0);

    Bus11_Store8_Main(ARM11, addr, val);
}

#undef c3ds
