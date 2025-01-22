#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../utils.h"
#include "arm.h"
#include "../../emu.h"
#include "bus.h"

#define CHECK(x, y, z) if (PatternMatch((struct Pattern) {0b##x, 0b##y}, bits)) { z(ARM9); } else

void THUMB9_DecodeMiscThumb(struct ARM946E_S* ARM9)
{
    const u16 bits = (ARM9->Instr[0].Data >> 3) & 0x1FF;

    CHECK(0000'0000'0, 1111'0000'0, THUMB9_ADD_SUB_SP) // adjust stack ptr
    CHECK(0100'0000'0, 1110'0000'0, THUMB9_PUSH) // push
    CHECK(1100'0000'0, 1110'0000'0, THUMB9_POP) // pop
    CHECK(1110'0000'0, 1110'0000'0, ARM9_PrefetchAbort) // bkpt
    ARM9_UndefinedInstruction(ARM9);
}

#undef CHECK
#define CHECK(x, y, z) if (PatternMatch((struct Pattern) {0b##x, 0b##y}, bits)) { return z; } else

void* ARM9_InitARMInstrLUT(const u16 bits)
{
    // multiply extension space
    CHECK(0000'0000'1001, 1111'1100'1111, ARM9_MUL_MLA) // short multiplies
    CHECK(0000'1000'1001, 1111'1000'1111, NULL) // long multiplies
    // control/dsp extension space
    CHECK(0001'0000'0000, 1111'1011'1111, ARM9_MRS) // mrs
    CHECK(0001'0010'0000, 1111'1011'1111, ARM9_MSRReg) // msr (reg)
    CHECK(0011'0010'0000, 1111'1011'0000, ARM9_MSRImm) // msr (imm)
    CHECK(0001'0010'0001, 1111'1111'1101, ARM9_BX_BLXReg) // bx/blx (reg)
    CHECK(0001'0010'0010, 1111'1111'1111, NULL) // bxj checkme: does this exist on arm9? does it decode to anything of use?
    CHECK(0001'0110'0001, 1111'1111'1111, NULL) // clz
    CHECK(0001'0000'0100, 1111'1001'1111, NULL) // q(d)add/sub
    CHECK(0001'0010'0111, 1111'1111'1111, ARM9_PrefetchAbort) // bkpt
    CHECK(0001'0000'1000, 1111'1001'1001, NULL) // signed multiplies
    // load/store extension space
    CHECK(0001'0000'1001, 1111'1011'1111, NULL) // swp
    CHECK(0001'1000'1001, 1111'1000'1111, NULL) // ldrex/strex (and variants)
    CHECK(0000'0000'1011, 1110'0000'1111, NULL) // ldrh/strh
    CHECK(0000'0001'1101, 1110'0001'1101, NULL) // ldrsh/ldrsb
    CHECK(0000'0000'1101, 1110'0001'1101, NULL) // ldrd/strd
    // explicitly defined undefined space
    CHECK(0111'1111'1111, 1111'1111'1111, ARM9_UndefinedInstruction) // undef instr
    // coproc extension space
    CHECK(1100'0000'0000, 1111'1010'0000, ARM9_UndefinedInstruction) // coprocessor? - checkme: longer undef?
    // data processing
    CHECK(0000'0000'0000, 1100'0000'0000, ARM9_ALU) // alu
    // load/store
    CHECK(0100'0000'0000, 1100'0000'0000, ARM9_LDR_STR) // load/store
    // coprocessor data processing
    CHECK(1110'0000'0000, 1111'0000'0001, ARM9_UndefinedInstruction) // cdp? - checkme: longer undef?
    // coprocessor register transfers
    CHECK(1110'0000'0001, 1111'0001'0001, ARM9_MCR_MRC) // mcr
    CHECK(1110'0001'0001, 1111'0001'0001, ARM9_MCR_MRC) // mrc
    // multiple load/store
    CHECK(1000'0000'0000, 1110'0000'0000, ARM9_LDM_STM) // ldm/stm
    // branch
    CHECK(1010'0000'0000, 1110'0000'0000, ARM9_B_BL) // b/bl

    return ARM9_UndefinedInstruction; // undef instr (raise exception)
}

