/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphiteTypes_DEFINED
#define skgpu_GraphiteTypes_DEFINED

#include "include/core/SkTypes.h"

namespace skgpu {

/**
 * Wraps a C++11 enum that we use as flags, and enables masking with type safety.
 */
template<typename E>
class FlagsEnum {
public:
    SK_ALWAYS_INLINE constexpr explicit FlagsEnum(E value) : FlagsEnum((int)value) {}
    SK_ALWAYS_INLINE constexpr explicit FlagsEnum(int value) : fValue(value) {}

    SK_ALWAYS_INLINE constexpr operator E() const { return (E)fValue; }
    SK_ALWAYS_INLINE constexpr operator bool() const { return fValue; }

    SK_ALWAYS_INLINE constexpr FlagsEnum operator~() { return FlagsEnum(~fValue); }

    SK_ALWAYS_INLINE constexpr FlagsEnum operator|(FlagsEnum flags) {
        return FlagsEnum(fValue | flags.fValue);
    }

    SK_ALWAYS_INLINE constexpr FlagsEnum operator&(FlagsEnum flags) {
        return FlagsEnum(fValue & flags.fValue);
    }

    SK_ALWAYS_INLINE constexpr FlagsEnum operator^(FlagsEnum flags) {
        return FlagsEnum(fValue ^ flags.fValue);
    }

    SK_ALWAYS_INLINE FlagsEnum& operator|=(E flag) { return (*this = *this | FlagsEnum(flag)); }
    SK_ALWAYS_INLINE FlagsEnum& operator&=(E flag) { return (*this = *this & FlagsEnum(flag)); }
    SK_ALWAYS_INLINE FlagsEnum& operator^=(E flag) { return (*this = *this & FlagsEnum(flag)); }

    SK_ALWAYS_INLINE FlagsEnum& operator|=(FlagsEnum flags) { return (*this = *this | flags); }
    SK_ALWAYS_INLINE FlagsEnum& operator&=(FlagsEnum flags) { return (*this = *this & flags); }
    SK_ALWAYS_INLINE FlagsEnum& operator^=(FlagsEnum flags) { return (*this = *this & flags); }

private:
    int fValue;
};

template<typename E>
FlagsEnum<E> SK_ALWAYS_INLINE constexpr operator|(E a, FlagsEnum<E> b) {
    return FlagsEnum<E>(a) | b;
}

template<typename E>
FlagsEnum<E> SK_ALWAYS_INLINE constexpr operator|(FlagsEnum<E> b, E a) {
    return a | FlagsEnum<E>(b);
}

template<typename E>
FlagsEnum<E> SK_ALWAYS_INLINE constexpr operator&(E a, FlagsEnum<E> b) {
    return FlagsEnum<E>(a) & b;
}

template<typename E>
FlagsEnum<E> SK_ALWAYS_INLINE constexpr operator&(FlagsEnum<E> b, E a) {
    return a & FlagsEnum<E>(b);
}

template<typename E>
FlagsEnum<E> SK_ALWAYS_INLINE constexpr operator^(E a, FlagsEnum<E> b) {
    return FlagsEnum<E>(a) ^ b;
}

template<typename E>
FlagsEnum<E> SK_ALWAYS_INLINE constexpr operator^(FlagsEnum<E> b, E a) {
    return a ^ FlagsEnum<E>(b);
}

/**
 * Defines functions that make it possible to use bitwise operators on an enum class.
 */
#define SKGPU_MAKE_FLAGS_ENUM_OPS(E) \
    SK_MAYBE_UNUSED constexpr FlagsEnum<E> operator~(E flags) { return ~FlagsEnum<E>(flags); } \
    SK_MAYBE_UNUSED constexpr FlagsEnum<E> operator|(E a, E b) { return FlagsEnum<E>(a) | b; } \
    SK_MAYBE_UNUSED constexpr FlagsEnum<E> operator&(E a, E b) { return FlagsEnum<E>(a) & b; } \
    SK_MAYBE_UNUSED constexpr FlagsEnum<E> operator^(E a, E b) { return FlagsEnum<E>(a) ^ b; } \
    SK_MAYBE_UNUSED inline E& operator|=(E& a, E b) { return (a = a | b); } \
    SK_MAYBE_UNUSED inline E& operator&=(E& a, E b) { return (a = a & b); } \
    SK_MAYBE_UNUSED inline E& operator^=(E& a, E b) { return (a = a ^ b); } \

#define GR_DECL_FLAGS_ENUM_OPS_FRIENDS(E) \
    friend constexpr FlagsEnum<E> operator~(E); \
    friend constexpr FlagsEnum<E> operator|(E, E); \
    friend constexpr FlagsEnum<E> operator&(E, E); \
    friend constexpr FlagsEnum<E> operator^(E, E); \
    friend E& operator|=(E&, E); \
    friend E& operator&=(E&, E); \
    friend E& operator^=(E&, E); \

/**
 * Possible 3D APIs that may be used by Graphite.
 */
enum class BackendApi : unsigned {
    kMetal,
    kMock,
};

/**
 * Is the texture mipmapped or not
 */
enum class Mipmapped: bool {
    kNo = false,
    kYes = true,
};

/**
 * Is the data protected on the GPU or not.
 */
enum class Protected : bool {
    kNo = false,
    kYes = true,
};

/**
 * An ordinal number that allows draw commands to be re-ordered so long as when they are executed,
 * the read/writes to the color|depth|stencil attachments respect the original painter's order.
 */
using CompressedPaintersOrder = uint16_t;

} // namespace skgpu

#endif // skgpu_GraphiteTypes_DEFINED
