/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGPUContextOptions_DEFINED
#define SkGPUContextOptions_DEFINED

#include "include/core/SkContextOptions.h"
#include "include/core/SkTypes.h"
#include "include/private/SkAPI.h"

#include <cstddef>

namespace skgpu {
class ShaderErrorHandler;
}

struct SK_API SkGPUContextOptions : public SkContextOptions {
    SkGPUContextOptions() = default;

    /**
     * Disables correctness workarounds that are enabled for particular GPUs,
     * OSes, or drivers. This does not affect code path choices that are made
     * for perfomance reasons nor does it override other ContextOption settings.
     */
    bool fDisableDriverCorrectnessWorkarounds = false;

    /**
     * If present, use this object to report shader compilation failures. If
     * not, report failures via SkDebugf and assert.
     */
    skgpu::ShaderErrorHandler* fShaderErrorHandler = nullptr;

    /**
     * Specifies the number of samples Skia should use when performing internal
     * draws with MSAA (hardware capabilities permitting).
     *
     * If <= 1, Skia will disable internal code paths that use multisampling.
     */
    int fInternalMultisampleCount = 4;

    /**
     * The maximum size of cache textures used for Skia's Glyph cache.
     */
    size_t fGlyphCacheTextureMaximumBytes = 2048 * 1024 * 4;

    /**
     * Below this threshold size in device space distance field fonts won't be
     * used. Distance field fonts don't support hinting which is more important
     * at smaller sizes.
     */
    float fMinDistanceFieldFontSize = 18;

    /**
     * Above this threshold size in device space glyphs are drawn as individual
     * paths.
     */
#if defined(SK_BUILD_FOR_ANDROID)
    float fGlyphsAsPathsFontSize = 384;
#elif defined(SK_BUILD_FOR_MAC)
    float fGlyphsAsPathsFontSize = 256;
#else
    float fGlyphsAsPathsFontSize = 324;
#endif

    /**
     * Can the glyph and path atlases use multiple textures. If allowed, each
     * texture's size is bound by fGlyphCacheTextureMaximumBytes (and
     * fMaxPathAtlasTextureSize on Graphite).
     */
    bool fAllowMultipleAtlasTextures = true;

    /**
     * If true, then add 1 pixel padding to all glyph masks in the atlas to
     * support bi-lerp rendering of all glyphs. This must be set to true to use
     * Slugs.
     */
    bool fSupportBilerpFromGlyphAtlas = false;
};

#endif  // SkGPUContextOptions_DEFINED
