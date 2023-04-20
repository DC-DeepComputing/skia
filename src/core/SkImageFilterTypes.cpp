/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkImageFilterTypes.h"

#include "src/core/SkColorFilterBase.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkRectPriv.h"

// This exists to cover up issues where infinite precision would produce integers but float
// math produces values just larger/smaller than an int and roundOut/In on bounds would produce
// nearly a full pixel error. One such case is crbug.com/1313579 where the caller has produced
// near integer CTM and uses integer crop rects that would grab an extra row/column of the
// input image when using a strict roundOut.
static constexpr float kRoundEpsilon = 1e-3f;

// Both [I]Vectors and Sk[I]Sizes are transformed as non-positioned values, i.e. go through
// mapVectors() not mapPoints().
static SkIVector map_as_vector(int32_t x, int32_t y, const SkMatrix& matrix) {
    SkVector v = SkVector::Make(SkIntToScalar(x), SkIntToScalar(y));
    matrix.mapVectors(&v, 1);
    return SkIVector::Make(SkScalarRoundToInt(v.fX), SkScalarRoundToInt(v.fY));
}

static SkVector map_as_vector(SkScalar x, SkScalar y, const SkMatrix& matrix) {
    SkVector v = SkVector::Make(x, y);
    matrix.mapVectors(&v, 1);
    return v;
}

// If m is epsilon within the form [1 0 tx], this returns true and sets out to [tx, ty]
//                                 [0 1 ty]
//                                 [0 0 1 ]
// TODO: Use this in decomposeCTM() (and possibly extend it to support is_nearly_scale_translate)
// to be a little more forgiving on matrix types during layer configuration.
static bool is_nearly_integer_translation(const skif::LayerSpace<SkMatrix>& m,
                                          skif::LayerSpace<SkIPoint>* out=nullptr) {
    float tx = SkScalarRoundToScalar(sk_ieee_float_divide(m.rc(0,2), m.rc(2,2)));
    float ty = SkScalarRoundToScalar(sk_ieee_float_divide(m.rc(1,2), m.rc(2,2)));
    SkMatrix expected = SkMatrix::MakeAll(1.f, 0.f, tx,
                                          0.f, 1.f, ty,
                                          0.f, 0.f, 1.f);
    for (int i = 0; i < 9; ++i) {
        if (!SkScalarNearlyEqual(expected.get(i), m.get(i), kRoundEpsilon)) {
            return false;
        }
    }

    if (out) {
        *out = skif::LayerSpace<SkIPoint>({(int) tx, (int) ty});
    }
    return true;
}

static SkRect map_rect(const SkMatrix& matrix, const SkRect& rect) {
    if (rect.isEmpty()) {
        return SkRect::MakeEmpty();
    }
    return matrix.mapRect(rect);
}

static SkIRect map_rect(const SkMatrix& matrix, const SkIRect& rect) {
    if (rect.isEmpty()) {
        return SkIRect::MakeEmpty();
    }
    // Unfortunately, there is a range of integer values such that we have 1px precision as an int,
    // but less precision as a float. This can lead to non-empty SkIRects becoming empty simply
    // because of float casting. If we're already dealing with a float rect or having a float
    // output, that's what we're stuck with; but if we are starting form an irect and desiring an
    // SkIRect output, we go through efforts to preserve the 1px precision for simple transforms.
    if (matrix.isScaleTranslate()) {
        double l = (double)matrix.getScaleX()*rect.fLeft   + (double)matrix.getTranslateX();
        double r = (double)matrix.getScaleX()*rect.fRight  + (double)matrix.getTranslateX();
        double t = (double)matrix.getScaleY()*rect.fTop    + (double)matrix.getTranslateY();
        double b = (double)matrix.getScaleY()*rect.fBottom + (double)matrix.getTranslateY();

        return {sk_double_saturate2int(sk_double_floor(std::min(l, r) + kRoundEpsilon)),
                sk_double_saturate2int(sk_double_floor(std::min(t, b) + kRoundEpsilon)),
                sk_double_saturate2int(sk_double_ceil(std::max(l, r)  - kRoundEpsilon)),
                sk_double_saturate2int(sk_double_ceil(std::max(t, b)  - kRoundEpsilon))};
    } else {
        return skif::RoundOut(matrix.mapRect(SkRect::Make(rect)));
    }
}

