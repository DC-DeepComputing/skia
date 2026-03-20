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
#include "src/gpu/graphite/text/TextAtlasManager.h"
#include "src/gpu/graphite/text/TextStrike.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/VertexFiller.h"

using MaskFormat = skgpu::MaskFormat;

namespace skgpu::graphite {

GlyphData::GlyphData(sk_sp<TextStrike> strike) : fTextStrike{std::move(strike)} {}

GlyphData::~GlyphData() = default;

Glyph GlyphData::makeGlyphFromID(SkPackedGlyphID id, MaskFormat format) {
    return Glyph{fTextStrike->getGlyph(id, format)};
}

std::tuple<bool, int> GlyphData::regenerateAtlas(int begin,
                                                 int end,
                                                 sktext::gpu::GlyphVector& glyphVector,
                                                 MaskFormat maskFormat,
                                                 int srcPadding,
                                                 Recorder* recorder) {
    SkASSERT(glyphVector.hasBackendData());

    auto atlasManager = recorder->priv().atlasProvider()->textAtlasManager();
    auto tokenTracker = recorder->priv().tokenTracker();

    // TODO: this is not a great place for this -- need a better way to init atlases when needed
    unsigned int numActiveProxies;
    const sk_sp<TextureProxy>* proxies = atlasManager->getProxies(maskFormat, &numActiveProxies);
    if (!proxies) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return {false, 0};
    }

    SkSpan<const Glyph> glyphs = glyphVector.accessBackendGlyphs<Glyph>();

    if (fLastKnownGoodEvictionCounter != atlasManager->evictionCounter(maskFormat)) {
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls.
        fBulkUseUpdater.reset();

        SkBulkGlyphMetricsAndImages metricsAndImages{fTextStrike->strikeSpec()};

        // Update the atlas information in the GrStrike.
        int glyphsPlacedInAtlas = 0;
        for (int i = begin; i < end; i++) {
            SkASSERT(glyphs[i].entry().fGlyphEntryKey.fFormat == maskFormat);
            if (!atlasManager->hasGlyph(maskFormat, glyphs[i].entry())) {
                const SkGlyph& skGlyph = *metricsAndImages.glyph(glyphs[i].packedID());
                switch (atlasManager->addGlyphToAtlas(skGlyph, &glyphs[i].entry(), srcPadding)) {
                    case TextAtlasManager::AddGlyphResult::kSuccess:
                        break;
                    case TextAtlasManager::AddGlyphResult::kFull:
                        return {true, glyphsPlacedInAtlas};
                    case TextAtlasManager::AddGlyphResult::kError:
                        return {false, glyphsPlacedInAtlas};
                }
            }
            atlasManager->addGlyphToBulkAndSetUseToken(&fBulkUseUpdater,
                                                       maskFormat,
                                                       glyphs[i].entry(),
                                                       tokenTracker->nextFlushToken());
            glyphsPlacedInAtlas++;
        }

        SkASSERT(begin + glyphsPlacedInAtlas == SkToInt(glyphs.size()));

        // Update eviction counter as there are no more glyphs to put in the atlas.
        // Need to get the freshest value from the atlas because  adding glyphs may
        // have changed it.
        fLastKnownGoodEvictionCounter = atlasManager->evictionCounter(maskFormat);

        return {true, glyphsPlacedInAtlas};
    }

#if defined(SK_DEBUG)
    for (size_t i = 0; i < glyphs.size(); ++i) {
        const Glyph& gpuGlyph = glyphs[i];
        SkASSERT(atlasManager->hasGlyph(maskFormat, gpuGlyph.entry()));
    }
#endif

    // The atlas hasn't changed, so our texture coordinates are still valid.
    if (end == glyphVector.glyphCount()) {
        // The atlas hasn't changed and the texture coordinates are all still valid. Update
        // all the plots used to the new use token.
        atlasManager->setUseTokenBulk(fBulkUseUpdater, tokenTracker->nextFlushToken(), maskFormat);
    }
    return {true, end - begin};
}

namespace {
struct AtlasPt {
    uint16_t u;
    uint16_t v;
};
}

void GlyphData::fillInstanceData(const sktext::gpu::VertexFiller& vf,
                                 SkSpan<const Glyph> glyphs,
                                 DrawWriter* dw,
                                 int offset,
                                 int count,
                                 unsigned short flags,
                                 uint32_t ssboIndex,
                                 SkScalar depth) const {
    auto quadData = [&]() {
        return SkMakeZip(glyphs.subspan(offset, count), vf.topLefts().subspan(offset, count));
    };

    DrawWriter::Instances instances{*dw, {}, {}, 4};
    instances.reserve(count);
    // Need to send width, height, uvPos, xyPos, and strikeToSourceScale
    // pre-transform coords = (s*w*b_x + t_x, s*h*b_y + t_y)
    // where (b_x, b_y) are the vertexID coords
    for (auto [glyph, leftTop] : quadData()) {
        uint16_t pageIdx = glyph.entry().fPageIdx;
        SkIRect r        = glyph.entry().fRect;
        auto    [w, h]   = r.size();
        auto    [l, t]   = r.topLeft();

        instances.append(1) << AtlasPt{SkToU16(w), SkToU16(h)}
                            << AtlasPt{SkToU16(l), SkToU16(t)}
                            << leftTop << pageIdx << flags
                            << 1.0f
                            << depth << ssboIndex;
    }
}

}  // namespace skgpu::graphite
