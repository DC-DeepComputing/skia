/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Log_DEFINED
#define skgpu_Log_DEFINED

namespace skgpu {
    enum class Priority {
        kError = 0,
        kWarning = 1,
        kDebug = 2,
    };
};  // namespace skgpu

#ifdef SK_DEBUG
    #define SKGPU_MAX_PRIORITY Priority::kWarning

    #define SKGPU_LOG(priority, fmt, ...) if (priority <= SKGPU_MAX_PRIORITY) { \
                                              SkDebugf("[graphite] " fmt "\n", ##__VA_ARGS__); \
                                          }
#else
    #define SKGPU_LOG(priority, fmt, ...)
#endif

#endif // skgpu_Log_DEFINED
