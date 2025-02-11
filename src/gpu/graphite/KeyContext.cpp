/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/KeyContext.h"

#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/RecorderPriv.h"

namespace skgpu::graphite {

KeyContext::KeyContext(skgpu::graphite::Recorder* recorder,
                       const SkM44& local2Dev,
                       const SkColorInfo& dstColorInfo,
                       OptimizeSampling optimizeSampling,
                       const SkColor4f& paintColor)
        : fRecorder(recorder)
        , fLocal2Dev(local2Dev)
        , fLocalMatrix(nullptr)
        , fDstColorInfo(dstColorInfo)
        , fOptimizeSampling(optimizeSampling)
        , fCaps(recorder->priv().caps()) {
    fDictionary = fRecorder->priv().shaderCodeDictionary();
    fRTEffectDict7aa7 = fRecorder->priv().runtimeEffectDictionary7aa7();
    fPaintColor = PaintParams::Color4fPrepForDst(paintColor, fDstColorInfo).makeOpaque().premul();
    fPaintColor.fA = paintColor.fA;
}

KeyContext::KeyContext(const KeyContext& other)
        : fRecorder(other.fRecorder)
        , fLocal2Dev(other.fLocal2Dev)
        , fLocalMatrix(other.fLocalMatrix)
        , fDictionary(other.fDictionary)
        , fRTEffectDict7aa7(other.fRTEffectDict7aa7)
        , fDstColorInfo(other.fDstColorInfo)
        , fPaintColor(other.fPaintColor)
        , fScope(other.fScope)
        , fOptimizeSampling(other.fOptimizeSampling)
        , fCaps(other.fCaps) {}

} // namespace skgpu::graphite
