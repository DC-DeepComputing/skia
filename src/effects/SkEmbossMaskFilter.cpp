/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/SkEmbossMaskFilter.h"

#include "include/core/SkBlurTypes.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/SkEmbossMask.h"

#if defined(SK_SUPPORT_LEGACY_EMBOSSMASKFILTER)
#include "include/effects/SkBlurMaskFilter.h"
#endif

#include <cstring>

sk_sp<SkMaskFilter> SkEmbossMaskFilter::Make(SkScalar blurSigma, const Light& light) {
    if (!SkIsFinite(blurSigma) || blurSigma <= 0) {
        return nullptr;
    }

    SkPoint3 lightDir{light.fDirection[0], light.fDirection[1], light.fDirection[2]};
    if (!lightDir.normalize()) {
        return nullptr;
    }
    Light newLight = light;
    newLight.fDirection[0] = lightDir.x();
    newLight.fDirection[1] = lightDir.y();
    newLight.fDirection[2] = lightDir.z();

    return sk_sp<SkMaskFilter>(new SkEmbossMaskFilter(blurSigma, newLight));
}

#ifdef SK_SUPPORT_LEGACY_EMBOSSMASKFILTER
sk_sp<SkMaskFilter> SkBlurMaskFilter::MakeEmboss(SkScalar blurSigma, const SkScalar direction[3],
                                                 SkScalar ambient, SkScalar specular) {
    if (direction == nullptr) {
        return nullptr;
    }

    SkEmbossMaskFilter::Light   light;

    memcpy(light.fDirection, direction, sizeof(light.fDirection));
    // ambient should be 0...1 as a scalar
    light.fAmbient = SkUnitScalarClampToByte(ambient);
    // specular should be 0..15.99 as a scalar
    static const SkScalar kSpecularMultiplier = SkIntToScalar(255) / 16;
    light.fSpecular = static_cast<U8CPU>(SkTPin(specular, 0.0f, 16.0f) * kSpecularMultiplier + 0.5);

    return SkEmbossMaskFilter::Make(blurSigma, light);
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkEmbossMaskFilter::SkEmbossMaskFilter(SkScalar blurSigma, const Light& light)
    : fLight(light), fBlurSigma(blurSigma)
{
    SkASSERT(fBlurSigma > 0);
    SkASSERT(SkIsFinite(fLight.fDirection, 3));
}

SkMask::Format SkEmbossMaskFilter::getFormat() const {
    return SkMask::k3D_Format;
}

bool SkEmbossMaskFilter::filterMask(SkMaskBuilder* dst, const SkMask& src,
                                    const SkMatrix& matrix, SkIPoint* margin) const {
    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

    SkScalar sigma = matrix.mapRadius(fBlurSigma);

    if (!SkBlurMask::BoxBlur(dst, src, sigma, kInner_SkBlurStyle)) {
        return false;
    }

    dst->format() = SkMask::k3D_Format;
    if (margin) {
        margin->set(SkScalarCeilToInt(3*sigma), SkScalarCeilToInt(3*sigma));
    }

    if (src.fImage == nullptr) {
        return true;
    }

    // create a larger buffer for the other two channels (should force fBlur to do this for us)

    {
        uint8_t* alphaPlane = dst->image();
        size_t totalSize = dst->computeTotalImageSize();
        if (totalSize == 0) {
            return false;  // too big to allocate, abort
        }
        size_t planeSize = dst->computeImageSize();
        SkASSERT(planeSize != 0);  // if totalSize didn't overflow, this can't either
        dst->image() = SkMaskBuilder::AllocImage(totalSize);
        memcpy(dst->image(), alphaPlane, planeSize);
        SkMaskBuilder::FreeImage(alphaPlane);
    }

    // run the light direction through the matrix...
    Light   light = fLight;
    matrix.mapVectors((SkVector*)(void*)light.fDirection,
                      (SkVector*)(void*)fLight.fDirection, 1);

    // now restore the length of the XY component
    // cast to SkVector so we can call setLength (this double cast silences alias warnings)
    SkVector* vec = (SkVector*)(void*)light.fDirection;
    vec->setLength(light.fDirection[0],
                   light.fDirection[1],
                   SkPoint::Length(fLight.fDirection[0], fLight.fDirection[1]));

    SkEmbossMask::Emboss(dst, light);

    // restore original alpha
    memcpy(dst->image(), src.fImage, src.computeImageSize());

    return true;
}

sk_sp<SkFlattenable> SkEmbossMaskFilter::CreateProc(SkReadBuffer& buffer) {
    Light light;
    if (buffer.readByteArray(&light, sizeof(Light))) {
        light.fPad = 0; // for the font-cache lookup to be clean
        const SkScalar sigma = buffer.readScalar();
        return Make(sigma, light);
    }
    return nullptr;
}

void SkEmbossMaskFilter::flatten(SkWriteBuffer& buffer) const {
    Light tmpLight = fLight;
    tmpLight.fPad = 0;    // for the font-cache lookup to be clean
    buffer.writeByteArray(&tmpLight, sizeof(tmpLight));
    buffer.writeScalar(fBlurSigma);
}

bool SkEmbossMaskFilter::asImageFilter(const SkMatrix& ctm,
                                       const SkPaint& paint,
                                       sk_sp<SkImageFilter>& dstImageFilter) const {
    dstImageFilter.reset();
    sk_sp<SkImageFilter> blend = SkImageFilters::Blur(fBlurSigma, fBlurSigma, nullptr);
    sk_sp<SkImageFilter> coverageMask = SkImageFilters::Blend(SkBlendMode::kDstIn, nullptr, std::move(blend));
    //note:not optimal but necessary for this shader which
    // sk_sp<SkImageFilter> srcColor = SkImageFilters::Shader(SkShaders::Color(paint.getColor()));
    // sk_sp<SkImageFilter> srcShader = SkImageFilters::Compose(SkImageFilters::Shader(paint.refShader()), srcColor);
    // sk_sp<SkImageFilter> nullptr = SkImageFilters::ColorFilter(paint.refColorFilter(), srcShader);

     // run the light direction through the matrix...
     Light   light = fLight;
     ctm.mapVectors((SkVector*)(void*)light.fDirection,
                       (SkVector*)(void*)fLight.fDirection, 1);

     // now restore the length of the XY component
     // cast to SkVector so we can call setLength (this double cast silences alias warnings)
     SkVector* vec = (SkVector*)(void*)light.fDirection;
     vec->setLength(light.fDirection[0],
                    light.fDirection[1],
                    SkPoint::Length(fLight.fDirection[0], fLight.fDirection[1]));
    //todo: add comment about what this is
    SkPoint3 diffuseDirection = SkPoint3::Make(-fLight.fDirection[0], -fLight.fDirection[1], fLight.fDirection[2]);
    if (!diffuseDirection.normalize()) {
        return true;
    }
    const int surfaceScale = 32;

    sk_sp<SkImageFilter> diffuse = SkImageFilters::DistantLitDiffuse(diffuseDirection,
                                                                     SK_ColorWHITE,
                                                                     surfaceScale,
                                                                     1,
                                                                     coverageMask);

    //todo: this can be an shader?
    float ambientf = static_cast<float>(fLight.fAmbient);
    uint8_t rgbTable[256];
    uint8_t alphaTable[256];
    for (int i = 0; i < 256; ++i) {
        rgbTable[i] = (i * ambientf) / 255;
        alphaTable[i] = i;
    }
    sk_sp<SkColorFilter> ambientCF = SkColorFilters::TableARGB(alphaTable, rgbTable, rgbTable, rgbTable);
    sk_sp<SkImageFilter> ambient = SkImageFilters::ColorFilter(ambientCF, nullptr);

    sk_sp<SkImageFilter> ambientdiffuse = SkImageFilters::Blend(SkBlendMode::kPlus, diffuse, ambient);
    sk_sp<SkImageFilter> ambientdiffuseBlend = SkImageFilters::Blend(SkBlendMode::kModulate, nullptr, ambientdiffuse);

    //todo: add comment about what this is
    //todo: change these numbers?
    SkPoint3 specularDirection = SkPoint3::Make(-fLight.fDirection[0], -fLight.fDirection[1], -fLight.fDirection[2]);
    sk_sp<SkImageFilter> specular = SkImageFilters::DistantLitSpecular(specularDirection,
                                                                       SK_ColorWHITE,
                                                                       32,
                                                                       1,
                                                                       (fLight.fSpecular >> 4),
                                                                       coverageMask);
    sk_sp<SkImageFilter> finalFilter = SkImageFilters::Blend(SkBlendMode::kPlus, ambientdiffuseBlend, specular);

    dstImageFilter =  sk_sp<SkImageFilter>(SkImageFilters::Blend(SkBlendMode::kDstIn, std::move(finalFilter), nullptr).release());
    return true;
}
