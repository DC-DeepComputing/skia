/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShadersPriv_DEFINED
#define SkShadersPriv_DEFINED

#include "include/core/SkRefCnt.h"

class SkShader;

// Internal utilities and factories for shader creation
namespace SkShadersPriv {
    SkShader* Color(SkArenaAlloc*, const SkColor4f&, sk_sp<SkColorSpace>);
    SkShader* Blend(SkArenaAlloc*,
                    sk_sp<SkBlender>,
                    sk_sp<SkShader> dst,
                    sk_sp<SkShader> src);
    size_t BlendShaderSize();
};

#endif // SkShadersPriv_DEFINED
