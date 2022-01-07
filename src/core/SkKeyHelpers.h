/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKeyHelpers_DEFINED
#define SkKeyHelpers_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"

class SkPaintParamsKey;

namespace skgpu {
    class UniformData;
};

namespace NoShaderBlock {
    void Dump(const SkPaintParamsKey &, uint32_t headerOffset);
}

namespace LinearGradientShaderBlock {
    void Dump(const SkPaintParamsKey &, uint32_t headerOffset);
}

namespace RadialGradientShaderBlock {
    void Dump(const SkPaintParamsKey &, uint32_t headerOffset);
}

namespace SweepGradientShaderBlock {
    void Dump(const SkPaintParamsKey &, uint32_t headerOffset);
}

namespace ConicalGradientShaderBlock {
    void Dump(const SkPaintParamsKey &, uint32_t headerOffset);
}

namespace BlendModeBlock {

    SkBlendMode ExtractFromKey(const SkPaintParamsKey &, uint32_t headerOffset);
    void AddToKey(SkPaintParamsKey *,
                  std::vector<sk_sp<skgpu::UniformData>>*,
                  SkBlendMode);
    void Dump(const SkPaintParamsKey &, uint32_t headerOffset);

} // namespace BlendModeBlock



#endif // SkKeyHelpers_DEFINED