namespace skif {

SkIRect RoundOut(SkRect r) { return r.makeInset(kRoundEpsilon, kRoundEpsilon).roundOut(); }

SkIRect RoundIn(SkRect r) { return r.makeOutset(kRoundEpsilon, kRoundEpsilon).roundIn(); }

bool Mapping::decomposeCTM(const SkMatrix& ctm, const SkImageFilter* filter,
                           const skif::ParameterSpace<SkPoint>& representativePt) {
    SkMatrix remainder, layer;
    SkSize decomposed;
    using MatrixCapability = SkImageFilter_Base::MatrixCapability;
    MatrixCapability capability =
            filter ? as_IFB(filter)->getCTMCapability() : MatrixCapability::kComplex;
    if (capability == MatrixCapability::kTranslate) {
        // Apply the entire CTM post-filtering
        remainder = ctm;
        layer = SkMatrix::I();
    } else if (ctm.isScaleTranslate() || capability == MatrixCapability::kComplex) {
        // Either layer space can be anything (kComplex) - or - it can be scale+translate, and the
        // ctm is. In both cases, the layer space can be equivalent to device space.
        remainder = SkMatrix::I();
        layer = ctm;
    } else if (ctm.decomposeScale(&decomposed, &remainder)) {
        // This case implies some amount of sampling post-filtering, either due to skew or rotation
        // in the original matrix. As such, keep the layer matrix as simple as possible.
        layer = SkMatrix::Scale(decomposed.fWidth, decomposed.fHeight);
    } else {
        // Perspective, which has a non-uniform scaling effect on the filter. Pick a single scale
        // factor that best matches where the filter will be evaluated.
        SkScalar scale = SkMatrixPriv::DifferentialAreaScale(ctm, SkPoint(representativePt));
        if (SkScalarIsFinite(scale) && !SkScalarNearlyZero(scale)) {
            // Now take the sqrt to go from an area scale factor to a scaling per X and Y
            // FIXME: It would be nice to be able to choose a non-uniform scale.
            scale = SkScalarSqrt(scale);
        } else {
            // The representative point was behind the W = 0 plane, so don't factor out any scale.
            // NOTE: This makes remainder and layer the same as the MatrixCapability::Translate case
            scale = 1.f;
        }

        remainder = ctm;
        remainder.preScale(SkScalarInvert(scale), SkScalarInvert(scale));
        layer = SkMatrix::Scale(scale, scale);
    }

    SkMatrix invRemainder;
    if (!remainder.invert(&invRemainder)) {
        // Under floating point arithmetic, it's possible to decompose an invertible matrix into
        // a scaling matrix and a remainder and have the remainder be non-invertible. Generally
        // when this happens the scale factors are so large and the matrix so ill-conditioned that
        // it's unlikely that any drawing would be reasonable, so failing to make a layer is okay.
        return false;
    } else {
        fParamToLayerMatrix = layer;
        fLayerToDevMatrix = remainder;
        fDevToLayerMatrix = invRemainder;
        return true;
    }
}

bool Mapping::adjustLayerSpace(const SkMatrix& layer) {
    SkMatrix invLayer;
    if (!layer.invert(&invLayer)) {
        return false;
    }
    fParamToLayerMatrix.postConcat(layer);
    fDevToLayerMatrix.postConcat(layer);
    fLayerToDevMatrix.preConcat(invLayer);
    return true;
}

// Instantiate map specializations for the 6 geometric types used during filtering
template<>
SkRect Mapping::map<SkRect>(const SkRect& geom, const SkMatrix& matrix) {
    return map_rect(matrix, geom);
}

template<>
SkIRect Mapping::map<SkIRect>(const SkIRect& geom, const SkMatrix& matrix) {
    return map_rect(matrix, geom);
}

template<>
SkIPoint Mapping::map<SkIPoint>(const SkIPoint& geom, const SkMatrix& matrix) {
    SkPoint p = SkPoint::Make(SkIntToScalar(geom.fX), SkIntToScalar(geom.fY));
    matrix.mapPoints(&p, 1);
    return SkIPoint::Make(SkScalarRoundToInt(p.fX), SkScalarRoundToInt(p.fY));
}

template<>
SkPoint Mapping::map<SkPoint>(const SkPoint& geom, const SkMatrix& matrix) {
    SkPoint p;
    matrix.mapPoints(&p, &geom, 1);
    return p;
}

template<>
IVector Mapping::map<IVector>(const IVector& geom, const SkMatrix& matrix) {
    return IVector(map_as_vector(geom.fX, geom.fY, matrix));
}

template<>
Vector Mapping::map<Vector>(const Vector& geom, const SkMatrix& matrix) {
    return Vector(map_as_vector(geom.fX, geom.fY, matrix));
}

template<>
SkISize Mapping::map<SkISize>(const SkISize& geom, const SkMatrix& matrix) {
    SkIVector v = map_as_vector(geom.fWidth, geom.fHeight, matrix);
    return SkISize::Make(v.fX, v.fY);
}

template<>
SkSize Mapping::map<SkSize>(const SkSize& geom, const SkMatrix& matrix) {
    SkVector v = map_as_vector(geom.fWidth, geom.fHeight, matrix);
    return SkSize::Make(v.fX, v.fY);
}

template<>
SkMatrix Mapping::map<SkMatrix>(const SkMatrix& m, const SkMatrix& matrix) {
    // If 'matrix' maps from the C1 coord space to the C2 coord space, and 'm' is a transform that
    // operates on, and outputs to, the C1 coord space, we want to return a new matrix that is
    // equivalent to 'm' that operates on and outputs to C2. This is the same as mapping the input
    // from C2 to C1 (matrix^-1), then transforming by 'm', and then mapping from C1 to C2 (matrix).
    SkMatrix inv;
    SkAssertResult(matrix.invert(&inv));
    inv.postConcat(m);
    inv.postConcat(matrix);
    return inv;
}

LayerSpace<SkRect> LayerSpace<SkMatrix>::mapRect(const LayerSpace<SkRect>& r) const {
    return LayerSpace<SkRect>(map_rect(fData, SkRect(r)));
}

LayerSpace<SkIRect> LayerSpace<SkMatrix>::mapRect(const LayerSpace<SkIRect>& r) const {
    return LayerSpace<SkIRect>(map_rect(fData, SkIRect(r)));
}

sk_sp<SkSpecialImage> FilterResult::imageAndOffset(SkIPoint* offset) const {
    auto [image, origin] = this->resolve(fLayerBounds);
    *offset = SkIPoint(origin);
    return image;
}

FilterResult FilterResult::applyCrop(const Context& ctx,
                                     const LayerSpace<SkIRect>& crop) const {
    LayerSpace<SkIRect> tightBounds = crop;
    // TODO(michaelludwig): Intersecting to the target output is only valid when the crop has
    // decal tiling (the only current option).
    if (!fImage || !tightBounds.intersect(ctx.desiredOutput())) {
        // The desired output would be filled with transparent black.
        SkASSERT(!fColorFilter);
        return {};
    }

    if ((!fColorFilter || !as_CFB(fColorFilter)->affectsTransparentBlack()) &&
        is_nearly_integer_translation(fTransform)) {
        // We can lift the crop to earlier in the order of operations and apply it to the image
        // subset directly, which is handled inside this resolve() call.
        // FIXME: resolve() is supposed to apply the color filter, which we don't really want in
        // this case. The crop lifting is easier when we have fSrcRect/fDstRect as properties but
        // maybe we just need to lift the logic for subsetting the special image into a helper function
        // so we can avoid resolve().
        return this->resolve(tightBounds);
    } else {
        // Otherwise cropping is the final operation to the FilterResult's image and can always be
        // applied by adjusting the layer bounds.
        FilterResult restrictedOutput = *this;
        if (!restrictedOutput.fLayerBounds.intersect(tightBounds)) {
            return {};
        }
        return restrictedOutput;
    }
}

FilterResult FilterResult::applyColorFilter(const Context& ctx,
                                            sk_sp<SkColorFilter> colorFilter) const {
    // A null filter is the identity, so it should have been caught during image filter DAG creation
    SkASSERT(colorFilter);

    // Color filters are applied after the transform and image sampling, but before the layer
    // bounds crop. Thus we can compose 'colorFilter' with any previously applied filter regardless
    // of the transform/sample state, we only have to consider the effect of the crop. If
    // 'colorFilter' does not affect transparent black, it can be moved before the crop without
    // changing the "shape" of the resolved image. When 'colorFilter' does affect transparent black,
    // the result fills ctx's desired output. Pixels within the desired output but outside of this
    // image's layer bounds are equal to the output of the 'colorFilter({0,0,0,0})' and no longer
    // transparent. If that region is empty (i.e. the desired output is contained in this image's
    // layer bounds), then the transparency-affecting 'colorFilter' can also be moved before the
    // crop and composed with any prior color filter on the FilterResult.
    if (as_CFB(colorFilter)->affectsTransparentBlack()) {
        if (!fImage) {
            // The input is fully transparent black so we need to fill the desired output with
            // 'colorFilter', which means we don't actually have any more color filter on the returned
            // filter result since we can't defer the null image evaluation at this point.
        }
        if (!fLayerBounds.contains(ctx.desiredOutput())) {
            // FIXME we don't need to resolve to entire layer bounds, we only need the desired output
            // intersection with layer bounds
            // That means we need to produce an image that preserves existing layer bounds, but that
            // can be tighter than the next desired output since we can defer the ATB color filter.
            // We just have to ensure that its layer bounds are *larger* than the transformed image bound
            return this->resolve(fLayerBounds);
        }
        // else the filter's impacts on the transparency outside fLayerBounds will not be visible,
        // so fall through to composing the filter and avoiding an intermediate renderpass.
    } else {
        if (!fImage) {
            return {}; // A transparent black image remains transparent after 'colorFilter'
        }
    }

    // If we got here, we can compose the new color filter with any previous filter and restrict
    // output to the desired output bounds.
    if (fColorFilter) {
        colorFilter = SkColorFilters::Compose(fColorFilter, std::move(colorFilter));
    }
    FilterResult filtered = *this;
    filtered.fColorFilter = std::move(colorFilter);
    if (!filtered.fLayerBounds.intersect(ctx.desiredOutput())) {
        return {};
    }
    return filtered;
}

static bool compatible_sampling(const SkSamplingOptions& currentSampling,
                                bool currentXformWontAffectNearest,
                                SkSamplingOptions* nextSampling,
                                bool nextXformWontAffectNearest) {
    // Both transforms could perform non-trivial sampling, but if they are similar enough we
    // assume performing one non-trivial sampling operation with the concatenated transform will
    // not be visually distinguishable from sampling twice.
    // TODO(michaelludwig): For now ignore mipmap policy, SkSpecialImages are not supposed to be
    // drawn with mipmapping, and the majority of filter steps produce images that are at the
    // proper scale and do not define mip levels. The main exception is the ::Image() filter
    // leaf but that doesn't use this system yet.
    if (currentSampling.isAniso() && nextSampling->isAniso()) {
        // Assume we can get away with one sampling at the highest anisotropy level
        *nextSampling =  SkSamplingOptions::Aniso(std::max(currentSampling.maxAniso,
                                                           nextSampling->maxAniso));
        return true;
    } else if (currentSampling.isAniso() && nextSampling->filter == SkFilterMode::kLinear) {
        // Assume we can get away with the current anisotropic filter since the next is linear
        *nextSampling = currentSampling;
        return true;
    } else if (nextSampling->isAniso() && currentSampling.filter == SkFilterMode::kLinear) {
        // Mirror of the above, assume we can just get away with next's anisotropic filter
        return true;
    } else if (currentSampling.useCubic && (nextSampling->filter == SkFilterMode::kLinear ||
                                            (nextSampling->useCubic &&
                                             currentSampling.cubic.B == nextSampling->cubic.B &&
                                             currentSampling.cubic.C == nextSampling->cubic.C))) {
        // Assume we can get away with the current bicubic filter, since the next is the same
        // or a bilerp that can be upgraded.
        *nextSampling = currentSampling;
        return true;
    } else if (nextSampling->useCubic && currentSampling.filter == SkFilterMode::kLinear) {
        // Mirror of the above, assume we can just get away with next's cubic resampler
        return true;
    } else if (currentSampling.filter == SkFilterMode::kLinear &&
               nextSampling->filter == SkFilterMode::kLinear) {
        // Assume we can get away with a single bilerp vs. the two
        return true;
    } else if (nextSampling->filter == SkFilterMode::kNearest && currentXformWontAffectNearest) {
        // The next transform and nearest-neighbor filtering isn't impacted by the current transform
        SkASSERT(currentSampling.filter == SkFilterMode::kLinear);
        return true;
    } else if (currentSampling.filter == SkFilterMode::kNearest && nextXformWontAffectNearest) {
        // The next transform doesn't change the nearest-neighbor filtering of the current transform
        SkASSERT(nextSampling->filter == SkFilterMode::kLinear);
        *nextSampling = currentSampling;
        return true;
    } else {
        // The current or next sampling is nearest neighbor, and will produce visible texels
        // oriented with the current transform; assume this is a desired effect and preserve it.
        return false;
    }
}

FilterResult FilterResult::applyTransform(const Context& ctx,
                                          const LayerSpace<SkMatrix> &transform,
                                          const SkSamplingOptions &sampling) const {
    if (!fImage) {
        // Transformed transparent black remains transparent black.
        SkASSERT(!fColorFilter);
        return {};
    }

    // Extract the sampling options that matter based on the current and next transforms.
    // We make sure the new sampling is bilerp (default) if the new transform doesn't matter
    // (and assert that the current is bilerp if its transform didn't matter). Bilerp can be
    // maximally combined, so simplifies the logic in compatible_sampling().
    const bool currentXformIsInteger = is_nearly_integer_translation(fTransform);
    const bool nextXformIsInteger = is_nearly_integer_translation(transform);

    SkASSERT(!currentXformIsInteger || fSamplingOptions == kDefaultSampling);
    SkSamplingOptions nextSampling = nextXformIsInteger ? kDefaultSampling : sampling;

    // Determine if the image is being visibly cropped by the layer bounds, in which case we can't
    // merge this transform with any previous transform (unless the new transform is an integer
    // translation).
    // FIXME can we move transforms in front of ATB in what cases? all cases(///unlikely) or just
    // the same cases as when we can move in front of a crop? Should be fine with the quadContainsRect check
    bool isCropped = false;
    const bool fillsLayerBounds = fColorFilter && as_CFB(fColorFilter)->affectsTransparentBlack();
    if (!nextXformIsInteger || fillsLayerBounds) {
        LayerSpace<SkIRect> imageBounds = fTransform.mapRect(
                    LayerSpace<SkIRect>{SkIRect::MakeWH(fImage->width(), fImage->height())});
        if (fillsLayerBounds || !fLayerBounds.contains(imageBounds)) {
            // Layer bounds restricts the mapped image, but it may not be visible.
            isCropped = !SkRectPriv::QuadContainsRect(SkMatrix(transform),
                                                      SkIRect(fLayerBounds),
                                                      SkIRect(ctx.desiredOutput()));
        }
    }

    FilterResult transformed;
    if (!isCropped && compatible_sampling(fSamplingOptions, currentXformIsInteger,
                                          &nextSampling, nextXformIsInteger)) {
        // We can concat transforms and 'nextSampling' will be either fSamplingOptions,
        // sampling, or a merged combination depending on the two transforms in play.
        transformed = *this;
    } else {
        // We'll have to resolve this FilterResult first before 'transform' and 'sampling' can be
        // correctly evaluated. 'nextSampling' will always be 'sampling'.
        // FIXME is it worth restricting fLayerBounds with the 'inverse(transform).mapRect(desiredOutput)'?
        transformed = this->resolve(fLayerBounds);
    }

    transformed.concatTransform(transform, nextSampling, ctx.desiredOutput());
    if (transformed.layerBounds().isEmpty()) {
        return {};
    } else {
        return transformed;
    }
}

// FIXME just remove this function and fold it into applyTransform's logic
void FilterResult::concatTransform(const LayerSpace<SkMatrix>& transform,
                                   const SkSamplingOptions& newSampling,
                                   const LayerSpace<SkIRect>& desiredOutput) {
    if (!fImage) {
        // Under normal circumstances, concatTransform() will only be called when we have an image,
        // but if resolve() fails to make a special surface, we may end up here at which point
        // doing nothing further is appropriate.
        return;
    }
    fSamplingOptions = newSampling;
    fTransform.postConcat(transform);
    // Rebuild the layer bounds and then restrict to the current desired output. The original value
    // of fLayerBounds includes the image mapped by the original fTransform as well as any
    // accumulated soft crops from desired outputs of prior stages. To prevent discarding that info,
    // we map fLayerBounds by the additional transform, instead of re-mapping the image bounds.
    // FIXME what logic has to happen here if we have a color filte ror an ATB color filter?
    // should it be safe since if we have colored boundary from ATB filtering at the layer edge,
    // we figured out that the mapped layer bounds are outside the desired output, so mapping it and
    // intersecting it with the desired output brings us to the layer bounds.
    fLayerBounds = transform.mapRect(fLayerBounds);
    if (!fLayerBounds.intersect(desiredOutput)) {
        // The transformed output doesn't touch the desired, so it would just be transparent black.
        // TODO: This intersection only applies when the tile mode is kDecal.
        fLayerBounds = LayerSpace<SkIRect>::Empty();
    }
}

std::pair<sk_sp<SkSpecialImage>, LayerSpace<SkIPoint>> FilterResult::resolve(
        LayerSpace<SkIRect> dstBounds) const {
    // TODO(michaelludwig): Only valid for kDecal, although kClamp would only need 1 extra
    // pixel of padding so some restriction could happen. We also should skip the intersection if
    // we need to include transparent black pixels.
    if (!fImage || !dstBounds.intersect(fLayerBounds)) {
        return {nullptr, {}};
    }

    // TODO: This logic to skip a draw will also need to account for the tile mode, but we can
    // always restrict to the intersection of dstBounds and the image's subset since we are
    // currently always decal sampling.
    // TODO(michaelludwig): If we get to the point where all filter results track bounds in
    // floating point, then we can extend this case to any S+T transform.
    LayerSpace<SkIPoint> origin;
    if (is_nearly_integer_translation(fTransform, &origin)) {
        LayerSpace<SkIRect> imageBounds(SkIRect::MakeXYWH(origin.x(), origin.y(),
                                                          fImage->width(), fImage->height()));
        if (!imageBounds.intersect(dstBounds)) {
            return {nullptr, {}};
        }

        // Offset the image subset directly to avoid issues negating (origin). With the prior
        // intersection (bounds - origin) will be >= 0, but (bounds + (-origin)) may not, (e.g.
        // origin is INT_MIN).
        SkIRect subset = { imageBounds.left() - origin.x(),
                           imageBounds.top() - origin.y(),
                           imageBounds.right() - origin.x(),
                           imageBounds.bottom() - origin.y() };
        SkASSERT(subset.fLeft >= 0 && subset.fTop >= 0 &&
                 subset.fRight <= fImage->width() && subset.fBottom <= fImage->height());

        return {fImage->makeSubset(subset), imageBounds.topLeft()};
    } // else fall through and attempt a draw

    sk_sp<SkSpecialSurface> surface = fImage->makeSurface(fImage->colorType(),
                                                          fImage->getColorSpace(),
                                                          SkISize(dstBounds.size()),
                                                          kPremul_SkAlphaType, {});
    if (!surface) {
        return {nullptr, {}};
    }
    SkCanvas* canvas = surface->getCanvas();
    // skbug.com/5075: GPU-backed special surfaces don't reset their contents.
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->translate(-dstBounds.left(), -dstBounds.top()); // dst's origin adjustment

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setBlendMode(SkBlendMode::kSrc);

    // TODO: When using a tile mode other than kDecal, we'll need to use SkSpecialImage::asShader()
    // and use drawRect(fLayerBounds).
    if (!fLayerBounds.contains(dstBounds)) {
        // We're resolving to a larger than necessary image, so make sure transparency outside of
        // fLayerBounds is preserved.
        // NOTE: This should only happen when the next layer requires processing transparent black.
        canvas->clipIRect(SkIRect(fLayerBounds));
    }
    canvas->concat(SkMatrix(fTransform)); // src's origin is embedded in fTransform
    fImage->draw(canvas, 0.f, 0.f, fSamplingOptions, &paint);

    return {surface->makeImageSnapshot(), dstBounds.topLeft()};
}

} // end namespace skif
