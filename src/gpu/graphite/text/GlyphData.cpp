/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/text/GlyphData.h"

#include "include/private/base/SkAssert.h"
#include "src/base/SkZip.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkStrike.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/text/Glyph.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"
#include "src/gpu/graphite/text/TextStrike.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/VertexFiller.h"

using MaskFormat = skgpu::MaskFormat;

namespace skgpu::graphite {

GlyphData::GlyphData(int glyphCount, PrivateKey) : fGlyphCount{glyphCount} {
    SkASSERT(fGlyphCount > 0);
}

GlyphData::~GlyphData() = default;

sktext::gpu::BackendGlyphDataOwner<GlyphData> GlyphData::Make(sktext::gpu::SubRunAllocator* alloc,
                                                              int glyphCount) {
    SkASSERT(glyphCount > 0);

    // Single heap allocation: header + flexible array member (array of pointers)
    // sizeof(GlyphData) includes room for 1 pointer.
    size_t size = sizeof(GlyphData) + (glyphCount - 1) * sizeof(Glyph*);
    auto result = alloc->makeSizedUnique<GlyphData>(size, glyphCount, PrivateKey{});

    std::fill_n(result->fGlyphPtrs, glyphCount, nullptr);

    return result;
}

void GlyphData::prepareGlyphs(sktext::gpu::GlyphVector& glyphVector,
                              sktext::gpu::StrikeCache* strikeCache) {
    if (fTextStrike == nullptr) {
        SkStrike* strike = glyphVector.strikePromise().strike();
        fTextStrike = TextStrike::GetOrCreate(strikeCache, strike->strikeSpec());

        // Get all the atlas locations for each glyph from the strike cache.
        auto packedIDs = glyphVector.glyphs();
        SkASSERT(static_cast<int>(packedIDs.size()) == fGlyphCount);

        for (int i = 0; i < fGlyphCount; i++) {
            fGlyphPtrs[i] = fTextStrike->getGlyph(packedIDs[i]);
        }

        // This must be pinned for the Atlas filling to work.
        strike->verifyPinnedStrike();

        // Drop the ref to the strike so that it can be purged if needed.
        glyphVector.strikePromise().resetStrike();
    }
}

std::tuple<bool, int> GlyphData::regenerateAtlas(int begin,
                                                 int end,
                                                 sktext::gpu::GlyphVector& glyphVector,
                                                 MaskFormat maskFormat,
                                                 int srcPadding,
                                                 Recorder* recorder) {
    auto atlasManager = recorder->priv().atlasProvider()->textAtlasManager();
    auto tokenTracker = recorder->priv().tokenTracker();

    // TODO: this is not a great place for this -- need a better way to init atlases when needed
    unsigned int numActiveProxies;
    const sk_sp<TextureProxy>* proxies = atlasManager->getProxies(maskFormat, &numActiveProxies);
    if (!proxies) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return {false, 0};
    }

    uint64_t currentAtlasGen = atlasManager->atlasGeneration(maskFormat);

    this->prepareGlyphs(glyphVector, recorder->priv().strikeCache());

    if (fAtlasGeneration != currentAtlasGen) {
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls.
        fBulkUseUpdater.reset();

        SkBulkGlyphMetricsAndImages metricsAndImages{fTextStrike->strikeSpec()};

        // Update the atlas information in the GrStrike.
        int glyphsPlacedInAtlas = 0;
        bool success = true;
        for (int i = begin; i < end; i++) {
            Glyph* gpuGlyph = fGlyphPtrs[i];
            SkASSERT(gpuGlyph != nullptr);

            if (!atlasManager->hasGlyph(maskFormat, gpuGlyph)) {
                const SkGlyph& skGlyph = *metricsAndImages.glyph(gpuGlyph->fPackedID);
                auto code = atlasManager->addGlyphToAtlas(skGlyph, gpuGlyph, srcPadding);
                if (code != DrawAtlas::ErrorCode::kSucceeded) {
                    success = code != DrawAtlas::ErrorCode::kError;
                    break;
                }
            }
            atlasManager->addGlyphToBulkAndSetUseToken(
                    &fBulkUseUpdater, maskFormat, gpuGlyph, tokenTracker->nextFlushToken());
            glyphsPlacedInAtlas++;
        }

        // Update atlas generation if there are no more glyphs to put in the atlas.
        if (success && begin + glyphsPlacedInAtlas == fGlyphCount) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fAtlasGeneration = atlasManager->atlasGeneration(maskFormat);
        }

        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == fGlyphCount) {
            // The atlas hasn't changed and the texture coordinates are all still valid. Update
            // all the plots used to the new use token.
            atlasManager->setUseTokenBulk(
                    fBulkUseUpdater, tokenTracker->nextFlushToken(), maskFormat);
        }
        return {true, end - begin};
    }
}

namespace {
struct AtlasPt {
    uint16_t u;
    uint16_t v;
};
}

void GlyphData::fillInstanceData(const sktext::gpu::VertexFiller& vf,
                                 DrawWriter* dw,
                                 int offset,
                                 int count,
                                 unsigned short flags,
                                 uint32_t ssboIndex,
                                 SkScalar depth) const {
    auto quadData = [&]() {
        return SkMakeZip(glyphs().subspan(offset, count), vf.topLefts().subspan(offset, count));
    };

    DrawWriter::Instances instances{*dw, {}, {}, 4};
    instances.reserve(count);
    // Need to send width, height, uvPos, xyPos, and strikeToSourceScale
    // pre-transform coords = (s*w*b_x + t_x, s*h*b_y + t_y)
    // where (b_x, b_y) are the vertexID coords
    for (auto [glyph, leftTop] : quadData()) {
        auto [al, at, ar, ab] = glyph->fAtlasLocator.getUVs();
        instances.append(1) << AtlasPt{uint16_t(ar-al), uint16_t(ab-at)}
                            << AtlasPt{uint16_t(al & 0x1fff), at}
                            << leftTop << /*index=*/uint16_t(al >> 13) << flags
                            << 1.0f
                            << depth << ssboIndex;
    }
}

}  // namespace skgpu::graphite
