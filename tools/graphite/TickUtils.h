/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_TickUtils_DEFINED
#define skiatest_graphite_TickUtils_DEFINED

#include "src/gpu/graphite/ContextPriv.h"

#if defined(SK_DAWN)
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE
#endif

namespace skiatest::graphite {

// This must be implemented in the header so the defines are picked up appropriately
inline skgpu::graphite::TickCallback TickCallbackImpl() {
#if defined(SK_DAWN)
    return [](skgpu::graphite::Context* context, skgpu::graphite::SharedContext* sharedContext) {
        auto dawnContext = static_cast<skgpu::graphite::DawnSharedContext*>(sharedContext);
        dawnContext->device().Tick();
        context->checkAsyncWorkCompletion();
    };
#else
    // No other backend needs to Tick
    return [](skgpu::graphite::Context& context, sk_sp<SharedContext>) {};
#endif
}

}  // namespace skiatest::graphite

#endif
