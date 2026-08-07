/*
 * Copyright (c) 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This header file defines implementation details of how the trace macros in
// SkTraceEventCommon.h collect and store trace events. Anything not
// implementation-specific should go in SkTraceEventCommon.h instead of here.

#ifndef SkTraceEvent_DEFINED
#define SkTraceEvent_DEFINED

#include "src/core/SkTraceEventCommon.h" // IWYU pragma: export

// Makes it easier to add traces with a simple TRACE_EVENT0("skia", TRACE_FUNC).
#if defined(_MSC_VER)
    #define TRACE_FUNC __FUNCSIG__
#else
    #define TRACE_FUNC __PRETTY_FUNCTION__
#endif

#if defined(SK_ANDROID_FRAMEWORK_USE_PERFETTO) || defined(SK_USE_PERFETTO)
    // By default, const char* argument values are assumed to have long-lived scope
    // and will not be copied. Use this macro to force a const char* to be copied.
    //
    // TRACE_STR_COPY should be used with short-lived strings that should be copied immediately.
    // TRACE_STR_STATIC should be used with pointers to string literals with process lifetime.
    // Neither should be used for string literals known at compile time.
    //
    // E.g. TRACE_EVENT0("skia", TRACE_STR_COPY(something.c_str()));
    #define TRACE_STR_COPY(str) (::perfetto::DynamicString{str})

    // Allows callers to pass static strings that aren't known at compile time to trace functions.
    //
    // TRACE_STR_COPY should be used with short-lived strings that should be copied immediately.
    // TRACE_STR_STATIC should be used with pointers to string literals with process lifetime.
    // Neither should be used for string literals known at compile time.
    //
    // E.g. TRACE_EVENT0("skia", TRACE_STR_STATIC(this->name()));
    // No-op when Perfetto is disabled, or outside of Android framework.
    #define TRACE_STR_STATIC(str) (::perfetto::StaticString{str})
#else // !SK_ANDROID_FRAMEWORK_USE_PERFETTO && !SK_USE_PERFETTO
    // By default, const char* argument values are assumed to have long-lived scope
    // and will not be copied. Use this macro to force a const char* to be copied.
    //
    // TRACE_STR_COPY should be used with short-lived strings that should be copied immediately.
    // TRACE_STR_STATIC should be used with pointers to string literals with process lifetime.
    // Neither should be used for string literals known at compile time.
    //
    // E.g. TRACE_EVENT0("skia", TRACE_STR_COPY(something.c_str()));
    #define TRACE_STR_COPY(str) (str)

    // Allows callers to pass static strings that aren't known at compile time to trace functions.
    //
    // TRACE_STR_COPY should be used with short-lived strings that should be copied immediately.
    // TRACE_STR_STATIC should be used with pointers to string literals with process lifetime.
    // Neither should be used for string literals known at compile time.
    //
    // E.g. TRACE_EVENT0("skia", TRACE_STR_STATIC(this->name()));
    // No-op when Perfetto is disabled, or outside of Android framework.
    #define TRACE_STR_STATIC(str) (str)
#endif // SK_ANDROID_FRAMEWORK_USE_PERFETTO

#endif
