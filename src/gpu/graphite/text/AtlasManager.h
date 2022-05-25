/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_AtlasManager_DEFINED
#define skgpu_graphite_AtlasManager_DEFINED

#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawAtlas.h"

namespace sktext::gpu {
class Glyph;
}
class SkGlyph;

namespace skgpu::graphite {

class DrawContext;
class Recorder;

//////////////////////////////////////////////////////////////////////////////////////////////////
/** The AtlasManager manages the lifetime of and access to DrawAtlases.
 *  [*** TODO: It is only available at flush and only via the GrOpFlushState.
 *
 *  This implies that all of the advanced atlasManager functionality (i.e.,
 *  adding glyphs to the atlas) are only available at flush time. ***]
 */
class AtlasManager : public AtlasGenerationCounter {
public:
    AtlasManager(Recorder*,
                 size_t maxTextureBytes,
                 DrawAtlas::AllowMultitexturing,
                 bool supportBilerpAtlas);
    ~AtlasManager();

    // Used to get proxies for test rendering
    const sk_sp<TextureProxy>* getProxies(MaskFormat format,
                                          unsigned int* numActiveProxies) {
        format = this->resolveMaskFormat(format);
        if (this->initAtlas(format)) {
            *numActiveProxies = this->getAtlas(format)->numActivePages();
            return this->getAtlas(format)->getProxies();
        }
        *numActiveProxies = 0;
        return nullptr;
    }

    void freeAll();

    bool hasGlyph(MaskFormat, sktext::gpu::Glyph*);

    DrawAtlas::ErrorCode addGlyphToAtlas(const SkGlyph&,
                                         sktext::gpu::Glyph*,
                                         int srcPadding,
                                         DrawContext*,
                                         Recorder*);

    // To ensure the DrawAtlas does not evict the Glyph Mask from its texture backing store,
    // the client must pass in the current draw token along with the sktext::gpu::Glyph.
    // A BulkUsePlotUpdater is used to manage bulk last use token updating in the Atlas.
    // For convenience, this function will also set the use token for the current glyph if required
    // NOTE: the bulk uploader is only valid if the subrun has a valid atlasGeneration
    void addGlyphToBulkAndSetUseToken(BulkUsePlotUpdater*, MaskFormat,
                                      sktext::gpu::Glyph*, DrawToken);

    void setUseTokenBulk(const BulkUsePlotUpdater& updater,
                         DrawToken token,
                         MaskFormat format) {
        this->getAtlas(format)->setLastUseTokenBulk(updater, token);
    }

    // add to texture atlas that matches this format
    DrawAtlas::ErrorCode addToAtlas(DrawContext*, Recorder*,
                                    skgpu::MaskFormat, int width, int height, const void* image,
                                    skgpu::AtlasLocator*);

    // Some clients may wish to verify the integrity of the texture backing store of the
    // GrDrawOpAtlas. The atlasGeneration returned below is a monotonically increasing number which
    // changes every time something is removed from the texture backing store.
    uint64_t atlasGeneration(skgpu::MaskFormat format) const {
        return this->getAtlas(format)->atlasGeneration();
    }

    // GrOnFlushCallbackObject overrides
    // (don't think we need these, except we need a place to call compact())

//    bool preFlush(GrOnFlushResourceProvider* onFlushRP) override {
//#if GR_TEST_UTILS
//        if (onFlushRP->failFlushTimeCallbacks()) {
//            return false;
//        }
//#endif
//
//        for (int i = 0; i < skgpu::kMaskFormatCount; ++i) {
//            if (fAtlases[i]) {
//                fAtlases[i]->instantiate(onFlushRP);
//            }
//        }
//        return true;
//    }
//
//    void postFlush(GrDeferredUploadToken startTokenForNextFlush) override {
//        for (int i = 0; i < skgpu::kMaskFormatCount; ++i) {
//            if (fAtlases[i]) {
//                fAtlases[i]->compact(startTokenForNextFlush);
//            }
//        }
//    }
//
//    // The AtlasGlyph cache always survives freeGpuResources so we want it to remain in the active
//    // OnFlushCallbackObject list
//    bool retainOnFreeGpuResources() override { return true; }

    ///////////////////////////////////////////////////////////////////////////
    // Functions intended debug only
//#ifdef SK_DEBUG
//    void dump(GrDirectContext*) const;
//#endif

    void setAtlasDimensionsToMinimum_ForTesting();
    void setMaxPages_TestingOnly(uint32_t maxPages);

private:
    bool initAtlas(MaskFormat);
    // Change an expected 565 mask format to 8888 if 565 is not supported (will happen when using
    // Metal on macOS). The actual conversion of the data is handled in get_packed_glyph_image() in
    // StrikeCache.cpp
    MaskFormat resolveMaskFormat(MaskFormat format) const {
        if (MaskFormat::kA565 == format &&
            !fCaps->getDefaultSampledTextureInfo(kRGB_565_SkColorType, 1, Protected::kNo,
                                                   Renderable::kNo).isValid()) {
            format = MaskFormat::kARGB;
        }
        return format;
    }

    // There is a 1:1 mapping between skgpu::MaskFormats and atlas indices
    static int MaskFormatToAtlasIndex(skgpu::MaskFormat format) {
        return static_cast<int>(format);
    }
    static skgpu::MaskFormat AtlasIndexToMaskFormat(int idx) {
        return static_cast<skgpu::MaskFormat>(idx);
    }

    DrawAtlas* getAtlas(skgpu::MaskFormat format) const {
        format = this->resolveMaskFormat(format);
        int atlasIndex = MaskFormatToAtlasIndex(format);
        SkASSERT(fAtlases[atlasIndex]);
        return fAtlases[atlasIndex].get();
    }

    DrawAtlas::AllowMultitexturing fAllowMultitexturing;
    std::unique_ptr<DrawAtlas> fAtlases[kMaskFormatCount];
    static_assert(kMaskFormatCount == 3);
    bool fSupportBilerpAtlas;
    sk_sp<const Caps> fCaps;
    DrawAtlasConfig fAtlasConfig;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_AtlasManager_DEFINED
