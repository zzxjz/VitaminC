#include <stdio.h>
#include <string.h>
#include <stdbit.h>

#ifdef __AVX2__
#include <immintrin.h>
#endif

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

void ARM9_MPU_Update(struct ARM946E_S* ARM9)
{
    if (!ARM9->CP15.Control.MPU)
    {
        // mpu disabled:
        // cache and wbuffer are disabled, no exceptions are raised.
        ARM9->RegionPerms[0][0] = MPU_READ | MPU_WRITE | MPU_EXEC;
        ARM9->RegionPerms[1][0] = MPU_READ | MPU_WRITE | MPU_EXEC;
        // this combination of mask and base guarantees the region is hit
        // thus ensuring that all accesses recieve the proper flags
        ARM9->RegionMask[0] = 0x00000000;
        ARM9->RegionBase[0] = 0x00000000;
    }

    for (int i = 0; i < 8; i++)
    {
        if (!(ARM9->CP15.MPURegion[i].Enable)) // check if region is disabled
        {
            // this combination of mask and base guarantees the region cannot be hit, effectively disabling it
            ARM9->RegionMask[i] = 0x00000000;
            ARM9->RegionBase[i] = 0xFFFFFFFF;
        }

        u8 userflags;
        u8 privflags;
        u8 sharedflags = 0;

        if (ARM9->CP15.Control.DCache) // if dcache enabled globally
            sharedflags |= ((ARM9->CP15.DCacheConfig >> i) & 0x1) << MPU_DCACHE_SHIFT;

        // write buffer has no disable
        sharedflags |= ((ARM9->CP15.WBufferControl >> i) & 0x1) << MPU_BUFFER_SHIFT;

        if (ARM9->CP15.Control.ICache) // if icache enabled globally
            sharedflags |= ((ARM9->CP15.ICacheConfig >> i) & 0x1) << MPU_ICACHE_SHIFT;

        // do the lookup for region perms
        const u8 dataperms  = ((ARM9->CP15.MPUDataPerms  >> (i*4)) & 0xF);
        const u8 instrperms = ((ARM9->CP15.MPUInstrPerms >> (i*4)) & 0xF);

        switch(dataperms)
        {
        case 0b0000:
            privflags = MPU_NOACCESS;
            userflags = MPU_NOACCESS;
            break;
        case 0b0001:
            privflags = MPU_READ | MPU_WRITE;
            userflags = MPU_NOACCESS;
            break;
        case 0b0010:
            privflags = MPU_READ | MPU_WRITE;
            userflags = MPU_READ;
            break;
        case 0b0011:
            privflags = MPU_READ | MPU_WRITE;
            userflags = MPU_READ | MPU_WRITE;
            break;
        case 0b0101:
            privflags = MPU_READ;
            userflags = MPU_NOACCESS;
            break;
        case 0b0110:
            privflags = MPU_READ;
            userflags = MPU_READ;
            break;
        default:
            printf("ARM9 - \"UNPREDICTABLE\" MPU DATA REGION SETTINGS!!! %01X\n", dataperms);
            privflags = MPU_NOACCESS;
            userflags = MPU_NOACCESS;
            break;
        }

        switch(instrperms)
        {
        case 0b0000:
            break;
        case 0b0001:
        case 0b0101:
            privflags |= MPU_EXEC;
            break;
        case 0b0010:
        case 0b0011:
        case 0b0110:
            privflags |= MPU_EXEC;
            userflags |= MPU_EXEC;
            break;
        default:
            printf("ARM9 - \"UNPREDICTABLE\" MPU INSTRUCTION REGION SETTINGS!!! %01X\n", instrperms);
            break;
        }

        privflags |= sharedflags;
        userflags |= sharedflags;

        // calculate the region's bounds
        // whether an access is within the region or not is determined by taking an address
        // performing a bitwise AND against a "size mask"
        // and doing a comparison against the regions "base" address
        // this works due to the fact that region's base addresses are forcibly aligned to a multiple of their size
        u32 size = ARM9->CP15.MPURegion[i].Size + 1;
        if (size < 12) size = 12;
        ARM9->RegionMask[i] = 0xFFFFFFFF << size;
        ARM9->RegionBase[i] = ARM9->CP15.MPURegion[i].Base << 12;

        ARM9->RegionPerms[0][i] = userflags;
        ARM9->RegionPerms[1][i] = privflags;
    }
}

