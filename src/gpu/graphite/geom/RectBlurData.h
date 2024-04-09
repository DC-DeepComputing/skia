/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_RectBlurData_DEFINED
#define skgpu_graphite_geom_RectBlurData_DEFINED

#include "include/core/SkMatrix.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Rect.h"

#include <optional>

class SkMatrix;
class SkRRect;

namespace skgpu::graphite {

class Recorder;

/**
 * RectBlurData holds the shader inputs used to do an analytic blur over rects, rrects, or circles.
 */
class RectBlurData {
public:
    enum class ShapeType {
        kRect,
        kRRect,
        kCircle,
    };

    static_assert(0 == static_cast<int>(ShapeType::kRect),
                  "Blur shader code depends on RectBlurData::ShapeType");
    static_assert(1 == static_cast<int>(ShapeType::kRRect),
                  "Blur shader code depends on RectBlurData::ShapeType");
    static_assert(2 == static_cast<int>(ShapeType::kCircle),
                  "Blur shader code depends on RectBlurData::ShapeType");

    RectBlurData() = delete;

    static std::optional<RectBlurData> Make(Recorder*,
                                            const SkMatrix& localToDevice,
                                            float transformedSigma,
                                            const SkRRect& srcRRect);

    const Rect& bounds() const { return fBounds; }
    const SkMatrix& deviceToScaledShape() const { return fDevToScaledShape; }
    const Rect& shapeData() const { return fShapeData; }
    ShapeType shapeType() const { return fShapeType; }
    bool isFast() const { return fIsFast; }
    float invSixSigma() const { return fInvSixSigma; }
    sk_sp<TextureProxy> refProxy() const { return fProxy; }

private:
    RectBlurData(const Rect& bounds,
                 const SkMatrix& devToScaledShape,
                 const Rect& shapeData,
                 ShapeType shapeType,
                 bool isFast,
                 float invSixSigma,
                 sk_sp<TextureProxy> proxy)
            : fBounds(bounds)
            , fDevToScaledShape(devToScaledShape)
            , fShapeData(shapeData)
            , fShapeType(shapeType)
            , fIsFast(isFast)
            , fInvSixSigma(invSixSigma)
            , fProxy(std::move(proxy)) {}

    static std::optional<RectBlurData> MakeRect(Recorder*,
                                                const SkMatrix& localToDevice,
                                                float transformedSigma,
                                                const SkRect& srcRect);

    Rect fBounds;
    SkMatrix fDevToScaledShape;
    Rect fShapeData;
    ShapeType fShapeType;
    bool fIsFast;
    float fInvSixSigma;
    sk_sp<TextureProxy> fProxy;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_RectBlurData_DEFINED
