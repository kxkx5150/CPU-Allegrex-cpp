#pragma once

#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <utility>

#include "Log.h"

#include "CommonTypes.h"

#define IS_LITTLE_ENDIAN (*(const u16 *)"\0\xff" >= 0x100)
#define IS_BIG_ENDIAN    (*(const u16 *)"\0\xff" < 0x100)

inline u8 Convert4To8(u8 v)
{
    return (v << 4) | (v);
}

inline u8 Convert5To8(u8 v)
{
    return (v << 3) | (v >> 2);
}

inline u8 Convert6To8(u8 v)
{
    return (v << 2) | (v >> 4);
}

static inline u8 clamp_u8(int i)
{
#ifdef ARM
    asm("usat %0, #8, %1" : "=r"(i) : "r"(i));
#else
    if (i > 255)
        return 255;
    if (i < 0)
        return 0;
#endif
    return i;
}

static inline s16 clamp_s16(int i)
{
#ifdef ARM
    asm("ssat %0, #16, %1" : "=r"(i) : "r"(i));
#else
    if (i > 32767)
        return 32767;
    if (i < -32768)
        return -32768;
#endif
    return i;
}

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(t)                                                                                    \
  private:                                                                                                             \
    t(const t &other);                                                                                                 \
    void operator=(const t &other);
#endif
