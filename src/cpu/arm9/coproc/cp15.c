#include <stdio.h>
#include "../arm.h"
#include "../../../utils.h"

void ARM9_UpdateITCM(struct ARM946E_S* ARM9)
{
    if (!ARM9->CP15.Control.ITCM) ARM9->ITCMMask = 0xFFFFFFFF;
    else
    {
        u32 size = ARM9->CP15.ITCMRegion.Size;
        if (size < 3) size = 3; // checkme?

        ARM9->ITCMMask = 0xFFFFFE00 << size;
    }
}

void ARM9_UpdateDTCM(struct ARM946E_S* ARM9)
{
    if (!ARM9->CP15.Control.DTCM)
    {
        // this combo should prevent it from ever hitting dtcm
        ARM9->DTCMMask = 0x00000000;
        ARM9->DTCMBase = 0xFFFFFFFF;
    }
    else
    {
        u32 size = ARM9->CP15.DTCMRegion.Size;
        if (size < 3) size = 3; // checkme?

        ARM9->DTCMMask = 0xFFFFFE00 << size;

        size += 9;
        ARM9->DTCMBase = ARM9->CP15.DTCMRegion.Data & ARM9->DTCMMask; // size aligned
    }
}

void ARM9_CP15_Store_Single(struct ARM946E_S* ARM9, u16 cmd, u32 val)
{
    switch(cmd)
    {
    case 0x0100:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }

        const u32 changemask = 0x000FF085;
        ARM9->CP15.Control.Data &= ~changemask;
        ARM9->CP15.Control.Data |= val & changemask;

        ARM9_UpdateDTCM(ARM9);
        ARM9_UpdateITCM(ARM9);
        printf("ARM9 - CP15: CONTROL REG WRITE!!!\n");
        break;
    }

    case 0x0200:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.DCacheConfig = val;
        printf("ARM9 - CP15: DCACHE CONFIG WRITE!!!\n");
        break;
    }
    case 0x0201:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.ICacheConfig = val;
        printf("ARM9 - CP15: ICACHE CONFIG WRITE!!!\n");
        break;
    }
    case 0x0300:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.WBufferControl = val;
        printf("ARM9 - CP15: WRITE BUFFER CONTROL WRITE!!!\n");
        break;
    }

    case 0x0500:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.MPUDataPerms = 0;
        for (int i = 0; i < 8; i++)
        {
            u8 perm = (val >> (i*2)) & 0x3;
            ARM9->CP15.MPUDataPerms |= perm << (i*4);
        }
        printf("ARM9 - CP15: LEGACY MPU DATA PERMISSIONS WRITE!!!\n");
        break;
    }
    case 0x0501:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.MPUInstrPerms = 0;
        for (int i = 0; i < 8; i++)
        {
            u8 perm = (val >> (i*2)) & 0x3;
            ARM9->CP15.MPUInstrPerms |= perm << (i*4);
        }
        printf("ARM9 - CP15: MPU LEGACY INSTRUCTION PERMISSIONS WRITE!!!\n");
        break;
    }
    case 0x0502:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.MPUDataPerms = val;
        printf("ARM9 - CP15: MPU DATA PERMISSIONS WRITE!!!\n");
        break;
    }
    case 0x0503:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.MPUInstrPerms = val;
        printf("ARM9 - CP15: MPU INSTRUCTION PERMISSIONS WRITE!!!\n");
        break;
    }
    
    case 0x0600:
    case 0x0610:
    case 0x0620:
    case 0x0630:
    case 0x0640:
    case 0x0650:
    case 0x0660:
    case 0x0670:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.MPURegion[(cmd >> 4) & 0xF].Data = val & 0xFFFFF03F;
        printf("ARM9 - CP15: MPU REGION WRITE!!!\n");
        break;
    }

    case 0x0704:
    case 0x0F82:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        printf("ARM9 - CP15: WAIT FOR INTERRUPT!!!\n");
        break;
    }
    case 0x0750:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        printf("ARM9 - CP15: INVALIDATE ENTIRE ICACHE!!!\n");
        break;
    }
    case 0x0751:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        printf("ARM9 - CP15: INVALIDATE ICACHE MVA!!!\n");
        break;
    }
    case 0x0760:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        printf("ARM9 - CP15: INVALIDATE ENTIRE DCACHE!!!\n");
        break;
    }
    case 0x0761:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        printf("ARM9 - CP15: INVALIDATE DCACHE MVA!!!\n");
        break;
    }
    case 0x07A1:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        printf("ARM9 - CP15: CLEAN DCACHE MVA!!!\n");
        break;
    }
    case 0x07A4:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        printf("ARM9 - CP15: DRAIN WRITE BUFFER\n");
        break;
    }
    case 0x07E1:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        printf("ARM9 - CP15: CLEAN+INVAL DCACHE MVA!!!\n");
        break;
    }

    case 0x0900:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        // checkme?
        ARM9->CP15.DCacheLockDown.Data = val & 0x80000003;
        printf("ARM9 - CP15: DCACHE LOCKDOWN!!!\n");
        break;
    }
    case 0x0901:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        // checkme?
        ARM9->CP15.ICacheLockDown.Data = val & 0x80000003;
        printf("ARM9 - CP15: ICACHE LOCKDOWN!!!\n");
        break;
    }

    case 0x0910:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }

        ARM9->CP15.DTCMRegion.Data = val & 0xFFFFF03E;
        ARM9_UpdateDTCM(ARM9);

        printf("ARM9 - CP15: DTCM REGION WRITE!!!\n");
        break;
    }
    case 0x0911:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }

        ARM9->CP15.ITCMRegion.Data = val & 0xFFFFF03E; // checkme: can you write to the base addr bits?
        ARM9_UpdateITCM(ARM9);

        printf("ARM9 - CP15: ITCM REGION WRITE!!!\n");
        break;
    }

    case 0x0D01:
    case 0x0D10:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.ProcessID = val;
        break;
    }

    default:
        printf("UNIMPLEMENTED CP15 WRITE %04X\n", cmd);
        break;
    }
}