void* ARM9_InitTHUMBInstrLUT(const u8 bits)
{
    CHECK(0001'10, 1111'10, THUMB9_ADD_SUB_Reg_Imm3); // add/sub reg/imm
    CHECK(0000'00, 1110'00, THUMB9_ShiftImm); // shift by imm
    CHECK(0010'00, 1110'00, THUMB9_ADD_SUB_CMP_MOV_Imm8); // add/sub/cmp/mov imm
    CHECK(0100'00, 1111'11, THUMB9_ALU); // data proc reg
    CHECK(0100'01, 1111'11, THUMB9_ALU_HI); // spec data proc/b(l)x
    CHECK(0100'10, 1111'10, THUMB9_LDRPCRel); // ldr pcrel
    CHECK(0101'00, 1111'00, THUMB9_LDR_STR_Reg); // ldr/str reg
    CHECK(0110'00, 1110'00, THUMB9_LDR_STR_Imm5); // (ld/st)r(b) imm
    CHECK(1000'00, 1111'00, THUMB9_LDRH_STRH_Imm5); // ldrh/strh imm
    CHECK(1001'00, 1111'00, THUMB9_LDR_STR_SPRel); // ldr/str sprel
    CHECK(1010'00, 1111'00, THUMB9_ADD_SP_PCRel); // add sp/pcrel
    CHECK(1011'00, 1111'00, THUMB9_DecodeMiscThumb); // misc
    CHECK(1100'00, 1111'00, THUMB9_LDMIA_STMIA); // ldm/stm
    CHECK(1101'00, 1111'00, THUMB9_CondB_SWI); // cond b/udf/svc
    CHECK(1110'00, 1111'10, THUMB9_B); // b
    CHECK(1111'00, 1111'10, THUMB9_BL_BLX_LO); // bl(x) prefix
    CHECK(1110'10, 1110'10, THUMB9_BL_BLX_HI); // bl(x) suffix
    return ARM9_UndefinedInstruction; // udf
}

void* ARM9_DecodeUncondInstr(const u32 bits)
{
    CHECK(0101'0101'0000'1111'0000'0000'0000, 1101'0111'0000'1111'0000'0000'0000, ARM9_PLD) // pld
    CHECK(1010'0000'0000'0000'0000'0000'0000, 1110'0000'0000'0000'0000'0000'0000, ARM9_BLX_Imm) // blx (imm)
    CHECK(1100'0100'0000'0000'0000'0000'0000, 1111'1110'0000'0000'0000'0000'0000, ARM9_UndefinedInstruction) // cp double reg - longer undef?
    CHECK(1110'0000'0000'0000'0000'0001'0000, 1111'0000'0000'0000'0000'0001'0000, ARM9_UndefinedInstruction) // cp reg transfer? - longer undef?
    CHECK(0001'0010'0000'0000'0000'0111'0000, 1111'1111'0000'0000'0000'1111'0000, ARM9_PrefetchAbort) // bkpt - checkme?
    return ARM9_UndefinedInstruction; // undefined instr? (checkme)
}

#undef CHECK

alignas(64) void (*ARM9_InstrLUT[0x1000]) (struct ARM946E_S* ARM9);
alignas(64) void (*THUMB9_InstrLUT[0x40]) (struct ARM946E_S* ARM9);

void ARM9_HardReset(struct Console* console)
{
    struct ARM946E_S* ARM9 = &console->ARM9;
    ARM9->console = console;
    ARM9->NextStep = ARM9_StartFetch;
    ARM9->Mode = MODE_SVC;
    ARM9->SPSR = &ARM9->SVC.SPSR;
    ARM9->CP15.Control.Data = 0x00002078; // checkme: itcm could be enabled on boot?
    ARM9->CP15.DTCMRegion.Size = 5;
    ARM9->CP15.ITCMRegion.Size = 6;
    ARM9->DTCMMask = 0x00000000;
    ARM9->DTCMBase = 0xFFFFFFFF;
    ARM9->FIQDisable = true;
    ARM9->IRQDisable = true;
    ARM9_MPU_Update(ARM9);
    ARM9_Branch(ARM9, 0xFFFF0000, false);
}

u32 ARM9_GetExceptionVector(struct ARM946E_S* ARM9)
{
    return ARM9->CP15.Control.HighVector ? 0xFFFF0000 : 0x00000000;
}

