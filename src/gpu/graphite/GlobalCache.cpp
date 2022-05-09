/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GlobalCache.h"

#include "src/core/SkShaderCodeDictionary.h"

namespace skgpu::graphite {

GlobalCache::GlobalCache(bool foo) : fShaderCodeDictionary(std::make_unique<SkShaderCodeDictionary>(foo)) {}

GlobalCache::~GlobalCache() {};

} // namespace skgpu::graphite
