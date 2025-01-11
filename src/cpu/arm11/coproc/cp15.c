#include <stdio.h>
#include "../interpreter.h"
#include "../../../types.h"
#include "../../../bus.h"

bool ARM11_CP15_PageTable_PrivilegeLookup(struct ARM11MPCore* ARM11, bool read, u8 domain, u8 ap)
{
    switch(domain)
    {
    case 0b00: // domain fault
        return false;
    case 0b01:
    {
        switch(ap)
        {
        case 0b000: // P: n/a - U: n/a
            return false;

        case 0b001: // P: r/w - U: n/a
            return (ARM11->Mode != 0x10);

        case 0b010: // P: r/w - U: r/o
            if (read) return true;
            else return (ARM11->Mode != 0x10);

        case 0b011: // P: r/w - U: r/w
            return true;

        case 0b100: // reserved
            printf("INVALID ACCESS PERMISSION 0b100!!!\n");
            return false;

        case 0b101: // P: r/o - U: r/o
            if (read) return (ARM11->Mode != 0x10);
            else return false;

        case 0b110: // P: r/o - U: n/a
            if (read) return true;
            else return false;

        case 0b111: // reserved
            printf("INVALID ACCESS PERMISSION 0b111!!!\n");
            return false;
        default: __builtin_unreachable();
        }
    }
    case 0b10: // reserved
        printf("INVALID DOMAIN PERM 0b10!!!\n");
        return false; // checkme
    case 0b11: // manager
        return true;
    default: __builtin_unreachable();
    }
}

bool ARM11_CP15_PageTable_Lookup(struct ARM11MPCore* ARM11, u32* addr, const u8 accesstype)
{
    if (!ARM11->CP15.MMU) return true;

    const u8 n = ARM11->CP15.TransTblControl & 0x7;
    u32 transmask = 0x3FFF >> n;
    u32 transmask2;

    u32 transtable;
    if (n && (*addr & ~transmask))
    {
        transtable = ARM11->CP15.TransTblBaseR1;
        transmask = 0x3FFF;
        transmask2 = 0xFFF00000;
    }
    else
    {
        transtable = ARM11->CP15.TransTblBaseR0;
        transmask2 = (0xFFF00000 >> n) & 0xFFF00000;
    }
    const u32 lookupaddr = (transtable & ~transmask) | ((*addr & transmask2) >> 18);
    union PageTableEntry entry;
    entry.Data = Bus11_PageTableLoad32(ARM11, lookupaddr);
    
    switch (entry.Section.Type)
    {
    case 0b00: // translation fault
        printf("TRANSLATION FAULT %08X\n", *addr);
        return false;
    case 0b01: // second table!!! WOOOOOOO!!!!!
    {
        const u32 lookupaddr2 = (entry.Coarse.LookupAddr << 10 | ((*addr & 0x000FF000) >> 10));
        union PageTableEntry entry2;
        entry2.Data = Bus11_PageTableLoad32(ARM11, lookupaddr2);
        
        const bool xp = ARM11->CP15.ExtPageTable;

        if (!xp)
        {
            printf("WAIT HOLD ON I DIDN'T FINISH THOSE!!!\n");
            switch(entry2.L2YSubpage.LargePage.Type)
            {
            case 0b00: // trans fault
                printf("TRANSLATION FAULT %08X\n", *addr);
                return false;
            case 0b01: // large page
            {
                *addr = (entry2.L2NSubpage.LargePage.BaseAddr << 16) | (*addr & 0x0000FFFF);
                return true;
            }
            case 0b10: // small page
            {
                *addr = (entry2.L2NSubpage.ExtSmallPage.BaseAddr << 12) | (*addr & 0x00000FFF);
                return true;
            }
            case 0b11: // ext small page
            {
                *addr = (entry2.L2NSubpage.ExtSmallPage.BaseAddr << 12) | (*addr & 0x00000FFF);
                return true;
            }
            default: __builtin_unreachable();
            }
        }
        else
        {
            switch(entry2.L2NSubpage.LargePage.Type)
            {
            case 0b00: // trans fault
                printf("TRANSLATION FAULT %08X\n", *addr);
                return false;
            case 0b01: // large page
            {
                if (entry2.L2NSubpage.LargePage.XN && accesstype & TLB_Instr) return false;

                const u8 domain = (ARM11->CP15.DomainAccessControl >> (entry.Coarse.Domain*2)) & 0x3;
                const u8 ap = (entry2.L2NSubpage.LargePage.APX << 2) | entry2.L2NSubpage.LargePage.AP;

                if (!ARM11_CP15_PageTable_PrivilegeLookup(ARM11, accesstype & TLB_Read, domain, ap)) return false;

                *addr = (entry2.L2NSubpage.LargePage.BaseAddr << 16) | (*addr & 0x0000FFFF);
                return true;
            }
            case 0b10: // ext small page
            case 0b11:
            {
                if (entry2.L2NSubpage.ExtSmallPage.XN && accesstype & TLB_Instr) return false;

                const u8 domain = (ARM11->CP15.DomainAccessControl >> (entry.Coarse.Domain*2)) & 0x3;
                const u8 ap = (entry2.L2NSubpage.ExtSmallPage.APX << 2) | entry2.L2NSubpage.ExtSmallPage.AP;

                if (!ARM11_CP15_PageTable_PrivilegeLookup(ARM11, accesstype & TLB_Read, domain, ap)) return false;

                *addr = (entry2.L2NSubpage.ExtSmallPage.BaseAddr << 12) | (*addr & 0x00000FFF);
                return true;
            }
            default: __builtin_unreachable();
            }
        }
    }
    case 0b10: // section/supersection
    {
        const bool xp = ARM11->CP15.ExtPageTable;

        if (entry.Section.Supersection && xp)
        {
            if (entry.Supersection.XN && accesstype & TLB_Instr) return false; // checkme: is this higher priority than a domain fault?

            const u8 domain = ARM11->CP15.DomainAccessControl & 0x3; // always domain 0
            const u8 ap = (entry.Supersection.APX << 2) | entry.Supersection.AP;

            if (!ARM11_CP15_PageTable_PrivilegeLookup(ARM11, accesstype & TLB_Read, domain, ap)) return false;

            *addr = (entry.Supersection.BaseAddr << 24) | (*addr & 0x00FFFFFF);
            return true;
        }
        else
        {
            if (xp && entry.Section.XN && accesstype & TLB_Instr) return false; // checkme: is this higher priority than a domain fault?

            const u8 domain = (ARM11->CP15.DomainAccessControl >> (entry.Section.Domain*2)) & 0x3;
            u8 ap = entry.Section.AP;
            if (xp) ap |= entry.Section.APX << 2;

            if (!ARM11_CP15_PageTable_PrivilegeLookup(ARM11, accesstype & TLB_Read, domain, ap)) return false;

            *addr = (entry.Section.BaseAddr << 20) | (*addr & 0x000FFFFF);
            return true; 
        }
    }
    case 0b11: // reserved?
        printf("RESERVED PAGE TABLE MODE????\n");
        return false; // checkme?
    default: __builtin_unreachable();
    }
}

