/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_BackendGlyphData_DEFINED
#define sktext_gpu_BackendGlyphData_DEFINED

#include <cstdint>

#include "include/private/text/gpu/SubRunAllocator.h"

namespace sktext::gpu {

class GlyphVector;
class StrikeCache;

/**
 * BackendGlyphData is an abstract interface for backend-specific per-glyph and per-SubRun data.
 * It is stored on AtlasSubRun. It does not participate in [de]serialization.
 */
class BackendGlyphData {
public:
    virtual ~BackendGlyphData() = 0;
};

inline BackendGlyphData::~BackendGlyphData() {}

template <std::derived_from<BackendGlyphData> T = BackendGlyphData>
using BackendGlyphDataOwner = std::unique_ptr<T, SubRunAllocator::Destroyer>;

}  // namespace sktext::gpu

#endif  // sktext_gpu_BackendGlyphData_DEFINED
