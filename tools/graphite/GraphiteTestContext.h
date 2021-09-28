/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphiteTestContext_DEFINED
#define skgpu_GraphiteTestContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "experimental/graphite/include/GraphiteTypes.h"

namespace skgpu {
    class Context;
}

namespace sk_graphite_test {

/**
 * An offscreen 3D context. This class is intended for Skia's internal testing needs and not
 * for general use.
 */
class TestContext {
public:
    TestContext(const TestContext&) = delete;
    TestContext& operator=(const TestContext&) = delete;

    virtual ~TestContext();

    virtual skgpu::BackendApi backend() = 0;

    virtual sk_sp<skgpu::Context> makeContext() = 0;

protected:
    TestContext();
};

}  // namespace sk_graphite_test

#endif // skgpu_GraphiteTestContext_DEFINED
