/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ProgramCache.h"

namespace {

static uint32_t next_id() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == skgpu::ProgramCache::kInvalidProgramID);
    return id;
}

} // anonymous namespace

namespace skgpu {

size_t ProgramCache::Hash::operator()(Combination c) const {
    return static_cast<int>(c.fShaderType) +
           static_cast<int>(c.fTileMode) +
           static_cast<int>(c.fBlendMode);
}

static const uint32_t kSolidColorFS = 1;
static const uint32_t kGradientFS   = 2;

ProgramCache::ProgramInfo::ProgramInfo(Combination c)
    : fID(next_id())
    , fCombination(c) {
}

uint32_t ProgramCache::findOrCreateProgram(Combination c) {
    if (c.fShaderType == ShaderCombo::ShaderType::kNone) {
        return kSolidColorFS;
    } else {
        return kGradientFS;
    }
}

} // namespace skgpu
