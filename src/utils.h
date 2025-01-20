#pragma once

#include <stdint.h>
#include <stddef.h>

typedef int8_t s8;
typedef uint8_t u8;

typedef int16_t s16;
typedef uint16_t u16;

typedef int32_t s32;
typedef uint32_t u32;

typedef int64_t s64;
typedef uint64_t u64;


// todo: actually add fallback paths if these dont exist for w/e-
#define likely(x) __builtin_expect(!!x, 1)
#define unlikely(x) __builtin_expect(!!x, 0)
#define bswap(x) _Generic((x), u16: __builtin_bswap16(x), u32: __builtin_bswap32(x))

enum
{
    ARM9ID = 0,
    ARM11ID = 1
};

struct Pattern
{
    u32 bits;
    u32 mask;
};

inline bool PatternMatch(const struct Pattern pattern, const u32 bits)
{
    return ((bits & pattern.mask) == pattern.bits);
}
