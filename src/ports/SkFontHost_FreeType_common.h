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
#include "src/core/SkGlyph.h"
#include "src/core/SkScalerContext.h"

class SkCanvas;

// These are forward declared to avoid pimpl but also hide the FreeType implementation.
typedef struct FT_FaceRec_* FT_Face;
typedef signed long FT_Pos;


#ifdef SK_DEBUG
const char* SkTraceFtrGetError(int);
#define SK_TRACEFTR(ERR, MSG, ...) \
    SkDebugf("%s:%d:1: error: 0x%x '%s' " MSG "\n", __FILE__, __LINE__, ERR, \
            SkTraceFtrGetError((int)(ERR)), __VA_ARGS__)
#else
#define SK_TRACEFTR(ERR, ...) do { sk_ignore_unused_variable(ERR); } while (false)
#endif

struct SkScalerContextFTUtils {
    FT_Face                 fFace;      // we are not an owner
    SkScalerContextRec      fRec;

    bool isSubpixel() const {
        return SkToBool(fRec.fFlags & SkScalerContext::kSubpixelPositioning_Flag);
    }

    bool isLinearMetrics() const {
        return SkToBool(fRec.fFlags & SkScalerContext::kLinearMetrics_Flag);
    }

    // See http://freetype.sourceforge.net/freetype2/docs/reference/ft2-bitmap_handling.html#FT_Bitmap_Embolden
    // This value was chosen by eyeballing the result in Firefox and trying to match it.
    static const FT_Pos kBitmapEmboldenStrength = 1 << 6;

    void init(FT_Face, const SkScalerContextRec&);

    bool drawCOLRv0Glyph(const SkGlyph&, uint32_t loadGlyphFlags,
                         SkSpan<SkColor> palette, SkCanvas*) const;
    bool drawCOLRv1Glyph(const SkGlyph&, uint32_t loadGlyphFlags,
                         SkSpan<SkColor> palette, SkCanvas*) const;
    bool drawSVGGlyph(const SkGlyph&, uint32_t loadGlyphFlags,
                      SkSpan<SkColor> palette, SkCanvas*) const;
    void generateGlyphImage(const SkGlyph&, void*, const SkMatrix& bitmapTransform,
                            const SkMaskGamma::PreBlend&) const;
    bool generateGlyphPath(SkPath*) const;

    /** Computes a bounding box for a COLRv1 glyph.
     *
     *  This method may change the configured size and transforms on FT_Face. Make sure to
     *  configure size, matrix and load glyphs as needed after using this function to restore the
     *  state of FT_Face.
     */
    static bool computeColrV1GlyphBoundingBox(FT_Face, SkGlyphID, SkRect* bounds);

private:
    bool generateFacePath(SkGlyphID, uint32_t loadGlyphFlags, SkPath*) const;
};

#endif // SKFONTHOST_FREETYPE_COMMON_H_
