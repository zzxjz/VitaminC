#include <string.h>
#include <stdio.h>
#include "arm.h"
#include "../../utils.h"
#include "../../emu.h"
#include "bus.h"
#include "../shared/arm.h"

#define CHECK(x, y, z) if (PatternMatch((struct Pattern) {0b##x, 0b##y}, bits)) { z(ARM11); } else

void THUMB11_DecodeMiscThumb(struct ARM11MPCore* ARM11)
{
    const u16 bits = (ARM11->Instr.Data >> 3) & 0x1FF;

    CHECK(0000'0000'0, 1111'0000'0, THUMB11_ADD_SUB_SP) // adjust stack ptr
    CHECK(0010'0000'0, 1111'0000'0, THUMB11_Extend) // sign/zero ext
    CHECK(0100'0000'0, 1110'0000'0, THUMB11_PUSH) // push
    CHECK(1100'0000'0, 1110'0000'0, THUMB11_POP) // pop
        //CHECK(011001000, 111111110, NULL) // mystery meat instruction :D
    //CHECK(011001010, 111111110, NULL) // setend
    //CHECK(011001100, 111111101, NULL) // cps
        //CHECK(011001101, 111111111, NULL) // is secret :)
        //CHECK(011001111, 111111111, NULL) // it is a mystery :ghost:
    //CHECK(101000000, 111100000, NULL) // rev
    //CHECK(111000000, 111000000, NULL) // bkpt
    //NULL();
    printf("ARM11 - UNIMPLEMENTED MISC THUMB INSTRUCTION %04X\n", ARM11->Instr.Data);
}

#undef CHECK
#define CHECK(x, y, z) if (PatternMatch((struct Pattern) {0b##x, 0b##y}, bits)) { return z; } else

void* ARM11_InitARMInstrLUT(const u16 bits)
{
    // media instruction space
    CHECK(0110'0000'0001, 1111'1000'0001, NULL) // parallel add/sub
    CHECK(0110'1000'0001, 1111'1111'0011, NULL) // halfword pack
    CHECK(0110'1010'0001, 1111'1010'0011, NULL) // word saturate
    CHECK(0110'1010'0011, 1111'1011'1111, NULL) // parallel halfword saturate
    CHECK(0110'1011'0011, 1111'1111'1111, NULL) // byte reverse word
    CHECK(0110'1011'1011, 1111'1111'1111, NULL) // byte reverse packed halfword
    CHECK(0110'1111'1011, 1111'1111'1111, NULL) // byte reverse signed halfword
    CHECK(0110'1000'1011, 1111'1111'1111, NULL) // select bytes
    CHECK(0110'1000'0111, 1111'1000'1111, NULL) // sign/zero extend (add)
    CHECK(0111'0000'0001, 1111'1000'0001, NULL) // multiplies (type 3)
    CHECK(0111'1000'0001, 1111'1111'1111, NULL) // unsigned sum of absolute differences (also acc variant?)
    // multiply extension space
    CHECK(000'00000'1001, 1111'1100'1111, ARM11_MUL_MLA) // short multiplies
    CHECK(000'00100'1001, 1111'1111'1111, NULL) // umaal
    CHECK(000'01000'1001, 1111'1000'1111, NULL) // long multiplies
    // control/dsp extension space
    CHECK(0001'0000'0000, 1111'1011'1111, ARM11_MRS) // mrs
    CHECK(0001'0010'0000, 1111'1011'1111, ARM11_MSRReg) // msr (reg)
    CHECK(0011'0010'0000, 1111'1011'0000, ARM11_MSRImm_Hints) // msr (imm) / hints
    CHECK(0001'0010'0001, 1111'1111'1101, ARM11_BX_BLXReg) // bx/blx (reg)
    CHECK(0001'0010'0010, 1111'1111'1111, NULL) // bxj
    CHECK(0001'0110'0001, 1111'1111'1111, NULL) // clz
    CHECK(0001'0000'0100, 1111'1001'1111, NULL) // q(d)add/sub
    CHECK(0001'0010'0111, 1111'1111'1111, NULL) // bkpt
    CHECK(0001'0000'1000, 1111'1001'1001, NULL) // signed multiplies
    // load/store extension space
    CHECK(0001'0000'1001, 1111'1011'1111, NULL) // swp
    CHECK(0001'1000'1001, 1111'1000'1111, NULL) // ldrex/strex (and variants)
    CHECK(0000'0000'1011, 1110'0000'1111, NULL) // ldrh/strh
    CHECK(0000'0001'1101, 1110'0001'1101, NULL) // ldrsh/ldrsb
    CHECK(0000'0000'1101, 1110'0001'1101, NULL) // ldrd/strd
    // explicitly defined undefined space
    CHECK(0111'1111'1111, 1111'1111'1111, NULL) // undef instr
    // coproc extension space
    CHECK(1100'0000'0000, 1111'1010'0000, NULL) // coprocessor?
    // data processing
    CHECK(0000'0000'0000, 1100'0000'0000, ARM11_ALU) // alu
    // load/store
    CHECK(0100'0000'0000, 1100'0000'0000, ARM11_LDR_STR) // load/store
    // coprocessor data processing
    CHECK(1110'0000'0000, 1111'0000'0001, NULL) // cdp?
    // coprocessor register transfers
    CHECK(1110'0000'0001, 1111'0001'0001, ARM11_MCR_MRC) // mcr
    CHECK(1110'0001'0001, 1111'0001'0001, ARM11_MCR_MRC) // mrc
    // multiple load/store
    CHECK(1000'0000'0000, 1110'0000'0000, ARM11_LDM_STM) // ldm/stm
    // branch
    CHECK(1010'0000'0000, 1110'0000'0000, ARM11_B_BL) // b/bl

    return NULL; // undef instr (raise exception)
}