u32 ARM9_CP15_Load_Single(struct ARM946E_S* ARM9, u16 cmd)
{
    if (ARM9->Mode == MODE_USR) { printf("CP15 USERMODE READ: CMD: %04X\n", cmd); return 0; }

    switch(cmd)
    {
    case 0x0000:
    case 0x0003 ... 0x0007: return 0x41059461;
    case 0x0001: return 0x0F0D2112;
    case 0x0002: return 0x00140180;

    case 0x0100: return ARM9->CP15.Control.Data;

    case 0x0200: return ARM9->CP15.ICacheConfig;
    case 0x0201: return ARM9->CP15.DCacheConfig;
    case 0x0300: return ARM9->CP15.WBufferControl;
    
    case 0x0500:
    {
        u32 ret = 0;
        for (int i = 0; i < 8; i++)
            ret |= ((ARM9->CP15.MPUDataPerms >> (i*4)) & 0x3) << (i*2);
        return ret;
    }
    case 0x0501:
    {
        u32 ret = 0;
        for (int i = 0; i < 8; i++)
            ret |= ((ARM9->CP15.MPUInstrPerms >> (i*4)) & 0x3) << (i*2);
        return ret;
    }
    case 0x0502: return ARM9->CP15.MPUDataPerms;
    case 0x0503: return ARM9->CP15.MPUInstrPerms;

    case 0x0600:
    case 0x0610:
    case 0x0620:
    case 0x0630:
    case 0x0640:
    case 0x0650:
    case 0x0660:
    case 0x0670:
        return ARM9->CP15.MPURegion[(cmd>>4)&0xF].Data;

    case 0x0900: return ARM9->CP15.DCacheLockDown.Data;
    case 0x0901: return ARM9->CP15.ICacheLockDown.Data;

    case 0x0910: return ARM9->CP15.DTCMRegion.Data;
    case 0x0911: return ARM9->CP15.ITCMRegion.Data;

    case 0x0D01:
    case 0x0D11:
        return ARM9->CP15.ProcessID;

    default: printf("UNIMPLEMENTED CP15 READ: CMD: %04X\n", cmd); return 0;
    }
}