void ARM11_CP15_Store_Single(struct ARM11MPCore* ARM11, u16 cmd, u32 val)
{
    switch(cmd)
    {
    case 0x0100:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        const u32 changemask = 0x3AC0BB03;
        ARM11->CP15.Control &= ~changemask;
        ARM11->CP15.Control |= val & changemask;
        printf("CP15 CONTROL REG WRITE!!!\n");
        break;
    }
    case 0x0101:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        ARM11->CP15.AuxControl = val & 0x7F;
        printf("CP15 AUX CONTROL REG WRITE!!!\n");
        break;
    }
    case 0x0102:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        ARM11->CP15.CoprocAccessControl = val & (0xF<<20); // checkme?
        printf("CP15 COPROCESSOR ACCESS CONTROL REG WRITE!!!\n");
        break;
    }

    case 0x0200:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        const u32 changemask = 0xFFFFFFFA; // checkme?
        ARM11->CP15.TransTblBaseR0 = val & changemask;
        printf("CP15 TRANS TABLE BASE R0 WRITE!!!\n");
        break;
    }
    case 0x0201:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        const u32 changemask = 0xFFFFC01E; // checkme?
        ARM11->CP15.TransTblBaseR1 = val & changemask;
        printf("CP15 TRANS TABLE BASE R1 WRITE!!!\n");
        break;
    }
    case 0x0202:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        const u8 changemask = 0x7;
        ARM11-> CP15.TransTblControl = val & changemask;
        printf("CP15 TRANS TABLE CONTROL WRITE!!!\n");
        break;
    }

    case 0x0300:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        ARM11->CP15.DomainAccessControl = val;
        printf("CP15 DOMAIN ACCESS CONTROL WRITE!!!\n");
        break;
    }

    case 0x0500:
    {
        const u16 changemask = 0x1CFF;
        ARM11->CP15.DataFaultStatus = val & changemask;
        break;
    }
    case 0x0501:
    {
        const u16 changemask = 0x14FF;
        ARM11->CP15.InstrFaultStatus = val & changemask;
        break;
    }

    case 0x0600:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        ARM11->CP15.DataFaultAddress = val;
        break;
    }
    case 0x0601:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break; // checkme?
        ARM11->CP15.WatchpointFaultAddress = val;
        break;
    }

    case 0x0704:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: WAIT FOR INTERRUPT!!!\n");
        break;
    }
    case 0x0750:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE ENTIRE ICACHE!!!\n");
        break;
    }
    case 0x0751:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE ICACHE MVA!!!\n");
        break;
    }
    case 0x0752:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE ICACHE IDX!!!\n");
        break;
    }
    case 0x0754:
    {
        printf("CP15: FLUSH PREFETCH!!!\n");
        break;
    }
    case 0x0756:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: FLUSH ENTIRE BRANCH CACHE!!!\n");
        break;
    }
    case 0x0757:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: FLUSH BRANCH CACHE ENTRY!!!\n");
        break;
    }
    case 0x0760:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE ENTIRE DCACHE!!!\n");
        break;
    }
    case 0x0761:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE DCACHE MVA!!!\n");
        break;
    }
    case 0x0762:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE DCACHE IDX!!!\n");
        break;
    }
    case 0x0770:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE ENTIRE I&DCACHE!!!\n");
        break;
    }
    case 0x0780:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: VA TO PA PRIVILEGED READ!!!\n");
        break;
    }
    case 0x0781:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: VA TO PA PRIVILEGED WRITE!!!\n");
        break;
    }
    case 0x0782:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: VA TO PA USER READ!!!\n");
        break;
    }
    case 0x0783:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: VA TO PA USER WRITE!!!\n");
        break;
    }
    case 0x07A0:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: CLEAN ENTIRE DCACHE!!!\n");
        break;
    }
    case 0x07A1:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: CLEAN DCACHE MVA!!!\n");
        break;
    }
    case 0x07A2:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: CLEAN DCACHE IDX!!!\n");
        break;
    }
    case 0x07A4:
    {
        //if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break; checkme?
        printf("CP15: DATA SYNC BARRIER!!!\n");
        break;
    }
    case 0x07A5:
    {
        //if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break; checkme?
        printf("CP15: DATA MEMORY BARRIER!!!\n");
        break;
    }
    case 0x07E0:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: CLEAN+INVAL ENTIRE DCACHE!!!\n");
        break;
    }
    case 0x07E1:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: CLEAN+INVAL DCACHE MVA!!!\n");
        break;
    }
    case 0x07E2:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: CLEAN+INVAL DCACHE IDX!!!\n");
        break;if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd);break;
    }

    case 0x0850:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE ENTIRE ITLB!!!\n");
        break;
    }
    case 0x0851:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE ITLB by MVA w/ ASID?!!!\n");
        break;
    }
    case 0x0852:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE ITLB ASID!!!\n");
        break;
    }
    case 0x0853:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE ITLB MVA!!!\n");
        break;
    }
    case 0x0860:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE ENTIRE DTLB!!!\n");
        break;
    }
    case 0x0861:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE DTLB by MVA w/ ASID?!!!\n");
        break;
    }
    case 0x0862:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE DTLB ASID!!!\n");
        break;
    }
    case 0x0863:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE DTLB MVA!!!\n");
        break;
    }
    case 0x0870:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE ENTIRE UTLB!!!\n");
        break;
    }
    case 0x0871:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE UTLB by MVA w/ ASID?!!!\n");
        break;
    }
    case 0x0872:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE UTLB ASID!!!\n");
        break;
    }
    case 0x0873:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        printf("CP15: INVALIDATE UTLB MVA!!!\n");
        break;
    }

    case 0x0900:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        // checkme?
        ARM11->CP15.DCacheLockdown &= ~0xF;
        ARM11->CP15.DCacheLockdown |= val & 0xF;
        printf("CP15: DCACHE LOCKDOWN!!!\n");
        break;
    }

    case 0x0A00:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        const u32 changemask = 0x1C000001;
        ARM11->CP15.TLBLockdown &= ~changemask;
        ARM11->CP15.TLBLockdown |= val & changemask;
        printf("CP15: TLB LOCKDOWN!!!\n");
        break;
    }
    case 0x0A20:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        const u32 changemask = 0x000FCFFF;
        ARM11->CP15.PrimaryMemRgnRemap = val & changemask;
        printf("CP15: PRIMARY MEMORY REGION REMAP WRITE!!!\n");
        break;
    }
    case 0x0A21:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        const u32 changemask = 0xCFFFCFFF;
        ARM11->CP15.NormalMemRgnRemap = val & changemask;
        printf("CP15: NORMAL MEMORY REGION REMAP WRITE!!!\n");
        break;
    }

    case 0x0D00:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        const u32 changemask = 0xFE000000; // checkme?
        ARM11->CP15.FCSEPID = val & changemask;
        printf("CP15: FCSEPID WRITE!!!\n");
        break;
    }
    case 0x0D01:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        ARM11->CP15.ContextID = val;
        printf("CP15: CONTEXTID WRITE!!!\n");
        break;
    }
    case 0x0D02:
    {
        ARM11->CP15.ThreadID[0] = val;
        printf("CP15: THREADID0 WRITE!!!\n");
        break;
    }
    case 0x0D03:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        ARM11->CP15.ThreadID[1] = val;
        printf("CP15: THREADID1 WRITE!!!\n");
        break;
    }
    case 0x0D04:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break;
        ARM11->CP15.ThreadID[2] = val;
        printf("CP15: THREADID2 WRITE!!!\n");
        break;
    }
    
    case 0x0FC0:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break; // checkme?
        const u32 changemask = 0x0FFFF07F;
        const u32 IRQmask = 0x00000700;
        ARM11->CP15.PerfMonitorControl &= ~(val & IRQmask); 
        ARM11->CP15.PerfMonitorControl = val & changemask;
        printf("CP15: PERF MONITOR CNT WRITE!!!\n");
        break;
    }
    case 0x0FC1:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break; // checkme?
        printf("CP15: CYCLE COUNTER WRITE!!!\n");
        break;
    }
    case 0x0FC2:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break; // checkme?
        printf("CP15: COUNT0 WRITE!!!\n");
        break;
    }
    case 0x0FC3:
    {
        if (ARM11->Mode == MODE_USR) printf("USR CP15 WRITE: %04X\n", cmd); break; // checkme?
        printf("CP15: COUNT1 WRITE!!!\n");
        break;
    }
    default:
        printf("UNIMPLEMENTED CP15 WRITE %04X\n", cmd);
        break;
    }
}

