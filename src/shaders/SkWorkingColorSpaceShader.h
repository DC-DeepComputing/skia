/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkWorkingColorSpaceShader_DEFINED
#define SkWorkingColorSpaceShader_DEFINED

#include "src/shaders/SkShaderBase.h"

class SkColorSpace;
class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

class SkWorkingColorSpaceShader final : public SkShaderBase {
public:
    SkWorkingColorSpaceShader(sk_sp<SkShader> shader, sk_sp<SkColorSpace> workingSpace)
            : fShader(std::move(shader)), fWorkingSpace(std::move(workingSpace)) {
        SkASSERT(fWorkingSpace);
    }

    ShaderType type() const override { return ShaderType::kWorkingColorSpace; }

    sk_sp<SkShader> shader() const { return fShader; }
    sk_sp<SkColorSpace> workingSpace() const { return fWorkingSpace; }

private:
    bool appendStages(const SkStageRec& rec, const SkShaders::MatrixRec&) const override;

    friend void ::SkRegisterWorkingColorSpaceShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkWorkingColorSpaceShader)

    void flatten(SkWriteBuffer& buffer) const override;

    sk_sp<SkShader> fShader;
    sk_sp<SkColorSpace> fWorkingSpace;
};

#endif
