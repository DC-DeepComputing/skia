/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyContext.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"

SkKeyContext::SkKeyContext(skgpu::graphite::Recorder* recorder,
                           const SkM44& local2Dev,
                           const SkM44& dev2Local)
        : fRecorder(recorder)
        , fLocal2Dev(local2Dev)
        , fDev2Local(dev2Local) {
    fDictionary = fRecorder->priv().shaderCodeDictionary();
}

SkKeyContext::SkKeyContext(const SkKeyContext& other, const SkMatrix& childLM)
        : fRecorder(other.fRecorder)
        , fLocal2Dev(other.fLocal2Dev)
        , fDev2Local(other.fDev2Local)
        , fDictionary(other.fDictionary) {
    fLocalMatrix = SkMatrix::Concat(childLM, other.fLocalMatrix);
}

#endif

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"

SkKeyContext::SkKeyContext(GrRecordingContext* rContext) : fRecordingContext(rContext) {
    // TODO: fill this out for Ganesh
    fDictionary = nullptr;
}
#endif
