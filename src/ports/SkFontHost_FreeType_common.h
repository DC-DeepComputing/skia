/*
 * Copyright 2006-2012 The Android Open Source Project
 * Copyright 2012 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKFONTHOST_FREETYPE_COMMON_H_
#define SKFONTHOST_FREETYPE_COMMON_H_

#include "include/core/SkColor.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "src/core/SkFDot6.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkScalerContext.h"

#include <optional>

class SkCanvas;
class SkPathBuilder;

// These are forward declared to avoid pimpl but also hide the FreeType implementation.
typedef struct FT_FaceRec_* FT_Face;
typedef signed long FT_Pos;


#ifdef SK_DEBUG
const char* SkTraceFtrGetError(int);
#define SK_TRACEFTR(ERR, MSG, ...) \
    SkDebugf("%s:%d:1: error: 0x%x '%s' " MSG "\n", __FILE__, __LINE__, (uint32_t)ERR, \
            SkTraceFtrGetError((int)(ERR)), __VA_ARGS__)
#else
#define SK_TRACEFTR(ERR, ...) do { sk_ignore_unused_variable(ERR); } while (false)
#endif

struct SkScalerContextFTUtils {
    SkColor                 fForegroundColor;
    SkScalerContext::Flags  fFlags;

    using LoadGlyphFlags = uint32_t;

    void init(SkColor fgColor, SkScalerContext::Flags);

    bool isSubpixel() const {
        return SkToBool(fFlags & SkScalerContext::kSubpixelPositioning_Flag);
    }

    bool isLinearMetrics() const {
        return SkToBool(fFlags & SkScalerContext::kLinearMetrics_Flag);
    }

    bool drawCOLRv0Glyph(FT_Face, const SkGlyph&, LoadGlyphFlags,
                         SkSpan<SkColor> palette, SkCanvas*) const;
    bool drawCOLRv1Glyph(FT_Face, const SkGlyph&, LoadGlyphFlags,
                         SkSpan<SkColor> palette, SkCanvas*) const;
    bool drawSVGGlyph(FT_Face, const SkGlyph&, LoadGlyphFlags,
                      SkSpan<SkColor> palette, SkCanvas*) const;
    void generateGlyphImage(FT_Face, const SkGlyph&, void*, const SkMatrix& bitmapTransform,
                            const SkMaskGamma::PreBlend&) const;
    bool generateGlyphPath(FT_Face, SkPathBuilder*) const;

    /** Computes a bounding box for a COLRv1 glyph.
     *
     *  This method may change the configured size and transforms on FT_Face. Make sure to
     *  configure size, matrix and load glyphs as needed after using this function to restore the
     *  state of FT_Face.
     */
    static bool computeColrV1GlyphBoundingBox(FT_Face, SkGlyphID, SkRect* bounds);

    struct OutlineShift {
        FT_Pos fX;
        FT_Pos fY;
    };

    /** Computes the translation that applies the subpixel offset (dx, dy) to an outline with
     *  control box [xMin, xMax] x [yMin, yMax] (26.6) and moves the floored left/bottom edge to
     *  the bitmap origin. Returns nullopt if the translated outline cannot be rasterized safely;
     *  the caller then leaves the glyph image blank. No side effects.
     */
    static std::optional<OutlineShift> ComputeOutlineShift(FT_Pos xMin, FT_Pos yMin,
                                                           FT_Pos xMax, FT_Pos yMax,
                                                           SkFDot6 dx, SkFDot6 dy);

private:
    bool generateFacePath(FT_Face, SkGlyphID, LoadGlyphFlags, SkPathBuilder*) const;
};

#endif // SKFONTHOST_FREETYPE_COMMON_H_
