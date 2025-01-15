#pragma once

#include "../../utils.h"

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

union PageTableEntry
{
	u32 Data;

	struct
	{
		u32 Type : 2;
		u32 : 3;
		u32 Domain : 4;
		bool ImplementationDefined : 1;
		u32 LookupAddr : 22;
	} Coarse;

	struct
	{
		u32 Type : 2;
		bool B : 1;
		bool C : 1;
		bool XN : 1;
		u32 Domain : 4;
		bool ImplementationDefined : 1;
		u32 AP : 2;
		u32 TEX : 3;
		bool APX : 1;
		bool S : 1;
		bool NG : 1;
		bool Supersection : 1;
		bool : 1;
		u32 BaseAddr : 12;
	} Section;

	struct
	{
		u32 Type : 2;
		bool B : 1;
		bool C : 1;
		bool XN : 1;
		u32 ExtAddr2 : 4;
		bool ImplementationDefined : 1;
		u32 AP : 2;
		u32 TEX : 3;
		bool APX : 1;
		bool S : 1;
		bool NG : 1;
		bool Supersection : 1;
		bool : 1;
		u32 ExtAddr1 : 4;
		u32 BaseAddr : 8;
	} Supersection;

	union
	{
		struct
		{
			u32 Type : 2;
			bool B : 1;
			bool C : 1;
			u32 AP0 : 2;
			u32 AP1 : 2;
			u32 AP2 : 2;
			u32 AP3 : 2;
			u32 TEX : 3;
			bool : 1;
			u32 BaseAddr : 16;
		} LargePage;

		struct
		{
			u32 Type : 2;
			bool B : 1;
			bool C : 1;
			u32 AP0 : 2;
			u32 AP1 : 2;
			u32 AP2 : 2;
			u32 AP3 : 2;
			u32 BaseAddr : 20;
		} SmallPage;

		struct
		{
			u32 Type : 2;
			bool B : 1;
			bool C : 1;
			u32 AP : 2;
			u32 TEX : 3;
			u32 : 3;
			u32 BaseAddr : 20;
		} ExtSmallPage;
	} L2YSubpage;

	union
	{
		struct
		{
			u32 Type : 2;
			bool B : 1;
			bool C : 1;
			u32 AP : 2;
			u32 : 3;
			bool APX : 1;
			bool S : 1;
			bool NG : 1;
			u32 TEX : 3;
			bool XN : 1;
			u32 BaseAddr : 16;
		} LargePage;

		struct
		{
			bool XN : 1;
			bool Type : 1;
			bool B : 1;
			bool C : 1;
			u32 AP : 2;
			u32 TEX : 3;
			bool APX : 1;
			bool S : 1;
			bool NG : 1;
			u32 BaseAddr : 20;
		} ExtSmallPage;

	} L2NSubpage;
};

enum
{
	TLB_Read  = 0b01,
	TLB_Instr = 0b10,
};

extern const u16 CondLookup[16];

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
			bool FIQDisable : 1;
			bool IRQDisable : 1;
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
	u8 CPUID;
	union // USR
	{
		u32 R[7];
		struct
		{
			u32 R8;
			u32 R9;
			u32 R10;
			u32 R11;
			u32 R12;
			u32 SP;
			u32 LR;
		};
	} USR;
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
		union
		{
			u32 TransTblBaseR0;
			struct
			{
				bool : 1;
				bool Shared : 1;
				bool ECC : 1;
				bool RGN : 1;
				u32 : 9;
				u32 : 18;
			} TTB_R0;
		};
		union
		{
			u32 TransTblBaseR1;
			struct
			{
				bool : 1;
				bool Shared : 1;
				bool : 1;
				bool RGN : 1;
				u32 : 9;
				u32 : 18;
			} TTB_R1;
		};
		union
		{
			u8 TransTblControl;
			u8 N : 3;
		};
		u32 DomainAccessControl;
		union
		{
			u16 DataFaultStatus;
			struct
			{
				u16 Type1 : 4;
				u16 Domain : 4;
				u16 : 2;
				u16 Type2 : 1;
				bool ReadOrWrite : 1;
				bool ExtAbtQual : 1;
			} DataFault;
		};
		union
		{
			u16 InstrFaultStatus;
			struct
			{
				u16 Type1 : 4;
				u16 Domain : 4;
				u16 : 2;
				u16 Type2 : 1;
				bool : 1;
				bool ExtAbtQual : 1;
			} InstrFault;
		};
		u32 DataFaultAddress;
		u32 WatchpointFaultAddress;
		u32 DCacheLockdown;
		u32 TLBLockdown;
		u32 PrimaryMemRgnRemap;
		u32 NormalMemRgnRemap;
		u32 FCSEPID;
		u32 ContextID;
		u32 ThreadID[3];
		u32 PerfMonitorControl;
	} CP15;
	union
	{
		u32 TimerControl;
		struct
		{
			bool Enable;
			bool AutoReload;
			bool IntEnable;
			u32 : 5;
			u32 Prescaler : 8;
		} Timer;
		u8 TimerIRQStat;
	};
};

