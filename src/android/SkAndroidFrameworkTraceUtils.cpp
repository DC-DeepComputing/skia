/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkAndroidFrameworkTraceUtils.h"

#include "include/core/SkTypes.h"

bool SkAndroidFrameworkTraceUtils::gEnableDetailedTracing = false;
bool SkAndroidFrameworkTraceUtils::gUsePerfettoTrackEvents = false;

#ifdef SK_ANDROID_FRAMEWORK_USE_PERFETTO
#ifdef __ANDROID__
#include "src/core/SkTraceEventCommon.h"
// This line cannot be placed in a header, and must not apply to host builds.
PERFETTO_TRACK_EVENT_STATIC_STORAGE();
#endif // __ANDROID__

void SkAndroidFrameworkTraceUtils::initPerfetto() {
#ifdef __ANDROID__
    ::perfetto::TracingInitArgs perfettoArgs;
    perfettoArgs.backends |= perfetto::kSystemBackend;
    ::perfetto::Tracing::Initialize(perfettoArgs);
    ::skia::TrackEvent::Register();
#endif // __ANDROID__
}
#endif // SK_ANDROID_FRAMEWORK_USE_PERFETTO