u8 ARM9_MPU_Lookup(const struct ARM946E_S* ARM9, const u32 addr)
{
#ifdef __AVX2__
    const u32 tmp[8] = {addr, addr, addr, addr, addr, addr, addr, addr};
    __m256i addrs; memcpy(&addrs, tmp, sizeof(addrs));
    __m256i bases; memcpy(&bases, ARM9->RegionBase, sizeof(bases));
    __m256i masks; memcpy(&masks, ARM9->RegionMask, sizeof(masks));

    u8 res = _mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(_mm256_and_si256(addrs, masks), bases)));

    u8 region = stdc_trailing_zeros(res);

    return ARM9->RegionPerms[ARM9->Mode != MODE_USR][region];
#else
    #pragma unroll 8
    for (int i = 0; i < 8; i++)
    {
        if ((addr & ARM9->RegionMask[i]) == ARM9->RegionBase[i])
            return ARM9->RegionPerms[ARM9->Mode != MODE_USR][i];
    }
    return MPU_NOACCESS;
#endif
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
        ARM9_MPU_Update(ARM9);
        printf("ARM9 - CP15: CONTROL REG WRITE!!!\n");
        break;
    }

    case 0x0200:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.DCacheConfig = val;
        ARM9_MPU_Update(ARM9);
        printf("ARM9 - CP15: DCACHE CONFIG WRITE!!!\n");
        break;
    }
    case 0x0201:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.ICacheConfig = val;
        ARM9_MPU_Update(ARM9);
        printf("ARM9 - CP15: ICACHE CONFIG WRITE!!!\n");
        break;
    }
    case 0x0300:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.WBufferControl = val;
        ARM9_MPU_Update(ARM9);
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
        ARM9_MPU_Update(ARM9);
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
        ARM9_MPU_Update(ARM9);
        printf("ARM9 - CP15: MPU LEGACY INSTRUCTION PERMISSIONS WRITE!!!\n");
        break;
    }
    case 0x0502:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.MPUDataPerms = val;
        ARM9_MPU_Update(ARM9);
        printf("ARM9 - CP15: MPU DATA PERMISSIONS WRITE!!!\n");
        break;
    }
    case 0x0503:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.MPUInstrPerms = val;
        ARM9_MPU_Update(ARM9);
        printf("ARM9 - CP15: MPU INSTRUCTION PERMISSIONS WRITE!!!\n");
        break;
    }
    
    case 0x0600: case 0x0601:
    case 0x0610: case 0x0611:
    case 0x0620: case 0x0621:
    case 0x0630: case 0x0631:
    case 0x0640: case 0x0641:
    case 0x0650: case 0x0651:
    case 0x0660: case 0x0661:
    case 0x0670: case 0x0671:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->CP15.MPURegion[(cmd >> 4) & 0xF].Data = val & 0xFFFFF03F;
        ARM9_MPU_Update(ARM9);
        if (cmd & 0x1) printf("ARM9 - CP15: \"UNPREDICTABLE\" MPU REGION WRITE?\n");
        else printf("ARM9 - CP15: MPU REGION WRITE!!!\n");
        break;
    }

    case 0x0704:
    case 0x0F82:
    {
        if (ARM9->Mode == MODE_USR) { printf("ARM9 - USR CP15 WRITE: %04X\n", cmd); break; }
        ARM9->Halted = true;
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

    case 0x0600: case 0x0601:
    case 0x0610: case 0x0611:
    case 0x0620: case 0x0621:
    case 0x0630: case 0x0631:
    case 0x0640: case 0x0641:
    case 0x0650: case 0x0651:
    case 0x0660: case 0x0661:
    case 0x0670: case 0x0671:
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
