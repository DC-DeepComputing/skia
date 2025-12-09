/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ganesh_Glyph_DEFINED
#define skgpu_ganesh_Glyph_DEFINED

#include "src/core/SkGlyph.h"
#include "src/gpu/AtlasTypes.h"

namespace skgpu::ganesh {

/**
 * Ganesh-specific glyph type with atlas location information.
 */
class Glyph {
public:
    explicit Glyph(SkPackedGlyphID id) : fPackedID(id) {}

    SkPackedGlyphID glyphID() const { return fPackedID; }

    const SkPackedGlyphID fPackedID;
    AtlasLocator fAtlasLocator;
};

}  // namespace skgpu::ganesh

#endif  // skgpu_ganesh_Glyph_DEFINED
