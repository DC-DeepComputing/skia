/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Encoding.h"
#include "include/core/SkPaint.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkUniformData.h"
#include "src/core/SkXfermodePriv.h"
#include "src/shaders/SkColorFilterShader.h"
#include "src/shaders/SkShaderBase.h"

static bool changes_alpha(const SkPaint& paint) {
    SkColorFilter* cf = paint.getColorFilter();
    return cf && !as_CFB(cf)->isAlphaUnchanged();
}

bool SkPaintPriv::Overwrites(const SkPaint* paint, ShaderOverrideOpacity overrideOpacity) {
    if (!paint) {
        // No paint means we default to SRC_OVER, so we overwrite iff our shader-override
        // is opaque, or we don't have one.
        return overrideOpacity != kNotOpaque_ShaderOverrideOpacity;
    }

    SkXfermode::SrcColorOpacity opacityType = SkXfermode::kUnknown_SrcColorOpacity;

    if (!changes_alpha(*paint)) {
        const unsigned paintAlpha = paint->getAlpha();
        if (0xff == paintAlpha && overrideOpacity != kNotOpaque_ShaderOverrideOpacity &&
            (!paint->getShader() || paint->getShader()->isOpaque()))
        {
            opacityType = SkXfermode::kOpaque_SrcColorOpacity;
        } else if (0 == paintAlpha) {
            if (overrideOpacity == kNone_ShaderOverrideOpacity && !paint->getShader()) {
                opacityType = SkXfermode::kTransparentBlack_SrcColorOpacity;
            } else {
                opacityType = SkXfermode::kTransparentAlpha_SrcColorOpacity;
            }
        }
    }

    const auto bm = paint->asBlendMode();
    if (!bm) {
        return false;   // don't know for sure, so we play it safe and return false.
    }
    return SkXfermode::IsOpaque(bm.value(), opacityType);
}

bool SkPaintPriv::ShouldDither(const SkPaint& p, SkColorType dstCT) {
    // The paint dither flag can veto.
    if (!p.isDither()) {
        return false;
    }

    // We always dither 565 or 4444 when requested.
    if (dstCT == kRGB_565_SkColorType || dstCT == kARGB_4444_SkColorType) {
        return true;
    }

    // Otherwise, dither is only needed for non-const paints.
    return p.getImageFilter() || p.getMaskFilter() ||
           (p.getShader() && !as_SB(p.getShader())->isConstant());
}

// return true if the paint is just a single color (i.e. not a shader). If its
// a shader, then we can't compute a const luminance for it :(
static bool just_a_color(const SkPaint& paint, SkColor* color) {
    SkColor c = paint.getColor();

    const auto* shader = as_SB(paint.getShader());
    if (shader && !shader->asLuminanceColor(&c)) {
        return false;
    }
    if (paint.getColorFilter()) {
        c = paint.getColorFilter()->filterColor(c);
    }
    if (color) {
        *color = c;
    }
    return true;
}

SkColor SkPaintPriv::ComputeLuminanceColor(const SkPaint& paint) {
    SkColor c;
    if (!just_a_color(paint, &c)) {
        c = SkColorSetRGB(0x7F, 0x80, 0x7F);
    }
    return c;
}

void SkPaintPriv::RemoveColorFilter(SkPaint* p, SkColorSpace* dstCS) {
    if (SkColorFilter* filter = p->getColorFilter()) {
        if (SkShader* shader = p->getShader()) {
            // SkColorFilterShader will modulate the shader color by paint alpha
            // before applying the filter, so we'll reset it to opaque.
            p->setShader(sk_make_sp<SkColorFilterShader>(sk_ref_sp(shader),
                                                         p->getAlphaf(),
                                                         sk_ref_sp(filter)));
            p->setAlphaf(1.0f);
        } else {
            p->setColor(filter->filterColor4f(p->getColor4f(), sk_srgb_singleton(), dstCS), dstCS);
        }
        p->setColorFilter(nullptr);
    }
}

SkScalar SkPaintPriv::ComputeResScaleForStroking(const SkMatrix& matrix) {
    // Not sure how to handle perspective differently, so we just don't try (yet)
    SkScalar sx = SkPoint::Length(matrix[SkMatrix::kMScaleX], matrix[SkMatrix::kMSkewY]);
    SkScalar sy = SkPoint::Length(matrix[SkMatrix::kMSkewX],  matrix[SkMatrix::kMScaleY]);
    if (SkScalarsAreFinite(sx, sy)) {
        SkScalar scale = std::max(sx, sy);
        if (scale > 0) {
            return scale;
        }
    }
    return 1;
}

/*
 * So, if we ignore pre-compilation, we want to be able to slurp in an SkPaint and end up
 * with either graphite-specific SkSL or ganesh-specific SkSL. The graphite-version is what
 * we really want but, we're stuck, short term, reusing the Ganesh SkSL from the FPs since it
 * is expensive to write all the uber shader SkSL. Note that there may also be a third case where
 * we can get SkVM-specific SkSL - since the raster backend may make different decisions for
 * efficiency.
 *
 * The idea is that the SkPaint and all the things that make it up can generate a set of codes that
 * have a 1 to 1 mapping to the SkSL fragments needed. That compressed form can then be used
 * as a unique key to represent the corresponding SkSL and can be expanded into it. (Ideally
 * the expansion would be just stitching together the SkSL fragments.)
 *
 * The invariant here is that, for every SkPaint the SkCanvas sees we should be able to pre-generate
 * and cache the SkSL for its VS and FS (ignoring ImageFilters). Then, when we see the actual
 * code in the compilation stage, we can assert that we already saw it.
 *
 * Next steps:
 */

std::vector<SkPaintParamsKey> SkPaintPriv::ToKeys(const SkPaint& paint,
                                                  SkShaderCodeDictionary* dict,
                                                  SkBackend backend,
                                                  std::vector<std::unique_ptr<SkUniformBlock>>* uniformBlocks) {

    std::vector<SkPaintParamsKey> keys;
    std::unique_ptr<SkUniformBlock> tmp = std::make_unique<SkUniformBlock>();

    // TODO: actually split the SkPaint into multiple PaintParams and generate the keys
    // for them separately.
    // TODO: actually collect and return the SkUniformData vector for each PaintParams derived
    // from the SkPaint
    {
        SkPaintParamsKey key;

        if (paint.getShader()) {
            as_SB(paint.getShader())->addToKeyS(dict, backend, &key, tmp.get());
        } else {
            SolidColorShaderBlock::AddToKey(dict, backend, &key, tmp.get(), paint.getColor4f());
        }

        if (paint.getColorFilter()) {
            paint.getColorFilter()->addToKeyCF(dict, backend, &key, tmp.get());
        }

        if (paint.getBlender()) {
            as_BB(paint.getBlender())->addToKeyB(dict, backend, &key, tmp.get());
        } else {
            BlendModeBlock::AddToKey(dict, backend, &key, tmp.get(), SkBlendMode::kSrcOver);
        }

        SkASSERT(key.sizeInBytes() > 0);

        keys.push_back(key);
        uniformBlocks->push_back(std::move(tmp));
    }

    return keys;
}
