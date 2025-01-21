#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "arm.h"
#include "../shared/arm.h"
#include "../../utils.h"
#include "bus.h"
#include "../shared/bus.h"
#include "../arm11/bus.h"
#include "../../pxi.h"

u8* Bios9;
const u32 Bios9_Size = 64 * 1024;
u8* ARM9WRAM;
const u32 ARM9WRAM_Size = 1 * 1024 * 1024;
u8* ITCM;
const u32 ITCM_PhySize = 32 * 1024;
u8* DTCM;
const u32 DTCM_PhySize = 16 * 1024;

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
    ARM9WRAM = calloc(1, ARM9WRAM_Size);
    ITCM = calloc(1, ITCM_PhySize);
    DTCM = calloc(1, DTCM_PhySize);
    return NULL;
}

void Bus9_Free()
{
    free(Bios9);
    free(ARM9WRAM);
    free(ITCM);
    free(DTCM);
}

u8 Bus9_Load8_IO(struct ARM946E_S* ARM9, const u32 addr)
{
    switch(addr)
    {
    case 0x10008000: return PXI_Sync[ARM9ID] & 0xFF;
    case 0x10008001:
    case 0x10008002: return 0;
    case 0x10008003: return PXI_Sync[ARM9ID] >> 24;

    default: printf("ARM9 - UNK IO LOAD8: %08X %08X\n", addr, ARM9->PC); return 0;
    }
}

u16 Bus9_Load16_IO(struct ARM946E_S* ARM9, const u32 addr)
{
    switch(addr)
    {
    case 0x10008000: return PXI_Sync[ARM9ID] & 0xFFFF;
    case 0x10008002: return PXI_Sync[ARM9ID] >> 16;
        
    default: printf("ARM9 - UNK IO LOAD16: %08X %08X\n", addr, ARM9->PC); return 0;
    }
}

u32 Bus9_Load32_IO(struct ARM946E_S* ARM9, const u32 addr)
{
    switch(addr)
    {
    case 0x10008000: return PXI_Sync[ARM9ID];
        
    default: printf("ARM9 - UNK IO LOAD32: %08X %08X\n", addr, ARM9->PC); return 0;
    }
}