void* ARM11_InitTHUMBInstrLUT(const u8 bits)
{
    CHECK(0001'10, 1111'10, THUMB11_ADD_SUB_Reg_Imm3); // add/sub reg/imm
    CHECK(0000'00, 1110'00, THUMB11_ShiftImm); // shift by imm
    CHECK(0010'00, 1110'00, THUMB11_ADD_SUB_CMP_MOV_Imm8); // add/sub/cmp/mov imm
    CHECK(0100'00, 1111'11, THUMB11_ALU); // data proc reg
    CHECK(0100'01, 1111'11, THUMB11_ALU_HI); // spec data proc/b(l)x
    CHECK(0100'10, 1111'10, THUMB11_LDRPCRel); // ldr pcrel
    CHECK(0101'00, 1111'00, THUMB11_LDR_STR_Reg); // ldr/str reg
    CHECK(0110'00, 1110'00, THUMB11_LDR_STR_Imm5); // (ld/st)r(b) imm
    CHECK(1000'00, 1111'00, THUMB11_LDRH_STRH_Imm5); // ldrh/strh imm
    CHECK(1001'00, 1111'00, THUMB11_LDR_STR_SPRel); // ldr/str sprel
    CHECK(1010'00, 1111'00, THUMB11_ADD_SP_PCRel); // add sp/pcrel
    CHECK(1011'00, 1111'00, THUMB11_DecodeMiscThumb); // misc
    CHECK(1100'00, 1111'00, THUMB11_LDMIA_STMIA); // ldm/stm
    CHECK(1101'00, 1111'00, THUMB11_CondB_SWI); // cond b/udf/svc
    CHECK(1110'00, 1111'10, THUMB11_B); // b
    CHECK(1111'00, 1111'10, THUMB11_BL_BLX_LO); // bl(x) prefix
    CHECK(1110'10, 1110'10, THUMB11_BL_BLX_HI); // bl(x) suffix
    return NULL; // udf
}

void* ARM11_DecodeUncondInstr(const u32 bits)
{
    CHECK(0001'0000'0000'0000'0000'0000'0000, 1111'1111'0001'0000'0000'0010'0000, NULL) // cps
    CHECK(0001'0000'0001'0000'0000'0000'0000, 1111'1111'1111'0000'0000'1111'0000, NULL) // setend
    CHECK(0101'0101'0000'1111'0000'0000'0000, 1101'0111'0000'1111'0000'0000'0000, ARM11_PLD) // pld
    CHECK(1000'0100'1101'0000'0101'0000'0000, 1110'0101'1111'0000'1111'0000'0000, NULL) // srs
    CHECK(1000'0001'0000'0000'1010'0000'0000, 1110'0101'0000'0000'1111'0000'0000, NULL) // rfe
    CHECK(1010'0000'0000'0000'0000'0000'0000, 1110'0000'0000'0000'0000'0000'0000, ARM11_BLX_Imm) // blx (imm)
    CHECK(1100'0100'0000'0000'0000'0000'0000, 1111'1110'0000'0000'0000'0000'0000, NULL) // cp double reg
    CHECK(1110'0000'0000'0000'0000'0001'0000, 1111'0000'0000'0000'0000'0001'0000, NULL) // cp reg transfer?
    CHECK(1111'0000'0000'0000'0000'0000'0000, 1111'0000'0000'0000'0000'0000'0000, NULL) // undef instr
    return NULL; // undefined instr? (checkme)
}

