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

enum : u8
{
    VECTOR_RST = 0x00,
    VECTOR_UND = 0x04,
    VECTOR_SVC = 0x08,
    VECTOR_PAB = 0x0C,
    VECTOR_DAB = 0x10,

    VECTOR_IRQ = 0x18,
    VECTOR_FIQ = 0x1C,
};

struct alignas(u64) Instruction
{
    u32 Data;
    bool PrefetchAbort;
};

extern const u16 CondLookup[16];