void ARM9_UpdateMode(struct ARM946E_S* ARM9, u8 oldmode, u8 newmode)
{
    if (oldmode == newmode) return;

    switch (oldmode)
    {
    case 0x10: // USR
    case 0x1F: // SYS
        memcpy(&ARM9->USR.R8, &ARM9->R8, 7*4);
        break;

    case 0x11: // FIQ
        memcpy(&ARM9->FIQ.R8, &ARM9->R8, 7*4);
        break;

    case 0x12: // IRQ
        memcpy(&ARM9->USR.R8, &ARM9->R8, 5*4);
        memcpy(&ARM9->IRQ.SP, &ARM9->SP, 2*4);
        break;

    case 0x13: // SVC
        memcpy(&ARM9->USR.R8, &ARM9->R8, 5*4);
        memcpy(&ARM9->SVC.SP, &ARM9->SP, 2*4);
        break;

    case 0x17: // ABT
        memcpy(&ARM9->USR.R8, &ARM9->R8, 5*4);
        memcpy(&ARM9->ABT.SP, &ARM9->SP, 2*4);
        break;

    case 0x1B: // UND
        memcpy(&ARM9->USR.R8, &ARM9->R8, 5*4);
        memcpy(&ARM9->UND.SP, &ARM9->SP, 2*4);
        break;
    }

    switch (newmode)
    {
    case 0x10: // USR
    case 0x1F: // SYS
        memcpy(&ARM9->R8, &ARM9->USR.R8, 7*4);
        ARM9->SPSR = NULL;
        break;

    case 0x11: // FIQ
        memcpy(&ARM9->R8, &ARM9->FIQ.R8, 7*4);
        ARM9->SPSR = &ARM9->FIQ.SPSR;
        break;

    case 0x12: // IRQ
        memcpy(&ARM9->R8, &ARM9->USR.R8, 5*4);
        memcpy(&ARM9->SP, &ARM9->IRQ.SP, 2*4);
        ARM9->SPSR = &ARM9->IRQ.SPSR;
        break;

    case 0x13: // SVC
        memcpy(&ARM9->R8, &ARM9->USR.R8, 5*4);
        memcpy(&ARM9->SP, &ARM9->SVC.SP, 2*4);
        ARM9->SPSR = &ARM9->SVC.SPSR;
        break;

    case 0x17: // ABT
        memcpy(&ARM9->R8, &ARM9->USR.R8, 5*4);
        memcpy(&ARM9->SP, &ARM9->ABT.SP, 2*4);
        ARM9->SPSR = &ARM9->ABT.SPSR;
        break;

    case 0x1B: // UND
        memcpy(&ARM9->R8, &ARM9->USR.R8, 5*4);
        memcpy(&ARM9->SP, &ARM9->UND.SP, 2*4);
        ARM9->SPSR = &ARM9->UND.SPSR;
        break;
    }
}

void ARM9_UndefinedInstruction(struct ARM946E_S* ARM9)
{
    printf("ARM9 - UNDEFINED INSTRUCTION: PC = %08X INSTR = %08X\n", ARM9->PC, ARM9->Instr[0].Data);

    ARM9_UpdateMode(ARM9, ARM9->Mode, MODE_UND);
    *ARM9->SPSR = ARM9->CPSR;
    ARM9->LR = ARM9_GetReg(ARM9, 15) - (ARM9->Thumb ? 2 : 4);
    ARM9->Mode = MODE_UND;
    ARM9->Thumb = false;
    ARM9->IRQDisable = true;
    ARM9_Branch(ARM9, ARM9_GetExceptionVector(ARM9) + VECTOR_UND, false);
}

void ARM9_SupervisorCall(struct ARM946E_S* ARM9)
{
    ARM9_UpdateMode(ARM9, ARM9->Mode, MODE_SVC);
    *ARM9->SPSR = ARM9->CPSR;
    ARM9->LR = ARM9_GetReg(ARM9, 15) - (ARM9->Thumb ? 2 : 4);
    ARM9->Mode = MODE_SVC;
    ARM9->Thumb = false;
    ARM9->IRQDisable = true;
    ARM9_Branch(ARM9, ARM9_GetExceptionVector(ARM9) + VECTOR_SVC, false);
}

void ARM9_PrefetchAbort(struct ARM946E_S* ARM9)
{
    if (ARM9->Instr[0].Data != 0)
    {
        printf("ARM9 - BKPT: PC = %08X\n", ARM9->PC);
    }
    else
    {
        printf("ARM9 - PREFETCH ABORT: PC = %08X\n", ARM9->PC);
        ARM9_MPU_PrintRGN(ARM9);
    }

    ARM9_UpdateMode(ARM9, ARM9->Mode, MODE_ABT);
    *ARM9->SPSR = ARM9->CPSR;
    ARM9->LR = ARM9_GetReg(ARM9, 15) - (ARM9->Thumb ? 0 : 4);
    ARM9->Mode = MODE_ABT;
    ARM9->Thumb = false;
    ARM9->IRQDisable = true;
    ARM9_Branch(ARM9, ARM9_GetExceptionVector(ARM9) + VECTOR_PAB, false);
    while(true)
        ;
}

