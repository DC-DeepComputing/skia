/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/dawn/DawnErrorChecker.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

#include <string>

#if defined(__EMSCRIPTEN__) && !defined(SK_USE_EMDAWNWEBGPU)
#include <emscripten/version.h>
#endif

using namespace skgpu::graphite;

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnErrorScopes, reporter, context, testContext) {
    auto* sharedContext = static_cast<DawnSharedContext*>(context->priv().sharedContext());
    if (!sharedContext->hasTick()) {
        return;
    }

    DawnErrorChecker checker(sharedContext);
    wgpu::BufferDescriptor descriptor{};
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::None;
    auto buffer = sharedContext->device().CreateBuffer(&descriptor);
    REPORTER_ASSERT(reporter, checker.popErrorScopes() == DawnErrorType::kValidation);
    REPORTER_ASSERT(reporter, checker.popErrorScopes() == DawnErrorType::kNoError);
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnShaderCompilationErrors, reporter, context, testContext) {
    auto* sharedContext = static_cast<DawnSharedContext*>(context->priv().sharedContext());
    if (!sharedContext->hasTick()) {
        return;
    }

    // Older legacy bindings do not implement GetCompilationInfo.
#if defined(__EMSCRIPTEN__) && !defined(SK_USE_EMDAWNWEBGPU) &&                                 \
        (__EMSCRIPTEN_major__ < 3 || (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ < 1) || \
         (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ == 1 && __EMSCRIPTEN_tiny__ < 51))
    return;
#endif

    struct ErrorHandler : skgpu::ShaderErrorHandler {
        int fCalls = 0;
        std::string fMessage;

        void compileError(const char*, const char* message, bool) override {
            ++fCalls;
            fMessage = message;
        }
    } handler;

    DawnErrorChecker checker(sharedContext);
    SkSL::NativeShader shader;
    shader.fText = "this is not valid WGSL";
    wgpu::ShaderModule module;
    REPORTER_ASSERT(reporter,
                    !DawnCompileWGSLShaderModule(
                            sharedContext, "invalid shader", shader, &module, &handler));
    REPORTER_ASSERT(reporter, handler.fCalls == 1);
    REPORTER_ASSERT(reporter, !handler.fMessage.empty());
    REPORTER_ASSERT(reporter, checker.popErrorScopes() == DawnErrorType::kValidation);
}
