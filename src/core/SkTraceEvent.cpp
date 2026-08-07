/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTraceEventCommon.h"

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
bool SkAndroidFrameworkTraceUtil::gEnableAndroidTracing = false;
bool SkAndroidFrameworkTraceUtil::gUsePerfettoTrackEvents = false;
#endif

#if defined(SK_ANDROID_FRAMEWORK_USE_PERFETTO) || defined(SK_USE_PERFETTO)
PERFETTO_TRACK_EVENT_STATIC_STORAGE_IN_NAMESPACE_WITH_ATTRS(skia_tracing, SK_API);
#endif
