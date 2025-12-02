/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_GlyphVector_DEFINED
#define sktext_gpu_GlyphVector_DEFINED

#include "include/core/SkSpan.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkStrike.h"  // IWYU pragma: keep
#include "src/text/StrikeForGPU.h"

#include <cstddef>
#include <cstdint>
#include <optional>

class SkReadBuffer;
class SkStrikeClient;
class SkWriteBuffer;

namespace sktext::gpu {
class SubRunAllocator;

// -- GlyphVector ----------------------------------------------------------------------------------
// GlyphVector provides a way to delay the lookup of Glyphs until the code is running on the GPU
// in single threaded mode. The GlyphVector is created in a multi-threaded environment, but the
// StrikeCache is only single threaded (and must be single threaded because of the atlas).
//
// GlyphVector contains only backend-agnostic data: the glyph IDs and strike information. This
// data is [de]serialized by its owning AtlasSubRun.
class GlyphVector {
public:
    GlyphVector(SkStrikePromise&& strikePromise, SkSpan<SkPackedGlyphID> glyphs);

    static GlyphVector Make(SkStrikePromise&& promise,
                            SkSpan<const SkPackedGlyphID> glyphs,
                            SubRunAllocator* alloc);

    SkSpan<const SkPackedGlyphID> glyphs() const { return fGlyphs; }

    const SkStrikePromise& strikePromise() const { return fStrikePromise; }

    static std::optional<GlyphVector> MakeFromBuffer(SkReadBuffer& buffer,
                                                     const SkStrikeClient* strikeClient,
                                                     SubRunAllocator* alloc);
    void flatten(SkWriteBuffer& buffer) const;

    // This doesn't need to include sizeof(GlyphVector) because this is embedded in each of
    // the sub runs.
    int unflattenSize() const { return GlyphVectorSize(fGlyphs.size()); }

    static size_t GlyphVectorSize(size_t count) { return sizeof(SkPackedGlyphID) * count; }

private:
    SkStrikePromise fStrikePromise;
    SkSpan<SkPackedGlyphID> fGlyphs;
};
}  // namespace sktext::gpu
#endif  // sktext_gpu_GlyphVector_DEFINED
