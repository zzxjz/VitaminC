#pragma once

#include "../../types.h"

enum : u8
{
	COND_EQ,
	COND_NE,
	COND_CS,
	COND_CC,
	COND_MI,
	COND_PL,
	COND_VS,
	COND_VC,
	COND_HI,
	COND_LS,
	COND_GE,
	COND_LT,
	COND_GT,
	COND_LE,
	COND_AL,
	COND_NV,
};

struct alignas(u64) Instruction
{
	u32 Data;
	bool PrefetchAbort;
};

struct ARM11MPCore
{
	// registers
	union alignas(64) 
	{
		u32 R[16];
		struct
		{
			u32 R0;
			u32 R1;
			u32 R2;
			u32 R3;
			u32 R4;
			u32 R5;
			u32 R6;
			u32 R7;
			u32 R8;
			u32 R9;
			u32 R10;
			u32 R11;
			u32 R12;
			u32 SP;
			u32 LR;
			u32 PC;
		};
	};
	union
	{
		u32 CPSR;
		struct
		{
			u32 Mode : 5;
			bool Thumb : 1;
			bool FIQEnable : 1;
			bool IRQEnable : 1;
			bool ImpreciseAbortEnable : 1;
			bool DataBigEndian : 1;
			u32 : 6;
			u32 GE : 4;
			u32 : 4;
			bool Jazelle : 1;
			u32 : 2;
			bool QSticky : 1;
			bool Overflow : 1;
			bool Carry : 1;
			bool Zero : 1;
			bool Negative : 1;
		};
	};
	u32* SPSR;
	union
	{
		u32 R_SVC[3];
		struct
		{
			u32 SP_SVC;
			u32 LR_SVC;
			u32 SPSR_SVC;
		};
	};
	union
	{
		u32 R_ABT[3];
		struct
		{
			u32 SP_ABT;
			u32 LR_ABT;
			u32 SPSR_ABT;
		};
	};
	union
	{
		u32 R_UND[3];
		struct
		{
			u32 SP_UND;
			u32 LR_UND;
			u32 SPSR_UND;
		};
	};
	union
	{
		u32 R_IRQ[3];
		struct
		{
			u32 SP_IRQ;
			u32 LR_IRQ;
			u32 SPSR_IRQ;
		};
	};
	union
	{
		u32 R_FIQ[8];
		struct
		{
			u32 R8_FIQ;
			u32 R9_FIQ;
			u32 R10_FIQ;
			u32 R11_FIQ;
			u32 R12_FIQ;
			u32 SP_FIQ;
			u32 LR_FIQ;
			u32 SPSR_FIQ;
		};
	};
	struct Instruction Instr;
	void (*NextStep)(struct ARM11MPCore* ARM11);
};

extern void (*ARM11_InstrLUT[0x1000]) (struct ARM11MPCore* ARM11);

extern struct ARM11MPCore ARM11[4];

void* ARM11_InitInstrLUT(const u16 bits);

u32 ARM11_ROR32(u32 val, u8 ror);

char* ARM11_Init();
void ARM11_Branch(struct ARM11MPCore* ARM11, const u32 addr);
u32 ARM11_GetReg(struct ARM11MPCore* ARM11, const int reg);
void ARM11_WriteReg(struct ARM11MPCore* ARM11, const int reg, const u32 val, const bool restore);
void ARM11_StartFetch(struct ARM11MPCore* ARM11);
void ARM11_StartExec(struct ARM11MPCore* ARM11);
void ARM11_RunInterpreter(struct ARM11MPCore* ARM11);
