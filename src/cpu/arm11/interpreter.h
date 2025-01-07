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

enum : u8
{
	MODE_USR = 0x10,
	MODE_FIQ = 0x11,
	MODE_IRQ = 0x12,
	MODE_SVC = 0x13,
	MODE_ABT = 0x17,
	MODE_UND = 0x1B,
	MODE_SYS = 0x1F,
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
		struct
		{
			u32 : 28;
			u32 Flags : 4;
		};
	};
	u32* SPSR;
	union // SVC
	{
		u32 R[3];
		struct
		{
			u32 SP;
			u32 LR;
			u32 SPSR;
		};
	} SVC;
	union // ABT
	{
		u32 R[3];
		struct
		{
			u32 SP;
			u32 LR;
			u32 SPSR;
		};
	} ABT;
	union // UND
	{
		u32 R[3];
		struct
		{
			u32 SP;
			u32 LR;
			u32 SPSR;
		};
	} UND;
	union // IRQ
	{
		u32 R[3];
		struct
		{
			u32 SP;
			u32 LR;
			u32 SPSR;
		};
	} IRQ;
	union // FIQ
	{
		u32 R[8];
		struct
		{
			u32 R8;
			u32 R9;
			u32 R10;
			u32 R11;
			u32 R12;
			u32 SP;
			u32 LR;
			u32 SPSR;
		};
	} FIQ;
	struct Instruction Instr;
	u64 Timestamp;
	void (*NextStep)(struct ARM11MPCore* ARM11);
	struct
	{
		union
		{
			u32 Control;
			struct
			{
				bool MMU : 1;
				bool AlignmentFault : 1;
				bool L1DCache : 1;
				u32 : 5;
				bool SysProt : 1;
				bool ROMProt : 1;
				bool : 1;
				bool ProgramFlowPred : 1;
				bool L1ICache : 1;
				bool HighVector : 1;
				bool : 1;
				bool ARMv4Thingy : 1;
				u32 : 6;
				bool ExtPageTable : 1;
				bool : 1;
				bool ExceptionEndian : 1;
				bool : 1;
				bool NMFIwhat : 1;
				bool TEXRemap : 1;
				bool ForceAP : 1;
				u32 : 2;
			};
		};
		union
		{
			u8 AuxControl;
			struct
			{
				bool ReturnStack : 1;
				bool DynamicBranchPred : 1;
				bool StaticBranchPred : 1;
				bool InstrFolding : 1;
				bool ExclusiveL1L2 : 1;
				bool CoherencyMode : 1;
				bool L1ParityCheck : 1;
			};
		};
		union
		{
			u32 CoprocAccessControl;
			struct
			{
				u32 : 20;
				u32 CP10Access : 2;
				u32 CP11Access : 2;
				u32 : 8;
			};
		};
	} CP15;
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
void ARM11_RunInterpreter(struct ARM11MPCore* ARM11, u64 target);

void ARM11_ALU(struct ARM11MPCore* ARM11);

void ARM11_LDR_STR(struct ARM11MPCore* ARM11);
void ARM11_LDM_STM(struct ARM11MPCore* ARM11);

void ARM11_B_BL(struct ARM11MPCore* ARM11);

void ARM11_MCR(struct ARM11MPCore* ARM11);
void ARM11_CP15_Store(struct ARM11MPCore* ARM11, u16 cmd, u32 val);