#undef CHECK

alignas(64) void (*ARM11_InstrLUT[0x1000]) (struct ARM11MPCore* ARM11);
alignas(64) void (*THUMB11_InstrLUT[0x40]) (struct ARM11MPCore* ARM11);

// WARNING: only pass a pointer to the __FIRST__ member of the 4 element "ARM11" array to this function
void ARM11_HardReset(struct Console* console)
{
    struct ARM11MPCore* ARM11 = console->ARM11;

    for (int i = 0; i < 4; i++)
    {
        ARM11[i].console = console;
        ARM11[i].NextStep = ARM11_StartFetch;
        ARM11_Branch(&ARM11[i], 0, false);
        ARM11[i].Mode = MODE_SVC;
        ARM11[i].SPSR = &ARM11[i].SVC.SPSR;
        ARM11[i].CP15.Control = 0x00054078;
        ARM11[i].CP15.AuxControl = 0x0F;
        ARM11[i].CP15.DCacheLockdown = 0xFFFFFFF0;
        ARM11[i].CPUID = i;
        ARM11[i].PrivRgn.IRQConfig[0] = 0xAA;
        ARM11[i].PrivRgn.IRQConfig[1] = 0xAA;
        ARM11[i].PrivRgn.IRQConfig[2] = 0xAA;
        ARM11[i].PrivRgn.IRQConfig[3] = 0xAA;
        ARM11[i].FIQDisable = true;
        ARM11[i].IRQDisable = true;
    }
}

void ARM11_UpdateMode(struct ARM11MPCore* ARM11, u8 oldmode, u8 newmode)
{
    if (oldmode == newmode) return;

    switch (oldmode)
    {
    case 0x10: // USR
    case 0x1F: // SYS
        memcpy(&ARM11->USR.R8, &ARM11->R8, 7*4);
        break;

    case 0x11: // FIQ
        memcpy(&ARM11->FIQ.R8, &ARM11->R8, 7*4);
        break;

    case 0x12: // IRQ
        memcpy(&ARM11->USR.R8, &ARM11->R8, 5*4);
        memcpy(&ARM11->IRQ.SP, &ARM11->SP, 2*4);
        break;

    case 0x13: // SVC
        memcpy(&ARM11->USR.R8, &ARM11->R8, 5*4);
        memcpy(&ARM11->SVC.SP, &ARM11->SP, 2*4);
        break;

    case 0x17: // ABT
        memcpy(&ARM11->USR.R8, &ARM11->R8, 5*4);
        memcpy(&ARM11->ABT.SP, &ARM11->SP, 2*4);
        break;
    
    case 0x1B: // UND
        memcpy(&ARM11->USR.R8, &ARM11->R8, 5*4);
        memcpy(&ARM11->UND.SP, &ARM11->SP, 2*4);
        break;
    }

    switch (newmode)
    {
    case 0x10: // USR
    case 0x1F: // SYS
        memcpy(&ARM11->R8, &ARM11->USR.R8, 7*4);
        ARM11->SPSR = NULL;
        break;

    case 0x11: // FIQ
        memcpy(&ARM11->R8, &ARM11->FIQ.R8, 7*4);
        ARM11->SPSR = &ARM11->FIQ.SPSR;
        break;

    case 0x12: // IRQ
        memcpy(&ARM11->R8, &ARM11->USR.R8, 5*4);
        memcpy(&ARM11->SP, &ARM11->IRQ.SP, 2*4);
        ARM11->SPSR = &ARM11->IRQ.SPSR;
        break;

    case 0x13: // SVC
        memcpy(&ARM11->R8, &ARM11->USR.R8, 5*4);
        memcpy(&ARM11->SP, &ARM11->SVC.SP, 2*4);
        ARM11->SPSR = &ARM11->SVC.SPSR;
        break;

    case 0x17: // ABT
        memcpy(&ARM11->R8, &ARM11->USR.R8, 5*4);
        memcpy(&ARM11->SP, &ARM11->ABT.SP, 2*4);
        ARM11->SPSR = &ARM11->ABT.SPSR;
        break;
    
    case 0x1B: // UND
        memcpy(&ARM11->R8, &ARM11->USR.R8, 5*4);
        memcpy(&ARM11->SP, &ARM11->UND.SP, 2*4);
        ARM11->SPSR = &ARM11->UND.SPSR;
        break;
    }
}

