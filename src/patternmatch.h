#pragma once

#include "types.h"

struct Pattern
{
    u32 bits;
    u32 mask;
};

inline bool PatternMatch(const struct Pattern pattern, const u32 bits)
{
    return ((bits & pattern.mask) == pattern.bits);
}
