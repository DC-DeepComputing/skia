/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GlyphData_DEFINED
#define skgpu_graphite_GlyphData_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/text/Glyph.h"
#include "src/text/gpu/BackendGlyphData.h"

#include <cstdint>
#include <tuple>

namespace skgpu::graphite {
class Recorder;
class TextStrike;
}

namespace sktext::gpu {
class GlyphVector;
class StrikeCache;
class VertexFiller;
}

namespace skgpu::graphite {
class DrawWriter;

/**
 * Stored on SubRun. It maintains mapping from SubRun's packed glyph IDs to atlas locations,
 * coordinating updates to this data as the data in the atlas evolves.
 */
class GlyphData final : public sktext::gpu::BackendGlyphData {
public:
    // Create GlyphData for the given glyph count (single heap allocation).
    static std::unique_ptr<GlyphData> Make(int glyphCount);

    ~GlyphData() override;

    // Regenerate atlas entries for glyphs in range [begin, end).
    // Returns {success, glyphs_placed_in_atlas}.
    std::tuple<bool, int> regenerateAtlas(int begin,
                                          int end,
                                          const sktext::gpu::GlyphVector& glyphVector,
                                          skgpu::MaskFormat maskFormat,
                                          int srcPadding,
                                          Recorder* recorder);

    void fillInstanceData(const sktext::gpu::VertexFiller&,
                          DrawWriter* dw,
                          int offset,
                          int count,
                          unsigned short flags,
                          uint32_t ssboIndex,
                          SkScalar depth) const;

private:
    void prepareGlyphs(const sktext::gpu::GlyphVector& glyphVector,
                       sktext::gpu::StrikeCache* strikeCache);

    SkSpan<const Glyph*> glyphs() const {
        return SkSpan{const_cast<const Glyph**>(fGlyphPtrs), fGlyphCount};
    }

    GlyphData(int glyphCount);

    int fGlyphCount;
    sk_sp<TextStrike> fTextStrike;
    uint64_t fAtlasGeneration{AtlasGenerationCounter::kInvalidGeneration};
    BulkUsePlotUpdater fBulkUseUpdater;

    // Flexible array member: variable-length array of Glyph pointers allocated inline
    Glyph* fGlyphPtrs[];
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_GlyphData_DEFINED
