
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/TestContext.h"

#include "include/gpu/GrDirectContext.h"
#include "src/core/SkTraceEvent.h"
#include "tools/gpu/GpuTimer.h"

namespace sk_gpu_test {
TestContext::TestContext() : fGpuTimer(nullptr) {}

TestContext::~TestContext() {
    // Subclass should call teardown.
    SkASSERT(!fGpuTimer);
}

sk_sp<GrDirectContext> TestContext::makeContext1(const GrContextOptions&) {
    return nullptr;
}

void TestContext::makeNotCurrent() const { this->onPlatformMakeNotCurrent(); }
void TestContext::makeCurrent() const { this->onPlatformMakeCurrent(); }

SkScopeExit TestContext::makeCurrentAndAutoRestore() const {
    auto asr = SkScopeExit(this->onPlatformGetAutoContextRestore());
    this->makeCurrent();
    return asr;
}

void TestContext::testAbandon() {
}

void TestContext::teardown() {
    fGpuTimer.reset();
}

}  // namespace sk_gpu_test
