/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if !defined(__has_feature)
    #define __has_feature(x) 0
#endif

//#if __has_feature(thread_sanitizer)

#if 0
extern "C" {

    SK_API const char* __tsan_default_suppressions() {
        const char* kDefaultSuppressions =

            // b/373932392 (Precompile isn't thread safe on Native Vulkan)
            "race:anv_shader_bin_create\n"                 // Intel Vulkan drivers (mesa-22.1.3).

            "race:libgallium-25.0.7-0\n" // Ubuntu24.04/QuadroP400 driver
            "race:libgallium-24.2.8-1";  // Ubuntu24.04/IntelIrisXe driver

        SkDebugf("%s\n", kDefaultSuppressions);
        return kDefaultSuppressions;
    }
}
#endif

//#endif
