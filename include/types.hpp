#ifndef FORGE_TYPES_HPP
#define FORGE_TYPES_HPP

namespace forge {

using u64 = unsigned long;
using u32 = unsigned int;
using u16 = unsigned short;
using u8  = unsigned char;

using i64 = long;
using i32 = int;
using i16 = short;
using i8  = signed char;

using f64 = double;
using f32 = float;

static_assert(sizeof(u64) == 8, "Forge requires unsigned long to be 64 bits");
static_assert(sizeof(u32) == 4, "Forge requires unsigned int to be 32 bits");
static_assert(sizeof(u16) == 2, "Forge requires unsigned short to be 16 bits");
static_assert(sizeof(u8) == 1, "Forge requires unsigned char to be 8 bits");
static_assert(sizeof(i64) == 8, "Forge requires long to be 64 bits");
static_assert(sizeof(i32) == 4, "Forge requires int to be 32 bits");
static_assert(sizeof(i16) == 2, "Forge requires short to be 16 bits");
static_assert(sizeof(i8) == 1, "Forge requires signed char to be 8 bits");
static_assert(sizeof(f64) == 8, "Forge requires double to be 64 bits");
static_assert(sizeof(f32) == 4, "Forge requires float to be 32 bits");

enum FloatType : u8 {
        POS_FINITE = 0, // signed = 0, special = 0, is_nan = 0
        POS_INF    = 2, // signed = 0, special = 1, is_nan = 0
        POS_NAN    = 3, // signed = 0, special = 1, is_nan = 1
        NEG_FINITE = 4, // signed = 1, special = 0, is_nan = 0
        NEG_INF    = 6, // signed = 1, special = 1, is_nan = 0
        NEG_NAN    = 7, // signed = 1, special = 1, is_nan = 1
};
static_assert(sizeof(FloatType) == sizeof(u8), "Forge requires FloatType to be 8 bits");

} // namespace forge

#endif // ANVIL_TYPES_HPP