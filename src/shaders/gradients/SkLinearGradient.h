/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearGradient_DEFINED
#define SkLinearGradient_DEFINED

struct GradData;

#include "src/shaders/gradients/SkGradientShaderPriv.h"

class SkLinearGradient : public SkGradientShaderBase {
public:
    static int Encode1(SkShaderCodeDictionary*, SkBackend, SkPaintParamsKey*, const GradData&);
#if defined(SK_DEBUG) || GR_TEST_UTILS || defined(SK_ENABLE_DUMP_GPU)
    static void Dump(const uint32_t* data, int numIntsAccessible);
#endif

    SkLinearGradient(const SkPoint pts[2], const Descriptor&);

    GradientType asAGradient(GradientInfo* info) const override;
#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif
    void addToKeyS(SkShaderCodeDictionary*,
                  SkBackend,
                  SkPaintParamsKey*,
                  SkUniformBlock*) const override;

protected:
    SkLinearGradient(SkReadBuffer& buffer);
    void flatten(SkWriteBuffer& buffer) const override;
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
#endif

    void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const final;

    skvm::F32 transformT(skvm::Builder*, skvm::Uniforms*,
                         skvm::Coord coord, skvm::I32* mask) const final;

private:
    SK_FLATTENABLE_HOOKS(SkLinearGradient)

    class LinearGradient4fContext;

    friend class SkGradientShader;
    using INHERITED = SkGradientShaderBase;
    const SkPoint fStart;
    const SkPoint fEnd;
};

#endif
