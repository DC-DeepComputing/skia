/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skiatest_GPUTest_DEFINED
#define skiatest_GPUTest_DEFINED

#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

namespace skgpu::graphite { class Context; }

namespace skiatest {

using GrContextFactoryContextType = sk_gpu_test::GrContextFactory::ContextType;

typedef void GrContextTestFn(Reporter*, const sk_gpu_test::ContextInfo&);
typedef bool GrContextTypeFilterFn(GrContextFactoryContextType);

extern bool IsGLContextType(GrContextFactoryContextType);
extern bool IsVulkanContextType(GrContextFactoryContextType);
extern bool IsMetalContextType(GrContextFactoryContextType);
extern bool IsDawnContextType(GrContextFactoryContextType);
extern bool IsDirect3DContextType(GrContextFactoryContextType);
extern bool IsRenderingGLContextType(GrContextFactoryContextType);
extern bool IsMockContextType(GrContextFactoryContextType);
void RunWithGPUTestContexts(GrContextTestFn*, GrContextTypeFilterFn*, Reporter*,
                            const GrContextOptions&);

namespace graphite {

typedef void GraphiteTestFn(Reporter*, skgpu::graphite::Context*);

void RunWithGraphiteTestContexts(GraphiteTestFn*, Reporter*);

} // namespace graphite

} // namespace skiatest

// TODO update all the callsites to support CtsEnforcement
#define DEF_GRAPHITE_TEST(name, reporter)                                            \
    static void test_##name(skiatest::Reporter*);                                    \
    static void test_graphite_##name(skiatest::Reporter* reporter,                   \
                                     const GrContextOptions& /*unused*/) {           \
        test_##name(reporter);                                                       \
    }                                                                                \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name,                  \
                                                             /*gpu=*/false,          \
                                                             /*graphite=*/true,      \
                                                             CtsEnforcement::kNever, \
                                                             test_graphite_##name)); \
    void test_##name(skiatest::Reporter* reporter)

// TODO update all the callsites to support CtsEnforcement
#define DEF_GRAPHITE_TEST_FOR_CONTEXTS(name, reporter, graphite_context)                      \
    static void test_##name(skiatest::Reporter*, skgpu::graphite::Context*);                  \
    static void test_graphite_contexts_##name(skiatest::Reporter* _reporter,                  \
                                              const GrContextOptions& /*unused*/) {           \
        skiatest::graphite::RunWithGraphiteTestContexts(test_##name, _reporter);              \
    }                                                                                         \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name,                           \
                                                             /*gpu=*/false,                   \
                                                             /*graphite=*/true,               \
                                                             CtsEnforcement::kNever,          \
                                                             test_graphite_contexts_##name)); \
    void test_##name(skiatest::Reporter* reporter, skgpu::graphite::Context* graphite_context)

// TODO update all the callsites to pass real API values
#define DEF_GPUTEST(name, reporter, options, ctsEnforcement)                                 \
    static void test_##name(skiatest::Reporter*, const GrContextOptions&);                   \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(                                \
            #name, /*gpu=*/true, /*graphite=*/false, ctsEnforcement, test_##name, nullptr)); \
    void test_##name(skiatest::Reporter* reporter, const GrContextOptions& options)

#define DEF_CONDITIONAL_GPUTEST_FOR_CONTEXTS(                                                    \
        name, context_filter, reporter, context_info, options_filter, condition, ctsEnforcement) \
    static void test_##name(skiatest::Reporter*, const sk_gpu_test::ContextInfo&);               \
    static void test_gpu_contexts_##name(skiatest::Reporter* reporter,                           \
                                         const GrContextOptions& options) {                      \
        skiatest::RunWithGPUTestContexts(test_##name, context_filter, reporter, options);        \
    }                                                                                            \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name,                              \
                                                             /*gpu=*/true,                       \
                                                             /*graphite=*/false,                 \
                                                             ctsEnforcement,                     \
                                                             test_gpu_contexts_##name,           \
                                                             options_filter),                    \
                                              condition);                                        \
    void test_##name(skiatest::Reporter* reporter, const sk_gpu_test::ContextInfo& context_info)

