/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskRunner_DEFINED
#define SkTaskRunner_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

class SK_API SkTask : public SkRefCnt {
public:
    ~SkTask() override {}

    enum class Priority { kHigh = 0, kLow = 1 };

    virtual void doit() = 0;
};

class SK_API SkTaskRunner : public SkRefCnt {
public:
    ~SkTaskRunner() override;

    virtual void add(sk_sp<SkTask>, SkTask::Priority) = 0;

    virtual void wait() = 0;

protected:
    SkTaskRunner() = default;
    SkTaskRunner(const SkTaskRunner&) = delete;
    SkTaskRunner& operator=(const SkTaskRunner&) = delete;
};

#endif // SkTaskRunner_DEFINED
