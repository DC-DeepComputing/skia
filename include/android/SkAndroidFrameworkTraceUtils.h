/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAndroidFrameworkTraceUtils_DEFINED
#define SkAndroidFrameworkTraceUtils_DEFINED

#include "include/core/SkTypes.h"

class SkAndroidFrameworkTraceUtils {
public:
    SkAndroidFrameworkTraceUtils() = delete;

    // Controls whether broad tracing is enabled. Warning: not thread-safe!
    //
    // Some key trace events may still be recorded when this is disabled, if a relevant tracing
    // session is active.
    //
    // ATrace is used by default, but can be replaced with Perfetto by calling
    // setUsePerfettoTrackEvents(true)
    static void setEnableDetailedTracing(bool enableDetailedTracing) {
        gEnableDetailedTracing = enableDetailedTracing;
    }

    // Deprecated: use setEnableDetailedTracing
    static void setEnableTracing(bool enableAndroidTracing) {
        setEnableDetailedTracing(enableAndroidTracing);
    }

    // Controls whether tracing uses Perfetto instead of ATrace. Warning: not thread-safe!
    //
    // Returns true if Skia was built with Perfetto, false otherwise.
    static bool setUsePerfettoTrackEvents(bool usePerfettoTrackEvents) {
#ifdef SK_ANDROID_FRAMEWORK_USE_PERFETTO
        // Ensure Perfetto is initialized if it wasn't already the preferred tracing backend.
        if (!gUsePerfettoTrackEvents && usePerfettoTrackEvents) {
            initPerfetto();
        }
        gUsePerfettoTrackEvents = usePerfettoTrackEvents;
        return true;
#else // !SK_ANDROID_FRAMEWORK_USE_PERFETTO
        return false;
#endif // SK_ANDROID_FRAMEWORK_USE_PERFETTO
    }

    static bool getEnableDetailedTracing() {
        return gEnableDetailedTracing;
    }

    // Deprecated: use getEnableDetailedTracing
    static bool getEnableTracing() {
        return getEnableDetailedTracing();
    }

    static bool getUsePerfettoTrackEvents() {
        return gUsePerfettoTrackEvents;
    }

private:
    static bool gEnableDetailedTracing;
    static bool gUsePerfettoTrackEvents;

#ifdef SK_ANDROID_FRAMEWORK_USE_PERFETTO
    // Initializes tracing systems, and establishes a connection to the 'traced' daemon.
    //
    // Can be called multiple times.
    static void initPerfetto();
#endif // SK_ANDROID_FRAMEWORK_USE_PERFETTO
};

// TODO(nscobie): remove this after migrating clients
using SkAndroidFrameworkTraceUtil = SkAndroidFrameworkTraceUtils;

#endif // SkAndroidFrameworkTraceUtils_DEFINED