u32 ARM11_CP15_Load_Single(struct ARM11MPCore* ARM11, u16 cmd)
{
    if ((ARM11->Mode == MODE_USR) && ((cmd != 0x0D02) || (cmd != 0x0D01))) { printf("CP15 USERMODE READ: CMD: %04X\n", cmd); return 0; }

    switch(cmd)
    {
    case 0x0000: return 0x410FB024;
    //case 0x0001: return 0x00000000; // todo: figure out cachesizes
    case 0x0003: return 0x00000800;
    case 0x0005: return ARM11->CPUID; // checkme: clusterid?

    case 0x0010: return 0x00000111;
    case 0x0011: return 0x00000001;
    case 0x0012: return 0x00000002;
    case 0x0014: return 0x01100103;
    case 0x0015: return 0x10020302;
    case 0x0016: return 0x01222000;
    case 0x0017: return 0x00000000;

    case 0x0020: return 0x00100011;
    case 0x0021: return 0x12002111;
    case 0x0022: return 0x11221011;
    case 0x0023: return 0x01102131;
    case 0x0024: return 0x00000141;

    case 0x0100: return ARM11->CP15.Control;
    case 0x0101: return ARM11->CP15.AuxControl;
    case 0x0102: return ARM11->CP15.CoprocAccessControl;

    case 0x0200: return ARM11->CP15.TransTblBaseR0;
    case 0x0201: return ARM11->CP15.TransTblBaseR1;
    case 0x0202: return ARM11->CP15.TransTblControl;

    case 0x0300: return ARM11->CP15.DomainAccessControl;
    
    case 0x0500: return ARM11->CP15.DataFaultStatus;
    case 0x0501: return ARM11->CP15.InstrFaultStatus;

    case 0x0600: return ARM11->CP15.DataFaultAddress;
    case 0x0601: return ARM11->CP15.WatchpointFaultAddress;

    //case 0x0740: return idk;

    case 0x0900: return ARM11->CP15.DCacheLockdown;

    case 0x0A00: return ARM11->CP15.TLBLockdown;
    case 0x0A20: return ARM11->CP15.PrimaryMemRgnRemap;
    case 0x0A21: return ARM11->CP15.NormalMemRgnRemap;

    case 0x0D00: return ARM11->CP15.FCSEPID;
    case 0x0D01: return ARM11->CP15.ContextID;
    case 0x0D02: return ARM11->CP15.ThreadID[0];
    case 0x0D03: return ARM11->CP15.ThreadID[1];
    case 0x0D04: return ARM11->CP15.ThreadID[2];

    case 0x0FC0: return ARM11->CP15.PerfMonitorControl;
    default: printf("UNIMPLEMENTED CP15 READ: CMD: %04X\n", cmd); return 0;
    }
}
