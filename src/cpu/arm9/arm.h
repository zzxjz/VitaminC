#pragma once

#include "../../utils.h"
#include "../shared/arm.h"


struct ARM946E_S
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
			u32 : 19;
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
	struct Instruction Instr[3];
	u64 Timestamp;
	void (*NextStep)(struct ARM946E_S* ARM9);
	struct
	{
		union
		{
			u32 Data;
			struct
			{
				bool MPU : 1;
				bool : 1;
				bool DCache : 1;
				u32 : 4;
				bool BigEndian : 1;
				u32 : 4;
				bool ICache : 1;
				bool HighVector : 1;
				bool RoundRobin : 1;
				bool TBitLoadDisable : 1;
				bool DTCM : 1;
				bool DTCMWriteOnly : 1;
				bool ITCM : 1;
				bool ITCMWriteOnly : 1;
			};
		} Control;
		u8 ICacheConfig;
		u8 DCacheConfig;
		u8 WBufferControl;
		u32 MPUDataPerms;
		u32 MPUInstrPerms;
		union
		{
			u32 Data;
			struct
			{
				bool Enable : 1;
				u32 Size : 5;
				u32 : 6;
				u32 Base : 20;
			};
		} MPURegion[8];
		union
		{
			u32 Data;
			struct
			{
				u32 Index : 2;
				u32 : 29;
				bool Load : 1;
			};
		} DCacheLockDown;
		union
		{
			u32 Data;
			struct
			{
				u32 Index : 2;
				u32 : 29;
				bool Load : 1;
			};
		} ICacheLockDown;
		union
		{
			u32 Data;
			struct
			{
				bool : 1;
				u32 Size : 5;
				u32 : 6;
				u32 Base : 20;
			};
		} DTCMRegion;
		union
		{
			u32 Data;
			struct
			{
				bool : 1;
				u32 Size : 5;
				u32 : 6;
				u32 Base : 20;
			};
		} ITCMRegion;
		u32 ProcessID;
	} CP15;
	u32 ITCMMask;
	u32 DTCMMask;
	u32 DTCMBase;
};

extern void (*ARM9_InstrLUT[0x1000]) (struct ARM946E_S* ARM9);
extern void (*THUMB9_InstrLUT[0x40]) (struct ARM946E_S* ARM9);

extern struct ARM946E_S _ARM9;

void* ARM9_InitARMInstrLUT(const u16 bits);
void* ARM9_InitTHUMBInstrLUT(const u8 bits);

u32 ARM9_ROR32(u32 val, u8 ror);

char* ARM9_Init();
void ARM9_UpdateMode(struct ARM946E_S* ARM9, u8 oldmode, u8 newmode);
void ARM9_Branch(struct ARM946E_S* ARM9, const u32 addr, const bool restore);
u32 ARM9_GetReg(struct ARM946E_S* ARM9, const int reg);
void ARM9_WriteReg(struct ARM946E_S* ARM9, const int reg, const u32 val, const bool restore, const bool canswap);
void ARM9_StartFetch(struct ARM946E_S* ARM9);
void ARM9_StartExec(struct ARM946E_S* ARM9);
void THUMB9_StartExec(struct ARM946E_S* ARM9);
void ARM9_RunInterpreter(struct ARM946E_S* ARM9, u64 target);


// instr implementations
void ARM9_ALU(struct ARM946E_S* ARM9);
void THUMB9_ShiftImm(struct ARM946E_S* ARM9);
void THUMB9_ADD_SUB_Reg_Imm3(struct ARM946E_S* ARM9);
void THUMB9_ADD_SUB_CMP_MOV_Imm8(struct ARM946E_S* ARM9);
void THUMB9_ALU(struct ARM946E_S* ARM9);
void THUMB9_ALU_HI(struct ARM946E_S* ARM9);
void THUMB9_ADD_SP_PCRel(struct ARM946E_S* ARM9);
void THUMB9_ADD_SUB_SP(struct ARM946E_S* ARM9);

void ARM9_MUL_MLA(struct ARM946E_S* ARM9);
void THUMB9_MULS(struct ARM946E_S* ARM9, const int rd, const u32 rdval, const u32 rmval);

void ARM9_LDR_STR(struct ARM946E_S* ARM9);
void ARM9_LDM_STM(struct ARM946E_S* ARM9);
void ARM9_PLD(struct ARM946E_S* ARM9);
void THUMB9_LDRPCRel(struct ARM946E_S* ARM9);
void THUMB9_LDR_STR_SPRel(struct ARM946E_S* ARM9);
void THUMB9_LDR_STR_Reg(struct ARM946E_S* ARM9);
void THUMB9_LDR_STR_Imm5(struct ARM946E_S* ARM9);
void THUMB9_LDRH_STRH_Imm5(struct ARM946E_S* ARM9);
void THUMB9_LDMIA_STMIA(struct ARM946E_S* ARM9);
void THUMB9_PUSH(struct ARM946E_S* ARM9);
void THUMB9_POP(struct ARM946E_S* ARM9);

void ARM9_B_BL(struct ARM946E_S* ARM9);
void ARM9_BX_BLXReg(struct ARM946E_S* ARM9);
void ARM9_BLX_Imm(struct ARM946E_S* ARM9);
void THUMB9_CondB_SWI(struct ARM946E_S* ARM9);
void THUMB9_B(struct ARM946E_S* ARM9);
void THUMB9_BL_BLX_LO(struct ARM946E_S* ARM9);
void THUMB9_BL_BLX_HI(struct ARM946E_S* ARM9);
void THUMB9_BLX_BX_Reg(struct ARM946E_S* ARM9, const u32 addr);

void ARM9_MCR_MRC(struct ARM946E_S* ARM9);
void ARM9_CP15_Store_Single(struct ARM946E_S* ARM9, u16 cmd, u32 val);
u32 ARM9_CP15_Load_Single(struct ARM946E_S* ARM9, u16 cmd);

void ARM9_MRS(struct ARM946E_S* ARM9);

void ARM9_MSRReg(struct ARM946E_S* ARM9);
void ARM9_MSRImm(struct ARM946E_S* ARM9);
