/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextOptions_DEFINED
#define skgpu_graphite_ContextOptions_DEFINED

#include "include/gpu/ShaderErrorHandler.h"

#include <vector>

namespace skgpu::graphite {

struct SK_API ContextOptions {
    enum class Enable {
        /** Forces an option to be disabled. */
        kNo,
        /** Forces an option to be enabled. */
        kYes,
        /**
         * Uses Skia's default behavior, which may use runtime properties (e.g. driver version).
         */
        kDefault
    };

    using ShaderErrorHandler = skgpu::ShaderErrorHandler;

    ContextOptions() {}

    /**
     * The maximum size of cache textures used for Skia's Glyph cache.
     */
    size_t fGlyphCacheTextureMaximumBytes = 2048 * 1024 * 4;

    /**
     * Below this threshold size in device space distance field fonts won't be used. Distance field
     * fonts don't support hinting which is more important at smaller sizes.
     */
    float fMinDistanceFieldFontSize = 18;

    /**
     * Above this threshold size in device space glyphs are drawn as individual paths.
     */
#if defined(SK_BUILD_FOR_ANDROID)
    float fGlyphsAsPathsFontSize = 384;
#elif defined(SK_BUILD_FOR_MAC)
    float fGlyphsAsPathsFontSize = 256;
#else
    float fGlyphsAsPathsFontSize = 324;
#endif

    /**
     * Can the glyph atlas use multiple textures. If allowed, the each texture's size is bound by
     * fGlypheCacheTextureMaximumBytes.
     */
    Enable fAllowMultipleGlyphCacheTextures = Enable::kDefault;

    /**
     * If present, use this object to report shader compilation failures. If not, report failures
     * via SkDebugf and assert.
     */
    ShaderErrorHandler* fShaderErrorHandler = nullptr;

    /**
     * If true, then add 1 pixel padding to all glyph masks in the atlas to support bi-lerp
     * rendering of all glyphs. This must be set to true to use Slugs.
     */
    #if defined(SK_EXPERIMENTAL_SIMULATE_DRAWGLYPHRUNLIST_WITH_SLUG) || \
        defined(SK_EXPERIMENTAL_SIMULATE_DRAWGLYPHRUNLIST_WITH_SLUG_SERIALIZE) || \
        defined(SK_EXPERIMENTAL_SIMULATE_DRAWGLYPHRUNLIST_WITH_SLUG_STRIKE_SERIALIZE)
    bool fSupportBilerpFromGlyphAtlas = true;
    #else
    bool fSupportBilerpFromGlyphAtlas = false;
    #endif

#if GR_TEST_UTILS
    /**
     * Private options that are only meant for testing within Skia's tools.
     */

    /**
     * Maximum width and height of internal texture atlases.
     */
    int  fMaxTextureAtlasSize = 2048;
#endif
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ContextOptions
