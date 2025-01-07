#include <string.h>
#include "interpreter.h"
#include "../../patternmatch.h"
#include "../../types.h"

// stolen from melonds oops
alignas(32) static const u16 CondLookup[16] =
{
    0xF0F0, // EQ
    0x0F0F, // NE
    0xCCCC, // CS
    0x3333, // CC
    0xFF00, // MI
    0x00FF, // PL
    0xAAAA, // VS
    0x5555, // VC
    0x0C0C, // HI
    0xF3F3, // LS
    0xAA55, // GE
    0x55AA, // LT
    0x0A05, // GT
    0xF5FA, // LE
    0xFFFF, // AL
    0x0000  // NV
};

inline u32 ARM11_ROR32(u32 val, u8 ror)
{
	return (val >> ror) | (val << (32-ror));
}

#define CHECK(x, y, z) if (PatternMatch((struct Pattern) {0b##x, 0b##y}, bits)) { return z; } else

void* ARM11_InitInstrLUT(const u16 bits)
{
	// media instruction space
	CHECK(011000000001, 111110000001, NULL) // parallel add/sub
	CHECK(011010000001, 111111110011, NULL) // halfword pack
	CHECK(011010100001, 111110100011, NULL) // word saturate
	CHECK(011010100011, 111110111111, NULL) // parallel halfword saturate
	CHECK(011010110011, 111111111111, NULL) // byte reverse word
	CHECK(011010111011, 111111111111, NULL) // byte reverse packed halfword
	CHECK(011011111011, 111111111111, NULL) // byte reverse signed halfword
	CHECK(011010001011, 111111111111, NULL) // select bytes
	CHECK(011010000111, 111110001111, NULL) // sign/zero extend (add)
	CHECK(011100000001, 111110000001, NULL) // multiplies (type 3)
	CHECK(011110000001, 111111111111, NULL) // unsigned sum of absolute differences (also acc variant?)
	// multiply extension space
	CHECK(000000001001, 111111001111, NULL) // short multiplies
	CHECK(000001001001, 111111111111, NULL) // umaal
	CHECK(000010001001, 111110001111, NULL) // long multiplies
	// control/dsp extension space
	CHECK(000100000000, 111110111111, NULL) // mrs
	CHECK(000100100000, 111110111111, NULL) // msr (reg)
	CHECK(001100100000, 111110110000, NULL) // msr (imm) / hints
	CHECK(000100100001, 111111111111, NULL) // bx
	CHECK(000100100010, 111111111111, NULL) // bxj
	CHECK(000101100001, 111111111111, NULL) // clz
	CHECK(000100100011, 111111111111, NULL) // blx (reg)
	CHECK(000100000100, 111110011111, NULL) // q(d)add/sub
	CHECK(000100100111, 111111111111, NULL) // bkpt
	CHECK(000100001000, 111110011001, NULL) // signed multiplies
	// load/store extension space
	CHECK(000100001001, 111110111111, NULL) // swp
	CHECK(000110001001, 111110001111, NULL) // ldrex/strex (and variants)
	CHECK(000000001011, 111000001111, NULL) // ldrh/strh
	CHECK(000000011101, 111000011101, NULL) // ldrsh/ldrsb
	CHECK(000000001101, 111000011101, NULL) // ldrd/strd
	// explicitly defined undefined space
	CHECK(011111111111, 111111111111, NULL) // undef instr
	// coproc extension space
	CHECK(110000000000, 111110100000, NULL) // coprocessor?
	// data processing
	CHECK(000000000000, 110000000000, ARM11_ALU) // alu
	// load/store
	CHECK(010000000000, 110000000000, ARM11_LDR_STR) // load/store
	// coprocessor data processing
	CHECK(111000000000, 111100000001, NULL) // cdp?
	// coprocessor register transfers
	CHECK(111000000001, 111100000001, NULL) // mcr/mrc
	// multiple load/store
	CHECK(100000000000, 111000000000, NULL) // ldm/stm
	// branch
	CHECK(101000000000, 111000000000, NULL) // b/bl

	return NULL; // undef instr (raise exception)
}

void* DecodeUncondInstr(const u32 bits)
{
	CHECK(0001000000000000000000000000, 1111111100010000000000100000, NULL) // cps
	CHECK(0001000000010000000000000000, 1111111111110000000011110000, NULL) // setend
	CHECK(0101010100001111000000000000, 1101011100001111000000000000, NULL) // pld
	CHECK(1000010011010000010100000000, 1110010111110000111100000000, NULL) // srs
	CHECK(1000000100000000101000000000, 1110010100000000111100000000, NULL) // rfe
	CHECK(1010000000000000000000000000, 1110000000000000000000000000, NULL) // blx (imm)
	CHECK(1100010000000000000000000000, 1111111000000000000000000000, NULL) // cp double reg
	CHECK(1110000000000000000000010000, 1111000000000000000000010000, NULL) // cp reg transfer?
	CHECK(1111000000000000000000000000, 1111000000000000000000000000, NULL) // undef instr
	return NULL; // undefined instr? (checkme)
}

#undef CHECK

alignas(64) void (*ARM11_InstrLUT[0x1000]) (struct ARM11MPCore* ARM11);

struct ARM11MPCore ARM11[4];

char* ARM11_Init()
{
	memset(ARM11, 0, sizeof(ARM11));
	for (int i = 0; i < 4; i++)
	{
		ARM11[i].NextStep = ARM11_StartFetch;
		ARM11_Branch(&ARM11[i], 0xFFFF0000);
		ARM11[i].Mode = MODE_SVC;
	}
	return NULL;
}

void ARM11_Branch(struct ARM11MPCore* ARM11, const u32 addr)
{
	ARM11->PC = addr;
}

inline u32 ARM11_GetReg(struct ARM11MPCore* ARM11, const int reg)
{
	return ARM11->R[reg];
}

inline void ARM11_WriteReg(struct ARM11MPCore* ARM11, const int reg, const u32 val, const bool restore)
{
	if (reg == 15)
	{
		ARM11_Branch(ARM11, val);
	}
	else
	{
		ARM11->R[reg] = val;
	}
}

void ARM11_CodeFetch(struct ARM11MPCore* ARM11)
{

}

void ARM11_StartFetch(struct ARM11MPCore* ARM11)
{
	ARM11->PC += 4;
}

void ARM11_StartExec(struct ARM11MPCore* ARM11)
{
    const u32 instr = ARM11->Instr.Data;
    const u8 condcode = instr >> 28;
	
	// Todo: handle IRQs
	if (CondLookup[condcode] & (1<<ARM11->Flags))
	{
    	const u16 decodebits = ((instr >> 16) & 0xFF0) | ((instr >> 4) & 0xF);

		(ARM11_InstrLUT[decodebits])(ARM11);
	}
	else if (condcode == COND_NV) // do special handling for unconditional instructions
	{
		DecodeUncondInstr(instr);
	}
	else if (false) // TODO: handle illegal BKPTs? might still execute regardless of condition fail like the ARM9?
	{

	}
	else
	{
		// instruction was not executed.
	}
}

void ARM11_RunInterpreter(struct ARM11MPCore* ARM11)
{
	while (true)
	{
        (ARM11->NextStep)(ARM11);
	}
}
