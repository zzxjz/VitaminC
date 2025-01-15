#include <stdlib.h>
#include <stdio.h>
#include "interpreter.h"
#include "../../utils.h"
#include "bus.h"

u8* Bios11;
const u32 Bios11_Size = 64 * 1024;

// io

// mpcore priv rgn io
u32 SCUControlReg;




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

    SCUControlReg = 0x1FFE;
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

u8 Bus11_Load8_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch(addr & 0x1FFC)
    {
    case 0x0000: return SCUControlReg;
        case 0x0001: return SCUControlReg>>8;
        case 0x0002: return SCUControlReg>>16;
        case 0x0003: return SCUControlReg>>24;
    }

    printf("UNK MPCORE PRIV RGN LOAD8: %08X %08X\n", addr, ARM11->PC);
    return 0;
}

u16 Bus11_Load16_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch(addr & 0x1FFC)
    {
    case 0x0000: return SCUControlReg;
        case 0x0002: return SCUControlReg>>16;
    }

    printf("UNK MPCORE PRIV RGN LOAD16: %08X %08X\n", addr, ARM11->PC);
    return 0;
}

u32 Bus11_Load32_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr)
{
    switch(addr & 0x1FFC)
    {
    case 0x0000: return SCUControlReg;

    case 0x0608: return ARM11->TimerControl;
    case 0x060C: return ARM11->TimerIRQStat;
    }

    printf("UNK MPCORE PRIV RGN LOAD32: %08X %08X\n", addr, ARM11->PC);
    return 0;
}

u8 Bus11_Load8_Main(struct ARM11MPCore* ARM11, const u32 addr)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        return *(u8*)&Bios11[addr & (Bios11_Size-1)];

    if ((addr & 0xFFFFE000) == 0x17E00000)
        return Bus11_Load8_MPCorePriv(ARM11, addr);

    if ((addr & 0xFFF00000) == 0x1FF00000)
        return *(u8*)&WRAM[addr & (WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return *(u8*)&FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return *(u8*)&FCRAM[1][addr & (FCRAM_Size-1)];

    printf("UNK LOAD8: %08X %08X\n", addr, ARM11->PC);
    return 0;
}

u16 Bus11_Load16_Main(struct ARM11MPCore* ARM11, const u32 addr)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        return *(u16*)&Bios11[addr & (Bios11_Size-1)];

    if ((addr & 0xFFFFE000) == 0x17E00000)
        return Bus11_Load16_MPCorePriv(ARM11, addr);

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

    if ((addr & 0xFFF00000) == 0x1FF00000)
        return *(u32*)&WRAM[addr & (WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return *(u32*)&FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return *(u32*)&FCRAM[1][addr & (FCRAM_Size-1)];

    printf("UNK LOAD32: %08X %08X\n", addr, ARM11->PC);
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
    default: printf("UNK MPCORE PRIV RGN STORE8: %08X %08X\n", addr, ARM11->PC); break;
    }
}

void Bus11_Store16_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr, const u16 val)
{
    switch(addr & 0x1FFE)
    {
    case 0x0000: SCUControlReg &= ~0xFFFF; SCUControlReg |= val & 0x3FFF; break;
        case 0x0002: break;
    default: printf("UNK MPCORE PRIV RGN STORE16: %08X %08X\n", addr, ARM11->PC); break;
    }
}

void Bus11_Store32_MPCorePriv(struct ARM11MPCore* ARM11, const u32 addr, const u32 val)
{
    switch(addr & 0x1FFC)
    {
    case 0x0000: SCUControlReg = val & 0x3FFF; break;

    case 0x0608: ARM11->TimerControl = val & 0xFF07; break;
    case 0x060C: ARM11->TimerIRQStat &= ~val; break;
    
    default: printf("UNK MPCORE PRIV RGN STORE32: %08X %08X\n", addr, ARM11->PC); break;
    }
}

void Bus11_Store8_Main(struct ARM11MPCore* ARM11, const u32 addr, const u8 val)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        Bios11[addr & (Bios11_Size-1)] = val;

    else if ((addr & 0xFFFFE000) == 0x17E00000)
        Bus11_Store8_MPCorePriv(ARM11, addr, val);

    else if ((addr & 0xFFF00000) == 0x1FF00000)
        WRAM[addr & (WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("UNK STORE8: %08X %08X\n", addr, ARM11->PC);
}

void Bus11_Store16_Main(struct ARM11MPCore* ARM11, const u32 addr, const u16 val)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        *(u16*)&Bios11[addr & (Bios11_Size-1)] = val;

    else if ((addr & 0xFFFFE000) == 0x17E00000)
        Bus11_Store16_MPCorePriv(ARM11, addr, val);

    else if ((addr & 0xFFF00000) == 0x1FF00000)
        *(u16*)&WRAM[addr & (WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        *(u16*)&FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        *(u16*)&FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("UNK STORE16: %08X %08X\n", addr, ARM11->PC);
}

void Bus11_Store32_Main(struct ARM11MPCore* ARM11, const u32 addr, const u32 val)
{
    if ((addr < 0x20000) || (addr >= 0xFFFF0000))
        *(u32*)&Bios11[addr & (Bios11_Size-1)] = val;

    else if ((addr & 0xFFFFE000) == 0x17E00000)
        Bus11_Store32_MPCorePriv(ARM11, addr, val);

    else if ((addr & 0xFFF00000) == 0x1FF00000)
        *(u32*)&WRAM[addr & (WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        *(u32*)&FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        *(u32*)&FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("UNK STORE32: %08X %08X\n", addr, ARM11->PC);
}

u32 Bus11_PageTableLoad32(struct ARM11MPCore* ARM11, u32 addr)
{
    u32 val = Bus11_Load32_Main(ARM11, addr & ~3);

    return ARM11->CP15.ExceptionEndian ? __builtin_bswap32(val) : val;
}

u32 Bus11_InstrLoad32(struct ARM11MPCore* ARM11, u32 addr)
{
    ARM11_CP15_PageTable_Lookup(ARM11, &addr, TLB_Instr | TLB_Read);

    return Bus11_Load32_Main(ARM11, addr & ~3);
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
