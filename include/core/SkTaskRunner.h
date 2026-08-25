/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskRunner_DEFINED
#define SkTaskRunner_DEFINED

#include "include/core/SkTypes.h"

class SK_API SkTaskRunner {
public:
    virtual ~SkTaskRunner();

protected:
    SkTaskRunner() = default;
    SkTaskRunner(const SkTaskRunner&) = delete;
    SkTaskRunner& operator=(const SkTaskRunner&) = delete;
};

#endif //SkTaskRunner_DEFINED