void ARM9_DataAbort(struct ARM946E_S* ARM9, const bool Write, const u32 abortaddr)
{
    printf("ARM9 - %s DATA ABORT: ADDR = %08X PC = %08X\n", Write ? "STORE" : "LOAD", abortaddr, ARM9->PC);
    ARM9_MPU_PrintRGN(ARM9);
    for (int i = 0; i < 16; i++)
        printf("%08X ", ARM9->R[i]);
    printf("\n");

    ARM9_UpdateMode(ARM9, ARM9->Mode, MODE_ABT);
    *ARM9->SPSR = ARM9->CPSR;
    ARM9->LR = ARM9_GetReg(ARM9, 15) + (ARM9->Thumb ? 4 : 0);
    ARM9->Mode = MODE_ABT;
    ARM9->Thumb = false;
    ARM9->IRQDisable = true;
    ARM9_Branch(ARM9, ARM9_GetExceptionVector(ARM9) + VECTOR_DAB, false);
}

void ARM9_InterruptRequest(struct ARM946E_S* ARM9)
{
    ARM9_UpdateMode(ARM9, ARM9->Mode, MODE_IRQ);
    *ARM9->SPSR = ARM9->CPSR;
    ARM9->LR = ARM9_GetReg(ARM9, 15) - (ARM9->Thumb ? 0 : 4);
    ARM9->Mode = MODE_IRQ;
    ARM9->Thumb = false;
    ARM9->IRQDisable = true;
    ARM9_Branch(ARM9, ARM9_GetExceptionVector(ARM9) + VECTOR_IRQ, false);
}

void ARM9_MRS(struct ARM946E_S* ARM9)
{
    const u32 curinstr = ARM9->Instr[0].Data;
    const bool r = curinstr & (1<<22);
    const u8 rd = (curinstr >> 12) & 0xF;

    u32 psr;
    if (r) psr = ARM9->SPSR ? *ARM9->SPSR : 0;
    else psr = ARM9->CPSR;
    ARM9_WriteReg(ARM9, rd, psr, false, false);
}

void ARM9_MSR(struct ARM946E_S* ARM9, u32 val)
{
    const u32 curinstr = ARM9->Instr[0].Data;
    const bool r = curinstr & (1<<22);
    const bool c = curinstr & (1<<16);
    const bool x = curinstr & (1<<17);
    const bool s = curinstr & (1<<18);
    const bool f = curinstr & (1<<19);

    u32 writemask = 0;

    if (f) writemask |= 0xF8000000;
    if (c && !(ARM9->Mode == MODE_USR)) writemask |= 0xEF;

    val &= writemask;

    if (r)
    {
        if (!ARM9->SPSR) return;
        *ARM9->SPSR &= ~writemask;
        *ARM9->SPSR |= val;
    }
    else
    {
        if (val & 0x01000020) printf("MSR SETTING T BIT!!! PANIC!!!!!!\n");

        val |= ARM9->CPSR & ~writemask;
        ARM9_UpdateMode(ARM9, ARM9->Mode, val&0x1F);
        ARM9->CPSR = val;
        //printf("ARM9 - CPSR UPDATE: %08X\n", ARM9->CPSR);
    }
}

void ARM9_MSRReg(struct ARM946E_S* ARM9)
{
    const u32 curinstr = ARM9->Instr[0].Data;
    const u32 val = ARM9_GetReg(ARM9, curinstr & 0xF);

    ARM9_MSR(ARM9, val);
}

void ARM9_MSRImm(struct ARM946E_S* ARM9)
{
    const u32 curinstr = ARM9->Instr[0].Data;
    const bool op = curinstr & (1<<22);
    const u8 op1 = (curinstr >> 16) & 0xF;
    const u8 op2 = curinstr & 0xFF;

    const u8 rorimm = ((curinstr >> 8) & 0xF) * 2;
    const u32 val = ROR32(op2, rorimm);
    ARM9_MSR(ARM9, val);
}

void ARM9_Branch(struct ARM946E_S* ARM9, u32 addr, const bool restore)
{
#if 0
    if (addr != 0xFFFF8208)
        printf("ARM9: Jumping to %08X from %08X via %08X\n", addr, ARM9->PC, ARM9->Instr[0].Data);
#endif

    if (restore)
    {
        const u32 spsr = *ARM9->SPSR;
        ARM9_UpdateMode(ARM9, ARM9->Mode, spsr & 0x1F);
        ARM9->CPSR = spsr;
        printf("ARM9 - CPSR RESTORE: %08X\n", ARM9->CPSR);

        addr &= ~0x1;
        addr |= ARM9->Thumb;
    }

    if (addr & 0x1)
    {
        ARM9->Thumb = true;

        addr &= ~0x1;
        ARM9->PC = addr + 2;

        if (addr & 0x2)
        {
            ARM9->Instr[1] = Bus9_InstrLoad32(ARM9, addr-2);
            ARM9->Instr[1].Data >>= 16;
            ARM9->Instr[2] = Bus9_InstrLoad32(ARM9, addr+2);
        }
        else
        {
            ARM9->Instr[1] = Bus9_InstrLoad32(ARM9, addr);
            ARM9->Instr[2] = ARM9->Instr[1];
            ARM9->Instr[2].Data >>= 16;
        }
    }
    else
    {
        ARM9->Thumb = false;

        addr &= ~0x3;
        ARM9->PC = addr + 4;

        ARM9->Instr[1] = Bus9_InstrLoad32(ARM9, addr);
        ARM9->Instr[2] = Bus9_InstrLoad32(ARM9, addr+4);
    }
}