extern void (*ARM11_InstrLUT[0x1000]) (struct ARM11MPCore* ARM11);
extern void (*THUMB11_InstrLUT[0x40]) (struct ARM11MPCore* ARM11);

extern struct ARM11MPCore ARM11[4];

void* ARM11_InitARMInstrLUT(const u16 bits);
void* ARM11_InitTHUMBInstrLUT(const u8 bits);

u32 ARM11_ROR32(u32 val, u8 ror);

char* ARM11_Init();
void ARM11_UpdateMode(struct ARM11MPCore* ARM11, u8 oldmode, u8 newmode);
void ARM11_Branch(struct ARM11MPCore* ARM11, const u32 addr, const bool restore);
u32 ARM11_GetReg(struct ARM11MPCore* ARM11, const int reg);
void ARM11_WriteReg(struct ARM11MPCore* ARM11, const int reg, const u32 val, const bool restore, const bool canswap);
void ARM11_StartFetch(struct ARM11MPCore* ARM11);
void ARM11_StartExec(struct ARM11MPCore* ARM11);
void THUMB11_StartExec(struct ARM11MPCore* ARM11);
void ARM11_RunInterpreter(struct ARM11MPCore* ARM11, u64 target);

bool ARM11_CP15_PageTable_Lookup(struct ARM11MPCore* ARM11, u32* addr, const u8 accesstype);


// instr implementations
void ARM11_ALU(struct ARM11MPCore* ARM11);
void THUMB11_ShiftImm(struct ARM11MPCore* ARM11);
void THUMB11_ADD_SUB_Reg_Imm3(struct ARM11MPCore* ARM11);
void THUMB11_ADD_SUB_CMP_MOV_Imm8(struct ARM11MPCore* ARM11);
void THUMB11_ALU(struct ARM11MPCore* ARM11);
void THUMB11_ALU_HI(struct ARM11MPCore* ARM11);
void THUMB11_ADD_SP_PCRel(struct ARM11MPCore* ARM11);
void THUMB11_ADD_SUB_SP(struct ARM11MPCore* ARM11);

void ARM11_LDR_STR(struct ARM11MPCore* ARM11);
void ARM11_LDM_STM(struct ARM11MPCore* ARM11);
void THUMB11_LDRPCRel(struct ARM11MPCore* ARM11);
void THUMB11_LDR_STR_SPRel(struct ARM11MPCore* ARM11);
void THUMB11_LDR_STR_Reg(struct ARM11MPCore* ARM11);
void THUMB11_LDR_STR_Imm5(struct ARM11MPCore* ARM11);
void THUMB11_LDRH_STRH_Imm5(struct ARM11MPCore* ARM11);
void THUMB11_PUSH(struct ARM11MPCore* ARM11);
void THUMB11_POP(struct ARM11MPCore* ARM11);

void ARM11_B_BL(struct ARM11MPCore* ARM11);
void ARM11_BX_BLXReg(struct ARM11MPCore* ARM11);
void THUMB11_CondB_SWI(struct ARM11MPCore* ARM11);
void THUMB11_B(struct ARM11MPCore* ARM11);
void THUMB11_BL_BLX_LO(struct ARM11MPCore* ARM11);
void THUMB11_BL_BLX_HI(struct ARM11MPCore* ARM11);
void THUMB11_BLX_BX_Reg(struct ARM11MPCore* ARM11, const u32 addr);

void ARM11_MCR_MRC(struct ARM11MPCore* ARM11);
void ARM11_CP15_Store_Single(struct ARM11MPCore* ARM11, u16 cmd, u32 val);
u32 ARM11_CP15_Load_Single(struct ARM11MPCore* ARM11, u16 cmd);

void ARM11_MRS(struct ARM11MPCore* ARM11);

void ARM11_MSRReg(struct ARM11MPCore* ARM11);
void ARM11_MSRImm_Hints(struct ARM11MPCore* ARM11);