void ARM11_MRS(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;
    const bool r = curinstr & (1<<22);
    const u8 rd = (curinstr >> 12) & 0xF;

    u32 psr;
    if (r) psr = ARM11->SPSR ? *ARM11->SPSR : 0;
    else psr = ARM11->CPSR;
    ARM11_WriteReg(ARM11, rd, psr, false, false);
}

void ARM11_MSR(struct ARM11MPCore* ARM11, u32 val)
{
    const u32 curinstr = ARM11->Instr.Data;
    const bool r = curinstr & (1<<22);
    const bool c = curinstr & (1<<16);
    const bool x = curinstr & (1<<17);
    const bool s = curinstr & (1<<18);
    const bool f = curinstr & (1<<19);

    u32 writemask;
    if (ARM11->Mode == MODE_USR)
    {
        writemask = 0xF80F0200;
    }
    else
    {
        writemask = 0xF90F03FF;
    }

    if (!c) writemask &= 0xFFFFFF00;
    if (!x) writemask &= 0xFFFF00FF;
    if (!s) writemask &= 0xFF00FFFF;
    if (!f) writemask &= 0x00FFFFFF;

    val &= writemask;

    if (r)
    {
        if (!ARM11->SPSR) return;
        *ARM11->SPSR &= ~writemask;
        *ARM11->SPSR |= val;
    }
    else
    {
        if (val & 0x01000020) printf("ARM11 - MSR SETTING T OR J BIT!!! PANIC!!!!!!\n");

        val |= ARM11->CPSR & ~writemask;
        ARM11_UpdateMode(ARM11, ARM11->Mode, val&0x1F);
        ARM11->CPSR = val;
        //printf("ARM11 - CPSR UPDATE: %08X\n", ARM11->CPSR);
    }
}

void ARM11_MSRReg(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;
    const u32 val = ARM11_GetReg(ARM11, curinstr & 0xF);

    ARM11_MSR(ARM11, val);
}

void ARM11_MSRImm_Hints(struct ARM11MPCore* ARM11)
{
    const u32 curinstr = ARM11->Instr.Data;
    const bool op = curinstr & (1<<22);
    const u8 op1 = (curinstr >> 16) & 0xF;
    const u8 op2 = curinstr & 0xFF;

    if (op1) // msr immediate
    {
        const u8 rorimm = ((curinstr >> 8) & 0xF) * 2;
        const u32 val = ROR32(op2, rorimm);
        ARM11_MSR(ARM11, val);
    }
    else if (!op) // hints
    {
        switch(op2)
        {
        case 0x00: // nop
            return;
        case 0x01: // yield
            printf("ARM11 - UNIMPLEMENTED HINT: YIELD!!!\n"); return;
        case 0x02: // wfe (wait for event)
            ARM11->WaitForEvent = true; return;
        case 0x03: // wfi (wait for interrupt)
            ARM11->WaitForInterrupt = true; return;
        case 0x04: // sev (send event)
            printf("ARM11 - UNIMPLEMENTED HINT: SEV\n"); return;
        case 0x14: // csdb (consumption of speculative data barrier?????????)
            printf("ARM11 - UNIMPLEMENTED HINT: CSDB\n"); return;
        default:
            printf("ARM11 - INVALID HINT %02X\n", op2); return;
        }
    }
}

u32 ARM11_CodeFetch(struct ARM11MPCore* ARM11)
{
    return Bus11_InstrLoad32(ARM11, ARM11->PC);
}

void ARM11_Branch(struct ARM11MPCore* ARM11, u32 addr, const bool restore)
{
#if 0
    if (addr != 0x00019220 &&
        addr != 0x0001920C &&
        //addr != 0x00018084 &&
        addr != 0x00014B45 &&
        addr != 0x00012103)
        printf("ARM11: Jumping to %08X from %08X via %08X\n", addr, ARM11->PC, ARM11->Instr.Data);
#endif
    if (restore)
    {
        const u32 spsr = *ARM11->SPSR;
        ARM11_UpdateMode(ARM11, ARM11->Mode, spsr & 0x1F);
        ARM11->CPSR = spsr;
        printf("ARM11 - CPSR RESTORE: %08X\n", ARM11->CPSR);

        addr &= ~0x1;
        addr |= ARM11->Thumb;
    }

    if (addr & 0x1)
    {
        ARM11->Thumb = true;

        addr &= ~0x1;
        ARM11->PC = addr;

        if (addr & 0x2)
        {
            ARM11->Instr.Data = ARM11_CodeFetch(ARM11);
        }
    }
    else
    {
        ARM11->Thumb = false;

        addr &= ~0x3;
        ARM11->PC = addr;
    }
}

