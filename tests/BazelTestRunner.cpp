/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "tests/Test.h"
#include "tools/Registry.h"

#if SK_SUPPORT_GPU || defined(SK_GRAPHITE_ENABLED)
#include "tests/GPUTest.h"
#include "tools/gpu/GrContextFactory.h"
#endif

class BazelReporter : public skiatest::Reporter {
public:
    void reportFailed(const skiatest::Failure& failure) override {
            SkDebugf("FAIL: %s\n", failure.toString().c_str());
            fFailed = true;
    }
    bool allowExtendedTest() const override { return false; }
    bool verbose() const override { return false; }
    bool ok() { return !fFailed; }
private:
    bool fFailed = false;
};

#if SK_SUPPORT_GPU || defined(SK_GRAPHITE_ENABLED)
namespace skiatest {
bool IsGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
#if defined(SK_GL)
    return GrBackendApi::kOpenGL == sk_gpu_test::GrContextFactory::ContextTypeBackend(type);
#else
    return false;
#endif
}
bool IsVulkanContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return false;
}
bool IsMetalContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return false;
}
bool IsDirect3DContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return false;
}
bool IsDawnContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return false;
}
bool IsRenderingGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return false;
}
bool IsMockContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return type == sk_gpu_test::GrContextFactory::kMock_ContextType;
}

void RunWithGPUTestContexts(GrContextTestFn* test, GrContextTypeFilterFn* contextTypeFilter,
                            Reporter* reporter, const GrContextOptions& options) {
    sk_gpu_test::GrContextFactory factory(options);
    // TODO(kjlubick) We will likely only be compiled with one of these types.
    auto ctxType = sk_gpu_test::GrContextFactory::kGL_ContextType;

    if (contextTypeFilter && !(*contextTypeFilter)(ctxType)) {
        return;
    }

    sk_gpu_test::ContextInfo ctxInfo = factory.getContextInfo(ctxType);
    if (ctxInfo.directContext()) {
        (*test)(reporter, ctxInfo);
        // In case the test changed the current context make sure we move it back before
        // calling flush.
        ctxInfo.testContext()->makeCurrent();
        // Sync so any release/finished procs get called.
        ctxInfo.directContext()->flushAndSubmit(/*sync*/true);
    } else {
        SkDebugf("Unable to make direct context for GPU test.\n");
        SkASSERT(false);
    }
}

} // namespace skiatest
#endif

int main() {
    BazelReporter reporter;
    for (skiatest::Test test : skiatest::TestRegistry::Range()) {
        if (test.fType == skiatest::kCPU_TestType) {
            SkDebugf("Running %s\n", test.fName);
            test.run(&reporter);
            SkDebugf("\tDone\n");
        }
    }

#if SK_SUPPORT_GPU || defined(SK_GRAPHITE_ENABLED)
    GrContextOptions grCtxOptions;
    // TODO(kjlubick) DM has grContextOptions set via flags. Should this runner have that too?
    grCtxOptions.fExecutor = nullptr;
    grCtxOptions.fAllowPathMaskCaching = true;
    grCtxOptions.fFailFlushTimeCallbacks = false;
    grCtxOptions.fAllPathsVolatile = false;
    grCtxOptions.fGpuPathRenderers = GpuPathRenderers::kDefault;
    grCtxOptions.fDisableDriverCorrectnessWorkarounds = false;
    grCtxOptions.fResourceCacheLimitOverride = -1;
    grCtxOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
    for (skiatest::Test test : skiatest::TestRegistry::Range()) {
        if (test.fType != skiatest::kCPU_TestType) {
            SkDebugf("Running %s\n", test.fName);
            test.run(&reporter, grCtxOptions);
            SkDebugf("\tDone\n");
        }
    }
#endif

    if (reporter.ok()) {
        SkDebugf("PASS\n");
        return 0;
    }
    SkDebugf("FAIL\n");
    return 1;
}