#define DEF_CONDITIONAL_GPUTEST_FOR_ALL_CONTEXTS(                \
        name, reporter, context_info, condition, ctsEnforcement) \
    DEF_CONDITIONAL_GPUTEST_FOR_CONTEXTS(                        \
            name, nullptr, reporter, context_info, nullptr, condition, ctsEnforcement)

#define DEF_CONDITIONAL_GPUTEST_FOR_RENDERING_CONTEXTS(                                     \
        name, reporter, context_info, condition, ctsEnforcement)                            \
    DEF_CONDITIONAL_GPUTEST_FOR_CONTEXTS(name,                                              \
                                         sk_gpu_test::GrContextFactory::IsRenderingContext, \
                                         reporter,                                          \
                                         context_info,                                      \
                                         nullptr,                                           \
                                         condition,                                         \
                                         ctsEnforcement)

#define DEF_GPUTEST_FOR_CONTEXTS(                                                     \
        name, context_filter, reporter, context_info, options_filter, ctsEnforcement) \
    DEF_CONDITIONAL_GPUTEST_FOR_CONTEXTS(                                             \
            name, context_filter, reporter, context_info, options_filter, true, ctsEnforcement)

#define DEF_GPUTEST_FOR_ALL_CONTEXTS(name, reporter, context_info, ctsEnforcement) \
    DEF_GPUTEST_FOR_CONTEXTS(name, nullptr, reporter, context_info, nullptr, ctsEnforcement)

#define DEF_GPUTEST_FOR_RENDERING_CONTEXTS(name, reporter, context_info, ctsEnforcement) \
    DEF_GPUTEST_FOR_CONTEXTS(name,                                                       \
                             sk_gpu_test::GrContextFactory::IsRenderingContext,          \
                             reporter,                                                   \
                             context_info,                                               \
                             nullptr,                                                    \
                             ctsEnforcement)
#define DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(name, reporter, context_info, ctsEnforcement) \
    DEF_GPUTEST_FOR_CONTEXTS(                                                         \
            name, &skiatest::IsGLContextType, reporter, context_info, nullptr, ctsEnforcement)
#define DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(name, reporter, context_info, ctsEnforcement) \
    DEF_GPUTEST_FOR_CONTEXTS(name,                                                          \
                             &skiatest::IsRenderingGLContextType,                           \
                             reporter,                                                      \
                             context_info,                                                  \
                             nullptr,                                                       \
                             ctsEnforcement)
#define DEF_GPUTEST_FOR_MOCK_CONTEXT(name, reporter, context_info) \
    DEF_GPUTEST_FOR_CONTEXTS(name,                                 \
                             &skiatest::IsMockContextType,         \
                             reporter,                             \
                             context_info,                         \
                             nullptr,                              \
                             CtsEnforcement::kNever)
#define DEF_GPUTEST_FOR_VULKAN_CONTEXT(name, reporter, context_info, ctsEnforcement) \
    DEF_GPUTEST_FOR_CONTEXTS(                                                        \
            name, &skiatest::IsVulkanContextType, reporter, context_info, nullptr, ctsEnforcement)
#define DEF_GPUTEST_FOR_METAL_CONTEXT(name, reporter, context_info) \
    DEF_GPUTEST_FOR_CONTEXTS(name,                                  \
                             &skiatest::IsMetalContextType,         \
                             reporter,                              \
                             context_info,                          \
                             nullptr,                               \
                             CtsEnforcement::kNever)
#define DEF_GPUTEST_FOR_D3D_CONTEXT(name, reporter, context_info) \
    DEF_GPUTEST_FOR_CONTEXTS(name,                                \
                             &skiatest::IsDirect3DContextType,    \
                             reporter,                            \
                             context_info,                        \
                             nullptr,                             \
                             CtsEnforcement::kNever)
#define DEF_GPUTEST_FOR_DAWN_CONTEXT(name, reporter, context_info) \
    DEF_GPUTEST_FOR_CONTEXTS(name,                                 \
                             &skiatest::IsDawnContextType,         \
                             reporter,                             \
                             context_info,                         \
                             nullptr,                              \
                             CtsEnforcement::kNever)

#endif