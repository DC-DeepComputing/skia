/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/utils/SkTracedRuntimeEffect.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/sksl/SkSLDebugTrace.h"
#include "src/shaders/SkRuntimeShader.h"
#include "src/shaders/SkShaderBase.h"
#include "src/sksl/tracing/SkSLDebugTraceImpl.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#include <utility>
#include <vector>

static sk_sp<SkSL::DebugTracePriv> make_debug_trace(SkRuntimeEffect* effect,
                                                    const SkIPoint& coord) {
    sk_sp<SkSL::DebugTracePriv> debugTrace = SkSL::DebugTraceImpl::Make();
    debugTrace->setSource(effect->source());
    debugTrace->setTraceCoord(coord);
    return debugTrace;
}

class SkTracedRuntimeShader {
public:
    static SkRuntimeEffects::TracedShader MakeTracedClone(SkRuntimeShader* shader,
                                                          const SkIPoint& coord) {
        SkASSERT(shader);
        sk_sp<SkRuntimeEffect> unoptimized = shader->fEffect->makeUnoptimizedClone();
        sk_sp<SkSL::DebugTracePriv> debugTrace = make_debug_trace(unoptimized.get(), coord);
        auto debugShader = sk_make_sp<SkRuntimeShader>(
                unoptimized, debugTrace, shader->uniformData(nullptr), SkSpan(shader->fChildren));

        return SkRuntimeEffects::TracedShader{std::move(debugShader), std::move(debugTrace)};
    }
};

namespace SkRuntimeEffects {
TracedShader MakeTraced(sk_sp<SkShader> shader,
                                          const SkIPoint& traceCoord) {
    SkASSERT(shader);
    SkRuntimeEffect* effect = as_SB(shader)->asRuntimeEffect();
    if (!effect) {
        return TracedShader{nullptr, nullptr};
    }
    // An SkShader with an attached SkRuntimeEffect must be an SkRuntimeShader.
    SkRuntimeShader* rtShader = static_cast<SkRuntimeShader*>(shader.get());
    return SkTracedRuntimeShader::MakeTracedClone(rtShader, traceCoord);
}

}
