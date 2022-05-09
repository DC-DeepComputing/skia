/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GlobalCache.h"

#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Gpu.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::graphite {
GlobalCache::GlobalCache(const Gpu* gpu)
        : fShaderCodeDictionary(std::make_unique<SkShaderCodeDictionary>(
                  gpu && gpu->caps()->shaderCaps()->atan2ImplementedAsAtanYOverX())) {}

GlobalCache::~GlobalCache() {};

} // namespace skgpu::graphite
