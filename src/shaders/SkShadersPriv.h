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

// Internal factories for shader creation
namespace SkShadersPriv {
//    static sk_sp<SkShader> Empty();
//    static sk_sp<SkShader> Color(SkColor);
    SkShader* Color(SkArenaAlloc*, const SkColor4f&, sk_sp<SkColorSpace>);
//    static sk_sp<SkShader> Blend(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src);
    SkShaderBase* Blend(SkArenaAlloc*,
                        sk_sp<SkBlender>,
                        sk_sp<SkShader> dst,
                        sk_sp<SkShader> src);
};

#endif // SkShadersPriv_DEFINED
