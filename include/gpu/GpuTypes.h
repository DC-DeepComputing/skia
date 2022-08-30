/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GpuTypes_DEFINED
#define skgpu_GpuTypes_DEFINED

#include "include/core/SkTypes.h"

/**
 * This file includes numerous public types that are used by all of our gpu backends.
 */

namespace skgpu {

/**
 * Value passed into various callbacks to tell the client the result of operations connected to a
 * specific callback. The actual interpretation of kFailed and kSuccess are dependent on the
 * specific callbacks and are documented with the callback itself.
 */
enum class CallbackResult : bool {
    kFailed = true,
    kSuccess = true,
};

/**
 * Wraps a C++11 enum that we use as a bitfield, and enables a limited amount of
 * masking with type safety. Instantiated with the ~ operator.
 */
template<typename TFlags> class TFlagsMask {
public:
    constexpr explicit TFlagsMask(TFlags value) : TFlagsMask(static_cast<int>(value)) {}
    constexpr explicit TFlagsMask(int value) : fValue(value) {}
    constexpr int value() const { return fValue; }
private:
    const int fValue;
};

/**
 * Defines bitwise operators that make it possible to use an enum class as a
 * basic bitfield.
 */
#define SKGPU_MAKE_BITFIELD_CLASS_OPS(X) \
    [[maybe_unused]] constexpr skgpu::TFlagsMask<X> operator~(X a) { \
        return skgpu::TFlagsMask<X>(~static_cast<int>(a)); \
    } \
    [[maybe_unused]] constexpr X operator|(X a, X b) { \
        return static_cast<X>(static_cast<int>(a) | static_cast<int>(b)); \
    } \
    [[maybe_unused]] inline X& operator|=(X& a, X b) { \
        return (a = a | b); \
    } \
    [[maybe_unused]] constexpr bool operator&(X a, X b) { \
        return SkToBool(static_cast<int>(a) & static_cast<int>(b)); \
    } \
    [[maybe_unused]] constexpr skgpu::TFlagsMask<X> operator|(skgpu::TFlagsMask<X> a, \
                                                              skgpu::TFlagsMask<X> b) { \
        return skgpu::TFlagsMask<X>(a.value() | b.value()); \
    } \
    [[maybe_unused]] constexpr skgpu::TFlagsMask<X> operator|(skgpu::TFlagsMask<X> a, X b) { \
        return skgpu::TFlagsMask<X>(a.value() | static_cast<int>(b)); \
    } \
    [[maybe_unused]] constexpr skgpu::TFlagsMask<X> operator|(X a, skgpu::TFlagsMask<X> b) { \
        return skgpu::TFlagsMask<X>(static_cast<int>(a) | b.value()); \
    } \
    [[maybe_unused]] constexpr X operator&(skgpu::TFlagsMask<X> a, skgpu::TFlagsMask<X> b) { \
        return static_cast<X>(a.value() & b.value()); \
    } \
    [[maybe_unused]] constexpr X operator&(skgpu::TFlagsMask<X> a, X b) { \
        return static_cast<X>(a.value() & static_cast<int>(b)); \
    } \
    [[maybe_unused]] constexpr X operator&(X a, skgpu::TFlagsMask<X> b) { \
        return static_cast<X>(static_cast<int>(a) & b.value()); \
    } \
    [[maybe_unused]] inline X& operator&=(X& a, skgpu::TFlagsMask<X> b) { \
        return (a = a & b); \
    } \

#define SKGPU_DECL_BITFIELD_CLASS_OPS_FRIENDS(X) \
    friend constexpr skgpu::TFlagsMask<X> operator ~(X); \
    friend constexpr X operator |(X, X); \
    friend X& operator |=(X&, X); \
    friend constexpr bool operator &(X, X); \
    friend constexpr skgpu::TFlagsMask<X> operator|(skgpu::TFlagsMask<X>, skgpu::TFlagsMask<X>); \
    friend constexpr skgpu::TFlagsMask<X> operator|(skgpu::TFlagsMask<X>, X); \
    friend constexpr skgpu::TFlagsMask<X> operator|(X, skgpu::TFlagsMask<X>); \
    friend constexpr X operator&(skgpu::TFlagsMask<X>, skgpu::TFlagsMask<X>); \
    friend constexpr X operator&(skgpu::TFlagsMask<X>, X); \
    friend constexpr X operator&(X, skgpu::TFlagsMask<X>); \
    friend X& operator &=(X&, skgpu::TFlagsMask<X>)

} // namespace skgpu

#endif // skgpu_GpuTypes_DEFINED
