/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Context_DEFINED
#define skgpu_Context_DEFINED

#include <vector>
#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"

namespace skgpu {

class Gpu;
namespace mtl { struct BackendContext; }

struct GradientCombo {
    GradientCombo() {}
    GradientCombo(std::vector<SkShader::GradientType> types,
                  std::vector<SkTileMode> tileModes)
            : fTypes(types)
            , fTileModes(tileModes) {
    }
    std::vector<SkShader::GradientType> fTypes;
    std::vector<SkTileMode> fTileModes;
};

struct ShaderCombo {
    std::vector<GradientCombo> fGradients;
    std::vector<SkBlendMode> fBlendModes;
};

class Context final : public SkRefCnt {
public:
    ~Context() override;

#ifdef SK_METAL
    static sk_sp<Context> MakeMetal(const skgpu::mtl::BackendContext&);
#endif

    void preCompile(const ShaderCombo&);

protected:
    Context(sk_sp<Gpu>);

private:
    sk_sp<Gpu> fGpu;
};

} // namespace skgpu

#endif // skgpu_Context_DEFINED

