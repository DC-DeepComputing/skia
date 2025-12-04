/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/GlyphVector.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "include/private/text/gpu/SubRunAllocator.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/StrikeForGPU.h"

#include <climits>
#include <optional>
#include <utility>

class SkStrikeClient;

namespace sktext::gpu {

// -- GlyphVector ----------------------------------------------------------------------------------
GlyphVector::GlyphVector(SkStrikePromise&& strikePromise, SkSpan<SkPackedGlyphID> glyphs)
        : fStrikePromise{std::move(strikePromise)}, fGlyphs{glyphs} {
    SkASSERT(!fGlyphs.empty());
}

GlyphVector GlyphVector::Make(SkStrikePromise&& promise,
                              SkSpan<const SkPackedGlyphID> packedIDs,
                              SubRunAllocator* alloc) {
    SkASSERT(!packedIDs.empty());
    int count = SkToInt(packedIDs.size());
    SkPackedGlyphID* glyphs = alloc->makePODArray<SkPackedGlyphID>(count);
    for (int i = 0; i < count; i++) {
        glyphs[i] = packedIDs[i];
    }
    return GlyphVector{std::move(promise), SkSpan(glyphs, count)};
}

std::optional<GlyphVector> GlyphVector::MakeFromBuffer(SkReadBuffer& buffer,
                                                       const SkStrikeClient* client,
                                                       SubRunAllocator* alloc) {
    std::optional<SkStrikePromise> promise =
            SkStrikePromise::MakeFromBuffer(buffer, client, SkStrikeCache::GlobalStrikeCache());
    if (!buffer.validate(promise.has_value())) {
        return std::nullopt;
    }

    int32_t glyphCount = buffer.read32();
    // Since the glyph count can never be zero. There was a buffer reading problem.
    if (!buffer.validate(glyphCount > 0)) {
        return std::nullopt;
    }

    // Make sure we can multiply without overflow in the check below.
    static constexpr int kMaxCount = (int)(INT_MAX / sizeof(uint32_t));
    if (!buffer.validate(glyphCount <= kMaxCount)) {
        return std::nullopt;
    }

    // Check for enough bytes to populate the packedGlyphID array. If not enough something has
    // gone wrong.
    if (!buffer.validate(glyphCount * sizeof(uint32_t) <= buffer.available())) {
        return std::nullopt;
    }

    SkPackedGlyphID* glyphs = alloc->makePODArray<SkPackedGlyphID>(glyphCount);
    for (int i = 0; i < glyphCount; i++) {
        glyphs[i] = SkPackedGlyphID(buffer.readUInt());
    }
    return GlyphVector{std::move(promise.value()), SkSpan(glyphs, glyphCount)};
}

void GlyphVector::flatten(SkWriteBuffer& buffer) const {
    // There should never be a glyph vector with zero glyphs.
    SkASSERT(!fGlyphs.empty());
    fStrikePromise.flatten(buffer);

    // Write out the span of packedGlyphIDs.
    buffer.write32(SkTo<int32_t>(fGlyphs.size()));
    for (SkPackedGlyphID packedID : fGlyphs) {
        buffer.writeUInt(packedID.value());
    }
}

}  // namespace sktext::gpu