inline u32 ARM11_GetReg(struct ARM11MPCore* ARM11, const int reg)
{
    if (reg == 15)
    {
        return ARM11->PC + (ARM11->Thumb ? 2 : 4);
    }
    else return ARM11->R[reg];
}

inline void ARM11_WriteReg(struct ARM11MPCore* ARM11, const int reg, u32 val, const bool restore, const bool canswap)
{
    if (reg == 15)
    {
        if (!canswap)
        {
            if (ARM11->Thumb) val |= 1;
            else val &= ~1;
        }
        ARM11_Branch(ARM11, val, restore);
    }
    else
    {
        ARM11->R[reg] = val;
    }
}

void ARM11_StartFetch(struct ARM11MPCore* ARM11)
{
    if (!(ARM11->PC & 0x2)) ARM11->Instr.Data = ARM11_CodeFetch(ARM11);
    else ARM11->Instr.Data >>= 16;

    ARM11->PC += (ARM11->Thumb ? 2 : 4);
}

void ARM11_StartExec(struct ARM11MPCore* ARM11)
{
    const u32 instr = ARM11->Instr.Data;
    const u8 condcode = instr >> 28;
    
    // Todo: handle IRQs
    if (CondLookup[condcode] & (1<<ARM11->Flags))
    {
        const u16 decodebits = ((instr >> 16) & 0xFF0) | ((instr >> 4) & 0xF);

        if (ARM11_InstrLUT[decodebits])
            (ARM11_InstrLUT[decodebits])(ARM11);
        else
        {
            printf("ARM11 - UNIMPL ARM INSTR: %08X @ %08X!!!\n", ARM11->Instr.Data, ARM11->PC);
            for (int i = 0; i < 16; i++) printf("%i, %08X ", i, ARM11->R[i]);
            while (true)
                ;
        }
        //if (ARM11->SP & 0x3) { printf("MISALIGNMENT!! %08X\n", ARM11->Instr.Data); while (true); }
        //for (int i = 0; i < 16; i++) printf("%i, %08X ", i, ARM11->R[i]);
        //printf("\n");
    }
    else if (condcode == COND_NV) // do special handling for unconditional instructions
    {
        void (*func)(struct ARM11MPCore*) = ARM11_DecodeUncondInstr(instr);
        if (func) func(ARM11);
        else
        {
            printf("ARM11 - UNCOND: %08X!!!!\n", ARM11->Instr.Data);
            for (int i = 0; i < 16; i++) printf("%i, %08X ", i, ARM11->R[i]);
            while (true)
                ;
        }
    }
    else if (false) // TODO: handle illegal BKPTs? might still execute regardless of condition fail like the ARM9?
    {
    }
    else
    {
        // instruction was not executed.
        //printf("INSTR: %08X SKIP\n", ARM11->Instr.Data);
    }
}

void THUMB11_StartExec(struct ARM11MPCore* ARM11)
{
    const u16 instr = ARM11->Instr.Data;

    const u8 decodebits = instr >> 10;

    if (THUMB11_InstrLUT[decodebits])
        (THUMB11_InstrLUT[decodebits])(ARM11);
    else
    {
        printf("ARM11 - UNIMPL THUMB INSTR: %04X @ %08X\n", instr, ARM11->PC);
        while(true)
            ;
    }
    //if (ARM11->SP & 0x3) { printf("MISALIGNMENT!! %04X @ %08X\n", ARM11->Instr.Data & 0xFFFF, ARM11->PC); while (true); }
}

void ARM11_RunInterpreter(struct ARM11MPCore* ARM11, u64 target)
{
    while (ARM11->Timestamp <= target)
    {
        //(ARM11->NextStep)(ARM11);
        if (ARM11->Halted)
        {
            if (ARM11->WaitForInterrupt)
            {
                // wake up on irq, fiq, or asynch abt regardless of masking out
            }
            else if (ARM11->WaitForEvent)
            {
                // wake up on:
                // sev executed by any other arm11
                // irq if !i bit of cpsr
                // fiq if !f bit of cpsr
                // async abt if !a bit of cpsr
                // timer event stream?
                // "implementation defined events"
                // event register...?
            }
        }
        else
        {
            ARM11_StartFetch(ARM11);
            if (ARM11->Thumb) THUMB11_StartExec(ARM11);
            else ARM11_StartExec(ARM11);
        }
        ARM11->Timestamp++;
        //printf("times: %li %li\n", ARM11->Timestamp, target);
    }
}
