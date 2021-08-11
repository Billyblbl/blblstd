#ifndef G_BASETYPES
# define G_BASETYPES

#include <stdint.h>
#include <cstddef>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

//might not be guaranted language wise
using f32 = float;
using f64 = double;

using USize = size_t;
using SSize = ptrdiff_t;

//Byte as defined by the platform, unsigned char is always supposed to be
//smallest size possible on a platform AFAIK
using Byte = unsigned char;

using cstring = char*;
using rcstring = const char*;
using lstring = char[];

using Any = void;

constexpr auto null = nullptr;

// System guard
static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);
static_assert(sizeof(s8) == 1);
static_assert(sizeof(s16) == 2);
static_assert(sizeof(s32) == 4);
static_assert(sizeof(s64) == 8);

#endif