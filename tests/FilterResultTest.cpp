/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkImageFilterTypes.h"

using namespace skif;

namespace {

class ApplyAction {
public:
    ApplyAction(const LayerSpace<SkMatrix>& matrix,
                const SkSamplingOptions& sampling,
                bool expectNewImage,
                const SkSamplingOptions& expectedSampling)
            : fType(Type::kTransform)
            , fTransformParams{matrix, sampling}
            , fExpectNewImage(expectNewImage)
            , fExpectedSampling(expectedSampling) {}
    ApplyAction(const LayerSpace<SkIRect>& cropRect,
                bool expectNewImage,
                const SkSamplingOptions& expectedSampling)
            : fType(Type::kCrop)
            , fCropParams{cropRect}
            , fExpectNewImage(expectNewImage)
            , fExpectedSampling(expectedSampling) {}

    // Test-simplified logic for bounds propagation similar to how image filters calculate bounds
    // while evaluating a filter DAG, which is outside of skif::FilterResult's responsibilities.
    LayerSpace<SkIRect> requiredInput(const LayerSpace<SkIRect>& desiredOutput) const {
        switch(fType) {
            case Type::kTransform: {
                LayerSpace<SkMatrix> inverse;
                if (!fTransformParams.fMatrix.invert(&inverse)) {
                    return LayerSpace<SkIRect>::Empty();
                }
                return inverse.mapRect(desiredOutput); }

            case Type::kCrop: {
                auto intersection = fCropParams.fRect;
                if (!intersection.intersect(desiredOutput)) {
                    intersection = LayerSpace<SkIRect>::Empty();
                }
                return intersection; }
        }
        SkUNREACHABLE;
    }

    FilterResult apply(const Context& ctx, const FilterResult& in) const {
        switch(fType) {
            case Type::kTransform: return in.applyTransform(ctx,
                                                            fTransformParams.fMatrix,
                                                            fTransformParams.fSampling);
            case Type::kCrop:      return in.applyCrop(ctx, fCropParams.fRect);
        }
        SkUNREACHABLE;
    }

    bool expectsNewImage() const { return fExpectNewImage; }
    const SkSamplingOptions& expectedSampling() const { return fExpectedSampling; }

private:
    enum class Type {
        kTransform, kCrop, /* kColorFilter, ... */
    };

    // Action
    Type fType;
    union {
        struct {
            LayerSpace<SkMatrix> fMatrix;
            SkSamplingOptions    fSampling;
        } fTransformParams;
        struct {
            LayerSpace<SkIRect>  fRect;
            // SkTileMode        fTileMode;
        } fCropParams;
        // fColorFilterParams...
    };

    // Expectation
    bool fExpectNewImage;
    SkSamplingOptions fExpectedSampling;
    // hmmm, how about expected layer bounds or the desired output?
    // these expectations are really dependent on what's coming in that the step is applied to.
    // that's okay, it just means that the list of action+expectations really represents a single
    // use case, and shouldn't be re-purposed across different source image/output checks.
    // And if that's the case, are there implications for how we specify the initial source image
    // or the desired output? I'd really prefer to be able to back propogate the expected layer
    // bounds.
    // FIXME And need a way to declare that we expect an empty image, which could possibly be
    // handled by looking at the layer bounds, but then we still should be figuring out those
    // from somewhere.
};

class TestCase {
public:
    TestCase(const LayerSpace<SkIRect>& sourceBounds, const SkColor4f& sourceColor)
            : fSourceBounds(sourceBounds)
            , fSourceColor(sourceColor)
            , fActions{} {}

    TestCase& applyCrop(const LayerSpace<SkIRect>& crop
                        bool expectNewImage = false) {
        // Fill-in automated expectations. A crop won't change sampling unless it produces a new
        // image. Otherwise it inherits the prior action's expectation.
        SkSamplingOptions expectedSampling;
        if (expectNewImage || fActions.empty()) {
            expectedSampling = FilterResult::kDefaultSampling;
        } else {
            expectedSampling = fActions[fActions.size() - 1].expectedSampling();
        }
        fActions.emplace_back(crop, expectNewImage, expectedSampling);
        return *this;
    }

    TestCase& applyTransform(const LayerSpace<SkMatrix>& matrix,
                             const SkSamplingOptions& sampling
                             bool expectNewImage = false,
                             std::optional<SkSamplingOptions> expectedSampling = {}) {
        // Fill-in automated expectations, which is simply that if it's not explicitly provided we
        // assume the sampling of the action will be output's sampling as well.
        if (!expectedSampling.has_value()) {
            expectedSampling = sampling;
        }
        fActions.emplace_back(matrix, sampling, expectNewImage, *expectedSampling);
    }

    // TODO: applyColorFilter() etc. to maintain parity with FilterResult API

    using SurfaceFactory = sk_sp<SkSpecialSurface>(*)(int width, int height);
    void validate(skiatest::Reporter*,
                  SurfaceFactory surfaceFactory,
                  const LayerSpace<SkIRect>& desiredOutput) const {
        SkASSERT(!fActions.empty()); // Bad test case if there aren't any actions

        // Every action has its own desired output, but they are calculated by propagating the root
        // bounds from the last action to the first.
        std::vector<LayerSpace<SkIRect>> desiredOutputs;
        desiredOutputs.resize(fActions.size(), desiredOutput);
        for (size_t i = fActions.size() - 2; i >= 0; --i) {
            desiredOutputs[i] = fActions[i+1].requiredInput(desiredOutputs[i+1]);
        }

        // Create the source image
        sk_sp<SkSpecialSurface> sourceSurface = surfaceFactory(fSourceBounds.width(),
                                                               fSourceBounds.height());
        sourceSurface->getCanvas()->clear(fSourceColor, nullptr);

        FilterResult source{sourceSurface->makeImageSnapshot(), fSourceBounds.topLeft()};
        Context baseContext{Mapping(SkMatrix::I()),
                            LayerSpace<SkIRect>::Empty(),
                            /*cache=*/nullptr,
                            source->image()->colorType(),
                            source->image()->getColorSpace(),
                            source};

        // Apply each action and validate, from first to last action
        for (size_t i = 0; i < fActions.size(); ++i) {
            auto ctx = baseContext.withNewDesiredOutput(desiredOutputs[i]);
            FilterResult output = fActions.apply(ctx, source);

            bool actualNewImage =
        }
    }

private:
    // Constructs an SkSpecialImage of the given size/location filled with the known color so that
    // color filter and crop results can be asserted against.
    LayerSpace<SkIRect> fSourceBounds;
    SkColor4f           fSourceColor;

    std::vector<ApplyAction> fActions;
};



} // anonymous namespace
