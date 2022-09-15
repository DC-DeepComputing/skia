/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFPArgs_DEFINED
#define GrFPArgs_DEFINED

#include "include/core/SkMatrix.h"
#include "src/shaders/SkShaderBase.h"

class GrColorInfo;
class GrRecordingContext;
class SkMatrixProvider;
class SkSurfaceProps;

struct GrFPArgs {
    GrFPArgs(GrRecordingContext* context,
             const SkMatrixProvider& matrixProvider,
             const GrColorInfo* dstColorInfo,
             const SkSurfaceProps& surfaceProps)
            : fContext(context)
            , fMatrixProvider(matrixProvider)
            , fDstColorInfo(dstColorInfo)
            , fSurfaceProps(surfaceProps) {
        SkASSERT(fContext);
    }

    class AppendLocalMatrix;

    GrFPArgs withNewMatrixProvider(const SkMatrixProvider& provider) const {
        GrFPArgs newArgs(fContext, provider, fDstColorInfo, fSurfaceProps);
        newArgs.fLocalMatrix = fLocalMatrix;
        return newArgs;
    }

    GrRecordingContext* fContext;
    const SkMatrixProvider& fMatrixProvider;

    const SkMatrix* fLocalMatrix = nullptr;

    const GrColorInfo* fDstColorInfo;

    const SkSurfaceProps& fSurfaceProps;
};

// Use this to augment a passed in GrFPArgs with an additional local matrix from the current level
// concatenated in order to invoke a child effect.
class GrFPArgs::AppendLocalMatrix final : public GrFPArgs {
public:
    AppendLocalMatrix(const GrFPArgs& args, const SkMatrix& childLM) : INHERITED(args) {
        if (!childLM.isIdentity()) {
            if (fLocalMatrix) {
                fStorage = SkShaderBase::ConcatLocalMatrices(*fLocalMatrix, childLM);
                fLocalMatrix = fStorage.isIdentity() ? nullptr : &fStorage;
            } else {
                fLocalMatrix = &childLM;
            }
        }
    }

private:
    AppendLocalMatrix(const AppendLocalMatrix&) = delete;
    AppendLocalMatrix& operator=(const AppendLocalMatrix&) = delete;

    SkMatrix fStorage;

    using INHERITED = GrFPArgs;
};

#endif
