/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphiteTypes_DEFINED
#define skgpu_GraphiteTypes_DEFINED

#include "include/core/SkMath.h"
#include "include/core/SkTypes.h"

class SkImage;
class SkSurface;

namespace skgpu {

class BackendSemaphore;

////////////////////////////////////////////////////////////////////////////////

/**
 * Defines overloaded bitwise operators to make it easier to use an enum as a
 * bitfield.
 */
#define MAKE_BITFIELD_OPS(X) \
    inline X operator |(X a, X b) { \
        return (X) (+a | +b); \
    } \
    inline X& operator |=(X& a, X b) { \
        return (a = a | b); \
    } \
    inline X operator &(X a, X b) { \
        return (X) (+a & +b); \
    } \
    inline X& operator &=(X& a, X b) { \
        return (a = a & b); \
    } \
    template <typename T> \
    inline X operator &(T a, X b) { \
        return (X) (+a & +b); \
    } \
    template <typename T> \
    inline X operator &(X a, T b) { \
        return (X) (+a & +b); \
    } \

#define DECL_BITFIELD_OPS_FRIENDS(X) \
    friend X operator |(X a, X b); \
    friend X& operator |=(X& a, X b); \
    \
    friend X operator &(X a, X b); \
    friend X& operator &=(X& a, X b); \
    \
    template <typename T> \
    friend X operator &(T a, X b); \
    \
    template <typename T> \
    friend X operator &(X a, T b); \

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
#define MAKE_BITFIELD_CLASS_OPS(X) \
    SK_MAYBE_UNUSED constexpr TFlagsMask<X> operator~(X a) { \
        return TFlagsMask<X>(~static_cast<int>(a)); \
    } \
    SK_MAYBE_UNUSED constexpr X operator|(X a, X b) { \
        return static_cast<X>(static_cast<int>(a) | static_cast<int>(b)); \
    } \
    SK_MAYBE_UNUSED inline X& operator|=(X& a, X b) { \
        return (a = a | b); \
    } \
    SK_MAYBE_UNUSED constexpr bool operator&(X a, X b) { \
        return SkToBool(static_cast<int>(a) & static_cast<int>(b)); \
    } \
    SK_MAYBE_UNUSED constexpr TFlagsMask<X> operator|(TFlagsMask<X> a, TFlagsMask<X> b) { \
        return TFlagsMask<X>(a.value() | b.value()); \
    } \
    SK_MAYBE_UNUSED constexpr TFlagsMask<X> operator|(TFlagsMask<X> a, X b) { \
        return TFlagsMask<X>(a.value() | static_cast<int>(b)); \
    } \
    SK_MAYBE_UNUSED constexpr TFlagsMask<X> operator|(X a, TFlagsMask<X> b) { \
        return TFlagsMask<X>(static_cast<int>(a) | b.value()); \
    } \
    SK_MAYBE_UNUSED constexpr X operator&(TFlagsMask<X> a, TFlagsMask<X> b) { \
        return static_cast<X>(a.value() & b.value()); \
    } \
    SK_MAYBE_UNUSED constexpr X operator&(TFlagsMask<X> a, X b) { \
        return static_cast<X>(a.value() & static_cast<int>(b)); \
    } \
    SK_MAYBE_UNUSED constexpr X operator&(X a, TFlagsMask<X> b) { \
        return static_cast<X>(static_cast<int>(a) & b.value()); \
    } \
    SK_MAYBE_UNUSED inline X& operator&=(X& a, TFlagsMask<X> b) { \
        return (a = a & b); \
    } \

#define DECL_BITFIELD_CLASS_OPS_FRIENDS(X) \
    friend constexpr TFlagsMask<X> operator ~(X); \
    friend constexpr X operator |(X, X); \
    friend X& operator |=(X&, X); \
    friend constexpr bool operator &(X, X); \
    friend constexpr TFlagsMask<X> operator|(TFlagsMask<X>, TFlagsMask<X>); \
    friend constexpr TFlagsMask<X> operator|(TFlagsMask<X>, X); \
    friend constexpr TFlagsMask<X> operator|(X, TFlagsMask<X>); \
    friend constexpr X operator&(TFlagsMask<X>, TFlagsMask<X>); \
    friend constexpr X operator&(TFlagsMask<X>, X); \
    friend constexpr X operator&(X, TFlagsMask<X>); \
    friend X& operator &=(X&, TFlagsMask<X>)

///////////////////////////////////////////////////////////////////////////////

/**
 * Possible 3D APIs that may be used by Ganesh.
 */
enum class BackendApi : unsigned {
    kOpenGL,
    kVulkan,
    kMetal,
    kDirect3D,
    kDawn,
    /**
     * Mock is a backend that does not draw anything. It is used for unit tests
     * and to measure CPU overhead.
     */
    kMock,
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Used to say whether a texture has mip levels allocated or not.
 */
enum class Mipmapped : bool {
    kNo = false,
    kYes = true
};

/*
 * Can a BackendObject be rendered to?
 */
enum class Renderable : bool {
    kNo = false,
    kYes = true
};

/*
 * Used to say whether texture is backed by protected memory.
 */
enum class Protected : bool {
    kNo = false,
    kYes = true
};

///////////////////////////////////////////////////////////////////////////////

/**
 * GPU SkImage and SkSurfaces can be stored such that (0, 0) in texture space may correspond to
 * either the top-left or bottom-left content pixel.
 */
enum class SurfaceOrigin : int {
    kTopLeft,
    kBottomLeft,
};

typedef void* GpuFinishedContext;
typedef void (*GpuFinishedProc)(GpuFinishedContext finishedContext);

typedef void* GpuSubmittedContext;
typedef void (*GpuSubmittedProc)(GpuSubmittedContext submittedContext, bool success);

/**
 * Struct to supply options to flush calls.
 *
 * After issuing all commands, fNumSemaphore semaphores will be signaled by the gpu. The client
 * passes in an array of fNumSemaphores BackendSemaphores. In general these BackendSemaphore's
 * can be either initialized or not. If they are initialized, the backend uses the passed in
 * semaphore. If it is not initialized, a new semaphore is created and the BackendSemaphore
 * object is initialized with that semaphore. The semaphores are not sent to the GPU until the next
 * Context::submit call is made. See the Context::submit for more information.
 *
 * The client will own and be responsible for deleting the underlying semaphores that are stored
 * and returned in initialized BackendSemaphore objects. The BackendSemaphore objects
 * themselves can be deleted as soon as this function returns.
 *
 * If a finishedProc is provided, the finishedProc will be called when all work submitted to the gpu
 * from this flush call and all previous flush calls has finished on the GPU. If the flush call
 * fails due to an error and nothing ends up getting sent to the GPU, the finished proc is called
 * immediately.
 *
 * If a submittedProc is provided, the submittedProc will be called when all work from this flush
 * call is submitted to the GPU. If the flush call fails due to an error and nothing will get sent
 * to the GPU, the submitted proc is called immediately. It is possibly that when work is finally
 * submitted, that the submission actual fails. In this case we will not reattempt to do the
 * submission. Skia notifies the client of these via the success bool passed into the submittedProc.
 * The submittedProc is useful to the client to know when semaphores that were sent with the flush
 * have actually been submitted to the GPU so that they can be waited on (or deleted if the submit
 * fails).
 * Note about GL: In GL work gets sent to the driver immediately during the flush call, but we don't
 * really know when the driver sends the work to the GPU. Therefore, we treat the submitted proc as
 * we do in other backends. It will be called when the next Context::submit is called after the
 * flush (or possibly during the flush if there is no work to be done for the flush). The main use
 * case for the submittedProc is to know when semaphores have been sent to the GPU and even in GL
 * it is required to call Context::submit to flush them. So a client should be able to treat all
 * backend APIs the same in terms of how the submitted procs are treated.
 */
struct FlushInfo {
    size_t fNumSemaphores = 0;
    BackendSemaphore* fSignalSemaphores = nullptr;
    GpuFinishedProc fFinishedProc = nullptr;
    GpuFinishedContext fFinishedContext = nullptr;
    GpuSubmittedProc fSubmittedProc = nullptr;
    GpuSubmittedContext fSubmittedContext = nullptr;
};

/**
 * Enum used as return value when flush with semaphores so the client knows whether the valid
 * semaphores will be submitted on the next Context::submit call.
 */
enum class SemaphoresSubmitted : bool {
    kNo = false,
    kYes = true
};

} // namespace skgpu

#endif // skgpu_GraphiteTypes_DEFINED