u8 Bus9_Load8_Main(struct ARM946E_S* ARM9, const u32 addr)
{
    if (addr >= 0xFFFF0000)
        return Bios9[addr & (Bios9_Size-1)];

    if ((addr & 0xF8000000) == 0x08000000)
        return ARM9WRAM[addr & (ARM9WRAM_Size-1)];

    if ((addr >= 0x10000000) && addr < 0x10200000) // checkme?
        return Bus9_Load8_IO(ARM9, addr);

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
        else { printf("ARM9 - ACCESSING UNALLOCATED SWRAM "); }
    }

    if ((addr & 0xFFF80000) == 0x1FF80000)
        return AXI_WRAM[addr & (AXI_WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return FCRAM[1][addr & (FCRAM_Size-1)];

    printf("ARM9 - UNK LOAD8: %08X %08X\n", addr, ARM9->PC);
    return 0;
}

u16 Bus9_Load16_Main(struct ARM946E_S* ARM9, const u32 addr)
{
    if (addr >= 0xFFFF0000)
        return *(u16*)&Bios9[addr & (Bios9_Size-1)];

    if ((addr & 0xF8000000) == 0x08000000)
        return *(u16*)&ARM9WRAM[addr & (ARM9WRAM_Size-1)];

    if ((addr >= 0x10000000) && addr < 0x10200000) // checkme?
        return Bus9_Load16_IO(ARM9, addr);

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
        else { printf("ARM9 - ACCESSING UNALLOCATED SWRAM "); }
    }

    if ((addr & 0xFFF80000) == 0x1FF80000)
        return *(u16*)&AXI_WRAM[addr & (AXI_WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return *(u16*)&FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return *(u16*)&FCRAM[1][addr & (FCRAM_Size-1)];

    printf("ARM9 - UNK LOAD16: %08X %08X\n", addr, ARM9->PC);
    return 0;
}

u32 Bus9_Load32_Main(struct ARM946E_S* ARM9, const u32 addr)
{
    if (addr >= 0xFFFF0000)
        return *(u32*)&Bios9[addr & (Bios9_Size-1)];

    if ((addr & 0xF8000000) == 0x08000000)
        return *(u32*)&ARM9WRAM[addr & (ARM9WRAM_Size-1)];

    if ((addr >= 0x10000000) && addr < 0x10200000) // checkme?
        return Bus9_Load32_IO(ARM9, addr);

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
        else { printf("ARM9 - ACCESSING UNALLOCATED SWRAM "); }
    }

    if ((addr & 0xFFF80000) == 0x1FF80000)
        return *(u32*)&AXI_WRAM[addr & (AXI_WRAM_Size-1)];
    
    if ((addr & 0xF8000000) == 0x20000000)
        return *(u32*)&FCRAM[0][addr & (FCRAM_Size-1)];
    if ((addr & 0xF8000000) == 0x28000000)
        return *(u32*)&FCRAM[1][addr & (FCRAM_Size-1)];

    printf("ARM9 - UNK LOAD32: %08X %08X\n", addr, ARM9->PC);
    while(addr == 0x10008000);
    return 0;
}

void Bus9_Store8_IO(struct ARM946E_S* ARM9, const u32 addr, const u8 val)
{
    switch(addr)
    {
    case 0x10008000: break;
    case 0x10008001: PXISync_WriteSend(val, ARM9ID); break;
    case 0x10008002: break;
    case 0x10008003: PXI9Sync_WriteIRQ(val); break;

    default: printf("ARM9 - UNK IO STORE8: %08X %08X\n", addr, ARM9->PC); break;
    }
}

void Bus9_Store16_IO(struct ARM946E_S* ARM9, const u32 addr, const u8 val)
{
    switch(addr)
    {
    case 0x10008000: PXISync_WriteSend(val >> 8, ARM9ID); break;
    case 0x10008002: PXI9Sync_WriteIRQ(val >> 8); break;
        
    default: printf("ARM9 - UNK IO STORE16: %08X %08X\n", addr, ARM9->PC); break;
    }
}

void Bus9_Store32_IO(struct ARM946E_S* ARM9, const u32 addr, const u8 val)
{
    switch(addr)
    {
    case 0x10008000:
        PXISync_WriteSend((val >> 8 & 0xFF), ARM9ID);
        PXI9Sync_WriteIRQ(val >> 24);
        break;
        
    default: printf("ARM9 - UNK IO STORE32: %08X %08X\n", addr, ARM9->PC); break;
    }
}

void Bus9_Store8_Main(struct ARM946E_S* ARM9, const u32 addr, const u8 val)
{
    if ((addr & 0xF8000000) == 0x08000000)
        ARM9WRAM[addr & (ARM9WRAM_Size-1)] = val;

    else if ((addr >= 0x10000000) && addr < 0x10200000) // checkme?
        Bus9_Store8_IO(ARM9, addr, val);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
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
        else { printf("ARM9 - UNALLOCATED SWRAM STORE8!! %08X %02X %08X\n", addr, val, ARM9->PC); }
    }

    else if ((addr & 0xFFF80000) == 0x1FF80000)
        AXI_WRAM[addr & (AXI_WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("ARM9 - UNK STORE8: %08X %02X %08X\n", addr, val, ARM9->PC);
}

void Bus9_Store16_Main(struct ARM946E_S* ARM9, const u32 addr, const u16 val)
{
    if ((addr & 0xF8000000) == 0x08000000)
        *(u16*)&ARM9WRAM[addr & (ARM9WRAM_Size-1)] = val;

    else if ((addr >= 0x10000000) && addr < 0x10200000) // checkme?
        Bus9_Store16_IO(ARM9, addr, val);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
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
        else { printf("ARM9 - UNALLOCATED SWRAM STORE16!! %08X %04X %08X\n", addr, val, ARM9->PC); }
    }

    else if ((addr & 0xFFF80000) == 0x1FF80000)
        *(u16*)&AXI_WRAM[addr & (AXI_WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        *(u16*)&FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        *(u16*)&FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("ARM9 - UNK STORE16: %08X %04X %08X\n", addr, val, ARM9->PC);
}

void Bus9_Store32_Main(struct ARM946E_S* ARM9, const u32 addr, const u32 val)
{
    if ((addr & 0xF8000000) == 0x08000000)
        *(u32*)&ARM9WRAM[addr & (ARM9WRAM_Size-1)] = val;

    else if ((addr >= 0x10000000) && addr < 0x10200000) // checkme?
        Bus9_Store32_IO(ARM9, addr, val);

    else if ((addr >= 0x18000000) && (addr < 0x18600000))
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
        else { printf("ARM9 - UNALLOCATED SWRAM STORE32!! %08X %08X %08X\n", addr, val, ARM9->PC); }
    }

    else if ((addr & 0xFFF80000) == 0x1FF80000)
        *(u32*)&AXI_WRAM[addr & (AXI_WRAM_Size-1)] = val;
    
    else if ((addr & 0xF8000000) == 0x20000000)
        *(u32*)&FCRAM[0][addr & (FCRAM_Size-1)] = val;
    else if ((addr & 0xF8000000) == 0x28000000)
        *(u32*)&FCRAM[1][addr & (FCRAM_Size-1)] = val;

    else printf("ARM9 - UNK STORE32: %08X %08X %08X\n", addr, val, ARM9->PC);
}

struct Instruction Bus9_InstrLoad32(struct ARM946E_S* ARM9, u32 addr)
{
    if (!(ARM9_MPU_Lookup(ARM9, addr) & MPU_EXEC))
        return (struct Instruction) { 0, true };

    addr &= ~0x3;

    if (!ARM9->CP15.Control.ITCMWriteOnly && !(addr & ARM9->ITCMMask))
        return (struct Instruction) { *(u32*)&ITCM[addr & (ITCM_PhySize-1)], false };

    return (struct Instruction) { Bus9_Load32_Main(ARM9, addr), false };
}

bool Bus9_Load32(struct ARM946E_S* ARM9, u32 addr, u32* ret)
{
    if (!(ARM9_MPU_Lookup(ARM9, addr) & MPU_READ))
        return false;

    if (addr & 0x3) printf("ARM9 - UNALIGNED LOAD32: %08X %08X\n", addr, ARM9->PC);
    addr &= ~0x3;

    if (!ARM9->CP15.Control.ITCMWriteOnly && !(addr & ARM9->ITCMMask))
        *ret = *(u32*)&ITCM[addr & (ITCM_PhySize-1)];

    else if (!ARM9->CP15.Control.DTCMWriteOnly && ((addr & ARM9->DTCMMask) == ARM9->DTCMBase))
        *ret = *(u32*)&DTCM[addr & (DTCM_PhySize-1)];

    *ret = Bus9_Load32_Main(ARM9, addr);
    return true;
}

bool Bus9_Load16(struct ARM946E_S* ARM9, u32 addr, u16* ret)
{
    if (!(ARM9_MPU_Lookup(ARM9, addr) & MPU_READ))
        return false;

    if (addr & 0x1) printf("ARM9 - UNALIGNED LOAD16 %08X %08X\n", addr, ARM9->PC);
    addr &= ~0x1;

    if (!ARM9->CP15.Control.ITCMWriteOnly && !(addr & ARM9->ITCMMask))
        *ret = *(u16*)&ITCM[addr & (ITCM_PhySize-1)];

    else if (!ARM9->CP15.Control.DTCMWriteOnly && ((addr & ARM9->DTCMMask) == ARM9->DTCMBase))
        *ret = *(u16*)&DTCM[addr & (DTCM_PhySize-1)];

    *ret = Bus9_Load16_Main(ARM9, addr);
    return true;
}

bool Bus9_Load8(struct ARM946E_S* ARM9, u32 addr, u8* ret)
{
    if (!(ARM9_MPU_Lookup(ARM9, addr) & MPU_READ))
        return false;

    if (!ARM9->CP15.Control.ITCMWriteOnly && !(addr & ARM9->ITCMMask))
        *ret = ITCM[addr & (ITCM_PhySize-1)];

    else if (!ARM9->CP15.Control.DTCMWriteOnly && ((addr & ARM9->DTCMMask) == ARM9->DTCMBase))
        *ret = DTCM[addr & (DTCM_PhySize-1)];

    else *ret = Bus9_Load8_Main(ARM9, addr);
    return true;
}

bool Bus9_Store32(struct ARM946E_S* ARM9, u32 addr, const u32 val)
{
    if (!(ARM9_MPU_Lookup(ARM9, addr) & MPU_WRITE))
        return false;

    if (addr & 0x3) printf("ARM9 - UNALIGNED STORE32: %08X %08X\n", addr, ARM9->PC);
    addr &= ~0x3;

    if (!(addr & ARM9->ITCMMask))
        *(u32*)&ITCM[addr & (ITCM_PhySize-1)] = val;

    else if ((addr & ARM9->DTCMMask) == ARM9->DTCMBase)
        *(u32*)&DTCM[addr & (DTCM_PhySize-1)] = val;

    else Bus9_Store32_Main(ARM9, addr, val);
    return true;
}

bool Bus9_Store16(struct ARM946E_S* ARM9, u32 addr, const u16 val)
{
    if (!(ARM9_MPU_Lookup(ARM9, addr) & MPU_WRITE))
        return false;

    if (addr & 0x1) printf("ARM9 - UNALIGNED STORE16: %08X %08X\n", addr, ARM9->PC);
    addr &= ~0x1;

    if (!(addr & ARM9->ITCMMask))
        *(u16*)&ITCM[addr & (ITCM_PhySize-1)] = val;

    else if ((addr & ARM9->DTCMMask) == ARM9->DTCMBase)
        *(u16*)&DTCM[addr & (DTCM_PhySize-1)] = val;

    else Bus9_Store16_Main(ARM9, addr, val);
    return true;
}

bool Bus9_Store8(struct ARM946E_S* ARM9, u32 addr, const u8 val)
{
    if (!(ARM9_MPU_Lookup(ARM9, addr) & MPU_WRITE))
        return false;

    if (!(addr & ARM9->ITCMMask))
        ITCM[addr & (ITCM_PhySize-1)] = val;

    else if ((addr & ARM9->DTCMMask) == ARM9->DTCMBase)
        DTCM[addr & (DTCM_PhySize-1)] = val;

    else Bus9_Store8_Main(ARM9, addr, val);
    return true;
}
