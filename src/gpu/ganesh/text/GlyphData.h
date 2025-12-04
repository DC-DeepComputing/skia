/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ganesh_GlyphData_DEFINED
#define skgpu_ganesh_GlyphData_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/ganesh/text/Glyph.h"
#include "src/text/gpu/BackendGlyphData.h"

#include <cstdint>
#include <tuple>

class GrMeshDrawTarget;

namespace sktext::gpu {
class GlyphVector;
class StrikeCache;
class VertexFiller;
}

namespace skgpu {
enum class MaskFormat : int;
}

namespace skgpu::ganesh {
class TextStrike;

/**
 * Stored on SubRun. It maintains mapping from SubRun's packed glyph IDs to atlas locations,
 * coordinating updates to this data as the data in the atlas evolves.
 */
class GlyphData final : public sktext::gpu::BackendGlyphData {
    class PrivateKey {
        PrivateKey() {}
        friend class GlyphData;
    };

public:
    // Create GlyphData for the given glyph count (single heap allocation).
    static std::unique_ptr<GlyphData, sktext::gpu::SubRunAllocator::Destroyer> Make(
            sktext::gpu::SubRunAllocator* alloc, int glyphCount);

    ~GlyphData() override;

    // Regenerate atlas entries for glyphs in range [begin, end).
    // Returns {success, glyphs_placed_in_atlas}.
    std::tuple<bool, int> regenerateAtlas(int begin,
                                          int end,
                                          sktext::gpu::GlyphVector& glyphVector,
                                          MaskFormat maskFormat,
                                          int srcPadding,
                                          GrMeshDrawTarget* target);

    size_t vertexStride(MaskFormat, const SkMatrix& positionMatrix) const;

    void fillVertexData(const sktext::gpu::VertexFiller& vf,
                        int offset,
                        int count,
                        const SkPMColor4f& pmColor,
                        const SkMatrix& positionMatrix,
                        SkIRect clip,
                        void* vertexBuffer);

#if defined(GPU_TEST_UTILS)
    void testingOnly_prepareGlyphs(sktext::gpu::GlyphVector& glyphVector,
                                   sktext::gpu::StrikeCache* strikeCache) {
        prepareGlyphs(glyphVector, strikeCache);
    }
#endif

    GlyphData(int glyphCount, PrivateKey);

private:
    GlyphData() = delete;
    GlyphData(const GlyphData&) = delete;
    GlyphData(GlyphData&&) = delete;
    GlyphData& operator=(const GlyphData&) = delete;
    GlyphData& operator=(GlyphData&&) = delete;

    void prepareGlyphs(sktext::gpu::GlyphVector& glyphVector,
                       sktext::gpu::StrikeCache* strikeCache);

    SkSpan<const Glyph*> glyphs() const {
        return SkSpan{const_cast<const Glyph**>(fGlyphPtrs), fGlyphCount};
    }

    int fGlyphCount;
    sk_sp<TextStrike> fTextStrike{nullptr};
    uint64_t fAtlasGeneration{AtlasGenerationCounter::kInvalidGeneration};
    BulkUsePlotUpdater fBulkUseUpdater;

    // Flexible array member: variable-length array of Glyph pointers allocated inline
    // Ideally, this would be empty [] or [0] but that triggers a warning on MSVC
    Glyph* fGlyphPtrs[1];
};

}  // namespace skgpu::ganesh

#endif  // skgpu_ganesh_GlyphData_DEFINED
