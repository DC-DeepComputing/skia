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
#else
    #define SKGPU_MAX_PRIORITY Priority::kError
#endif
#define SKGPU_LOG(priority, fmt, ...) if (priority <= SKGPU_MAX_PRIORITY) { \
                                          SkDebugf("[graphite] " fmt "\n", ##__VA_ARGS__); \
                                      }

#endif // skgpu_Log_DEFINED
