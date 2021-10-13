/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCycles_DEFINED
#define SkCycles_DEFINED

#include <cstdint>

class SkCycles {
public:
    static uint64_t Now() {
        uint32_t cpuInfo;
        return __builtin_ia32_rdtscp(&cpuInfo);
    }
};

#endif  // SkCycles_DEFINED
