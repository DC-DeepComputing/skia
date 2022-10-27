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

#if 0
    SkDebugf("  childLM       *     priorLM      =    newLM\n");
    SkDebugf("%.2f %.2f %.2f     %.1f %.1f %.1f     %.1f %.1f %.1f\n",
             childLM[0], childLM[1], childLM[2],
             other.fLocalMatrix[0],  other.fLocalMatrix[1], other.fLocalMatrix[2],
             fLocalMatrix[0], fLocalMatrix[1], fLocalMatrix[2]);
    SkDebugf("%.2f %.2f %.2f     %.1f %.1f %.1f     %.1f %.1f %.1f\n",
             childLM[3], childLM[4], childLM[5],
             other.fLocalMatrix[3],  other.fLocalMatrix[4], other.fLocalMatrix[5],
             fLocalMatrix[3], fLocalMatrix[4], fLocalMatrix[5]);
    SkDebugf("%.2f %.2f %.2f     %.1f %.1f %.1f     %.1f %.1f %.1f\n",
             childLM[6], childLM[7], childLM[8],
             other.fLocalMatrix[6],  other.fLocalMatrix[7], other.fLocalMatrix[8],
             fLocalMatrix[6], fLocalMatrix[7], fLocalMatrix[8]);
#endif
}

#endif

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"

SkKeyContext::SkKeyContext(GrRecordingContext* rContext) : fRecordingContext(rContext) {
    // TODO: fill this out for Ganesh
    fDictionary = nullptr;
}
#endif