inline u32 ARM9_GetReg(struct ARM946E_S* ARM9, const int reg)
{
    if (reg == 15)
    {
        return ARM9->PC;
    }
    else return ARM9->R[reg];
}

inline void ARM9_WriteReg(struct ARM946E_S* ARM9, const int reg, u32 val, const bool restore, const bool canswap)
{
    if (reg == 15)
    {
        if (!canswap)
        {
            if (ARM9->Thumb) val |= 1;
            else val &= ~1;
        }
        ARM9_Branch(ARM9, val, restore);
    }
    else
    {
        ARM9->R[reg] = val;
    }
}

void ARM9_StartFetch(struct ARM946E_S* ARM9)
{
    ARM9->PC += (ARM9->Thumb ? 2 : 4); // todo: this should technically be incremented *after* the fetch is done

    ARM9->Instr[0] = ARM9->Instr[1];
    if (!(ARM9->PC & 0x2))
    {
        ARM9->Instr[1] = ARM9->Instr[2];
        ARM9->Instr[2] = Bus9_InstrLoad32(ARM9, ARM9->PC);
    }
    else
    {
        ARM9->Instr[1] = ARM9->Instr[2];
        ARM9->Instr[2].Data >>= 16;
    }
}

void ARM9_StartExec(struct ARM946E_S* ARM9)
{
    const u32 instr = ARM9->Instr[0].Data;
    const u8 condcode = instr >> 28;

    // Todo: handle IRQs
    if (ARM9->Instr[0].PrefetchAbort)
    {
        ARM9_PrefetchAbort(ARM9);
    }
    else if (CondLookup[condcode] & (1<<ARM9->Flags))
    {
        const u16 decodebits = ((instr >> 16) & 0xFF0) | ((instr >> 4) & 0xF);

        if (ARM9_InstrLUT[decodebits])
            (ARM9_InstrLUT[decodebits])(ARM9);
        else
        {
            printf("ARM9 - UNIMPL ARM INSTR: %08X @ %08X!!!\n", ARM9->Instr[0].Data, ARM9->PC);
            for (int i = 0; i < 16; i++) printf("%i, %08X ", i, ARM9->R[i]);
            while (true)
                ;
        }
        //for (int i = 0; i < 16; i++) printf("%i, %08X ", i, ARM9->R[i]);
        //printf("%08X\n", ARM9->Instr[0].Data);
    }
    else if (condcode == COND_NV) // do special handling for unconditional instructions
    {
        ((void(*)(struct ARM946E_S*))ARM9_DecodeUncondInstr(instr))(ARM9);
    }
    else if ((ARM9->Instr[0].Data & 0x0FF000F0) == 0x01200070) // BKPT always executes, ignoring it's condition bits
    {
        ARM9_PrefetchAbort(ARM9);
    }
    else
    {
        // instruction was not executed.
        //printf("INSTR: %08X SKIP\n", ARM9->Instr[0].Data);
    }
}

void THUMB9_StartExec(struct ARM946E_S* ARM9)
{
    const u16 instr = ARM9->Instr[0].Data;

    const u8 decodebits = instr >> 10;

    (THUMB9_InstrLUT[decodebits])(ARM9);
}

void ARM9_RunInterpreter(struct ARM946E_S* ARM9, u64 target)
{
    while (ARM9->Timestamp <= target)
    {
        //(ARM9->NextStep)(ARM9);
        if (ARM9->Halted)
        {
            while (true)
                ;
            if (false) // IRQSignaled = IE & IF;
            {
                ARM9->WaitForInterrupt = false;
                if (!ARM9->IRQDisable)
                {
                    
                }
            }
        }
        else
        {
            ARM9_StartFetch(ARM9);
            if (ARM9->Thumb) THUMB9_StartExec(ARM9);
            else ARM9_StartExec(ARM9);
        }
        ARM9->Timestamp++;
    }
}
