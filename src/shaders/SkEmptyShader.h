/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEmptyShader_DEFINED
#define SkEmptyShader_DEFINED

#include "src/shaders/SkShaderBase.h"

// TODO: move this to private, as there is a public factory on SkShader

/**
 *  \class SkEmptyShader
 *  A Shader that always draws nothing. Its createContext always returns nullptr.
 */
class SkEmptyShader : public SkShaderBase {
public:
    SkEmptyShader() {}

protected:
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override {
        return nullptr;
    }
#endif

    void flatten(SkWriteBuffer& buffer) const override {
        // Do nothing.
        // We just don't want to fall through to SkShader::flatten(),
        // which will write data we don't care to serialize or decode.
    }

    bool onAppendStages(const SkStageRec&) const override {
        return false;
    }

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

private:
    SK_FLATTENABLE_HOOKS(SkEmptyShader)

    using INHERITED = SkShaderBase;
};

#endif
