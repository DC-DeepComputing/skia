/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterShader_DEFINED
#define SkColorFilterShader_DEFINED

#include "src/core/SkColorFilterBase.h"
#include "src/shaders/SkShaderBase.h"

class SkArenaAlloc;

class SkColorFilterShader : public SkShaderBase {
public:
    SkColorFilterShader(sk_sp<SkShader> shader, float alpha, sk_sp<SkColorFilter> filter);

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

private:
    bool isOpaque() const override;
    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const SkStageRec&) const override;

    skvm::Color onProgram(skvm::Builder* builder,
                          skvm::Coord device,
                          skvm::Coord local,
                          skvm::Color paint,
                          const SkMatrixProvider& matrices,
                          const SkMatrix* localM,
                          const SkColorInfo& dstInfo,
                          skvm::UniformsAllocator* uniforms) const override;
    bool onKeyAndUniforms(SkKeyBuilder* key,
                          skvm::UniformsData* uniforms,
                          const SkMatrixProvider& matrices,
                          const SkMatrix* localM,
                          const SkColorInfo& dstInfo,
                          SkArenaAlloc* alloc) const override;

    SK_FLATTENABLE_HOOKS(SkColorFilterShader)

    sk_sp<SkShader>          fShader;
    sk_sp<SkColorFilterBase> fFilter;
    float                    fAlpha;

    using INHERITED = SkShaderBase;
};

#endif
