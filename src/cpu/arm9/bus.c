#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "arm.h"
#include "../../utils.h"
#include "bus.h"
#include "../shared/bus.h"
#include "../arm11/bus.h"

u8* Bios9;
const u32 Bios9_Size = 64 * 1024;

char* Bus9_Init()
{
    Bios9 = malloc(Bios9_Size);
    FILE* file = fopen("bios_ctr9.bin", "rb");
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

void Bus9_Free()
{
    free(Bios9);
}

u8 Bus9_Load8_Main(struct ARM946E_S* ARM9, const u32 addr)
{
    if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (CFG11.GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000)) // VRAM A
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMA_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMA_Lo_Disable))
                    return VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }

            if ((addr >= 0x18300000) && (addr < 0x18600000)) // VRAM B
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMB_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMB_Lo_Disable))
                    return VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }
        }
    }

    if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(addr);
        if (bank) return bank[addr&(SWRAM_Size-1)];
        else { printf("ACCESSING UNALLOCATED SWRAM "); }
    }

    if ((addr & 0xFFF80000) == 0x1FF80000)
        return AXI_WRAM[addr & (AXI_WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return FCRAM[1][addr & (FCRAM_Size-1)];

    printf("UNK LOAD8: %08X %08X\n", addr, ARM9->PC);
    return 0;
}

u16 Bus9_Load16_Main(struct ARM946E_S* ARM9, const u32 addr)
{
    if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (CFG11.GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000)) // VRAM A
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMA_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMA_Lo_Disable))
                    return *(u16*)&VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }

            if ((addr >= 0x18300000) && (addr < 0x18600000)) // VRAM B
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMB_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMB_Lo_Disable))
                    return *(u16*)&VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }
        }
    }

    if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(addr);
        if (bank) return *(u16*)&bank[addr&(SWRAM_Size-1)];
        else { printf("ACCESSING UNALLOCATED SWRAM "); }
    }

    if ((addr & 0xFFF80000) == 0x1FF80000)
        return *(u16*)&AXI_WRAM[addr & (AXI_WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return *(u16*)&FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return *(u16*)&FCRAM[1][addr & (FCRAM_Size-1)];

    printf("UNK LOAD16: %08X %08X\n", addr, ARM9->PC);
    return 0;
}

u32 Bus9_Load32_Main(struct ARM946E_S* ARM9, const u32 addr)
{
    if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (CFG11.GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000)) // VRAM A
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMA_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMA_Lo_Disable))
                    return *(u32*)&VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }

            if ((addr >= 0x18300000) && (addr < 0x18600000)) // VRAM B
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMB_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMB_Lo_Disable))
                    return *(u32*)&VRAM[addr - 0x18000000];
                else
                    ; // todo: hang cpu 
            }
        }
    }

    if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(addr);
        if (bank) return *(u32*)&bank[addr&(SWRAM_Size-1)];
        else { printf("ACCESSING UNALLOCATED SWRAM "); }
    }

    if ((addr & 0xFFF80000) == 0x1FF80000)
        return *(u32*)&AXI_WRAM[addr & (AXI_WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return *(u32*)&FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return *(u32*)&FCRAM[1][addr & (FCRAM_Size-1)];

    printf("UNK LOAD32: %08X %08X\n", addr, ARM9->PC);
    return 0;
}

void Bus9_Store8_Main(struct ARM946E_S* ARM9, const u32 addr, const u8 val)
{
    if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (CFG11.GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000))
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMA_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMA_Lo_Disable))
                    VRAM[addr - 0x18000000] = val;
            }

            else if ((addr >= 0x18300000) && (addr < 0x18600000))
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMB_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMB_Lo_Disable))
                    VRAM[addr - 0x18000000] = val;
            }
        }
    }
    
    else if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(addr);
        if (bank) bank[addr&(SWRAM_Size-1)] = val;
        else { printf("UNALLOCATED SWRAM STORE8!! %08X %02X %08X\n", addr, val, ARM9->PC); }
    }

    else if ((addr & 0xFFF80000) == 0x1FF80000)
        AXI_WRAM[addr & (AXI_WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("UNK STORE8: %08X %02X %08X\n", addr, val, ARM9->PC);
}

void Bus9_Store16_Main(struct ARM946E_S* ARM9, const u32 addr, const u16 val)
{
    if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (CFG11.GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000))
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMA_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMA_Lo_Disable))
                    *(u16*)&VRAM[addr - 0x18000000] = val;
            }

            else if ((addr >= 0x18300000) && (addr < 0x18600000))
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMB_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMB_Lo_Disable))
                    *(u16*)&VRAM[addr - 0x18000000] = val;
            }
        }
    }

    else if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(addr);
        if (bank) *(u16*)&bank[addr&(SWRAM_Size-1)] = val;
        else { printf("UNALLOCATED SWRAM STORE16!! %08X %04X %08X\n", addr, val, ARM9->PC); }
    }

    else if ((addr & 0xFFF80000) == 0x1FF80000)
        *(u16*)&AXI_WRAM[addr & (AXI_WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        *(u16*)&FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        *(u16*)&FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("UNK STORE16: %08X %04X %08X\n", addr, val, ARM9->PC);
}

void Bus9_Store32_Main(struct ARM946E_S* ARM9, const u32 addr, const u32 val)
{
    if ((addr >= 0x18000000) && (addr < 0x18600000))
    {
        if (CFG11.GPUCnt.Sub.GPURegVRAM_Enable)
        {
            if ((addr >= 0x18000000) && (addr < 0x18300000))
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMA_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMA_Lo_Disable))
                    *(u32*)&VRAM[addr - 0x18000000] = val;
            }

            else if ((addr >= 0x18300000) && (addr < 0x18600000))
            {
                if (!((addr & 0x8) ? GPUIO.VRAMPower.Sub.VRAMB_Hi_Disable : GPUIO.VRAMPower.Sub.VRAMB_Lo_Disable))
                    *(u32*)&VRAM[addr - 0x18000000] = val;
            }
        }
    }
    
    else if ((addr & 0xFFF80000) == 0x1FF00000)
    {
        u8* bank = GetSWRAM(addr);
        if (bank) *(u32*)&bank[addr&(SWRAM_Size-1)] = val;
        else { printf("UNALLOCATED SWRAM STORE32!! %08X %08X %08X\n", addr, val, ARM9->PC); }
    }

    else if ((addr & 0xFFF80000) == 0x1FF80000)
        *(u32*)&AXI_WRAM[addr & (AXI_WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        *(u32*)&FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        *(u32*)&FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("UNK STORE32: %08X %08X %08X\n", addr, val, ARM9->PC);
}

u32 Bus9_InstrLoad32(struct ARM946E_S* ARM9, u32 addr)
{
    ARM9_CP15_PageTable_Lookup(ARM9, &addr, TLB_Instr | TLB_Read);

    addr &= ~0x3;

    return Bus9_Load32_Main(ARM9, addr);
}

u32 Bus9_Load32(struct ARM946E_S* ARM9, u32 addr)
{
    ARM9_CP15_PageTable_Lookup(ARM9, &addr, TLB_Read);

    if (addr & 0x3) printf("UNALIGNED LOAD32 %08X %08X\n", addr, ARM9->PC);
    addr &= ~0x3;

    return Bus9_Load32_Main(ARM9, addr);
}

u16 Bus9_Load16(struct ARM946E_S* ARM9, u32 addr)
{
    ARM9_CP15_PageTable_Lookup(ARM9, &addr, TLB_Read);

    if (addr & 0x1) printf("UNALIGNED LOAD16 %08X %08X\n", addr, ARM9->PC);
    addr &= ~0x1;

    return Bus9_Load16_Main(ARM9, addr);
}

u8 Bus9_Load8(struct ARM946E_S* ARM9, u32 addr)
{
    ARM9_CP15_PageTable_Lookup(ARM9, &addr, TLB_Read);

    return Bus9_Load8_Main(ARM9, addr);
}

void Bus9_Store32(struct ARM946E_S* ARM9, u32 addr, const u32 val)
{
    ARM9_CP15_PageTable_Lookup(ARM9, &addr, 0);

    if (addr & 0x3) printf("UNALIGNED STORE32 %08X %08X\n", addr, ARM9->PC);
    addr &= ~0x3;

    Bus9_Store32_Main(ARM9, addr, val);
}

void Bus9_Store16(struct ARM946E_S* ARM9, u32 addr, const u16 val)
{
    ARM9_CP15_PageTable_Lookup(ARM9, &addr, 0);

    if (addr & 0x1) printf("UNALIGNED STORE16 %08X %08X\n", addr, ARM9->PC);
    addr &= ~0x1;

    Bus9_Store16_Main(ARM9, addr, val);
}

void Bus9_Store8(struct ARM946E_S* ARM9, u32 addr, const u8 val)
{
    ARM9_CP15_PageTable_Lookup(ARM9, &addr, 0);

    Bus9_Store8_Main(ARM9, addr, val);
}
