/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkImageFilterTypes.h"

using namespace skif;

namespace {

void compare_image(skiatest::Reporter* reporter,
                   SkSpecialImage* expected,
                   LayerSpace<SkIPoint> expectedOrigin,
                   const FilterResult& actual) {
    // TODO: implement this
}

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

    // Performs the action to be tested
    FilterResult apply(const Context& ctx, const FilterResult& in) const {
        switch(fType) {
            case Type::kTransform: return in.applyTransform(ctx,
                                                            fTransformParams.fMatrix,
                                                            fTransformParams.fSampling);
            case Type::kCrop:      return in.applyCrop(ctx, fCropParams.fRect);
        }
        SkUNREACHABLE;
    }

    // Explicitly specified, and computed expectations for this action
    bool expectsNewImage() const { return fExpectNewImage; }
    const SkSamplingOptions& expectedSampling() const { return fExpectedSampling; }

    LayerSpace<SkIRect> expectedBounds(const LayerSpace<SkIRect>& inputBounds) const {
        // This assumes anything outside 'inputBounds' is transparent black.
        switch(fType) {
            case Type::kTransform: {
                if (inputBounds.empty()) {
                    return LayerSpace<SkIRect>::Empty();
                }
                return fTransformParams.fMatrix.mapRect(inputBounds); }
            case Type::kCrop: {
                auto intersection = fCropParams.fRect;
                if (!intersection.intersect(desiredOutput)) {
                    intersection = LayerSpace<SkIRect>::Empty();
                }
                return intersection; }
        }
        SkUNREACHABLE;
    }

    sk_sp<SkSpecialImage> expectedImage(sk_sp<SkSpecialImage> source,
                                        const LayerSpace<SkIPoint>& origin,
                                        const LayerSpace<SkIRect>& desiredOutput) const {
        SkASSERT(source);

        auto surface = source->makeSurface(source->colorType(),
                                           source->getColorSpace(),
                                           SkISize(desiredOutput.size()),
                                           kPremul_SkAlphaType, {});
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->translate(-desiredOutput.left(), -desiredOutput.top());

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setBlendMode(SkBlendMode::kSrc);
        SkSamplingOptions sampling = FilterResult::kDefaultSampling;
        switch(fType) {
            case Type::kTransform:
                canvas->concat(SkMatrix(fTransformParams.fMatrix));
                sampling = fTransformParams.fSampling;
                break;
            case Type::kCrop:
                canvas->clipIRect(SkIRect(fCropParams.fRect));
                break;
        }
        source->draw(canvas, origin.x(), origin.y(), sampling, &paint);
        return surface->makeImageSnapshot();
    }

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
    // The expected desired outputs and layer bounds are calculated automatically based on the
    // action type and parameters to simplify test case specification.
};

using SurfaceFactory = sk_sp<SkSpecialSurface>(*)(int width, int height);

class TestCase {
public:
    TestCase(skiatest::Reporter* reporter, SurfaceFactory surfaceFactory)
            : fReporter(reporter)
            , fSurfaceFactory(surfaceFactory)
            , fSourceBounds(LayerSpace<SkIRect>::Empty())
            , fSourceColor(SkColors::kTransparent) {}

    TestCase& source(const LayerSpace<SkIRect>& bounds, const SkColor4f& color) {
        fSourceBounds = bounds;
        fSourceColor = color;
        return *this;
    }

    TestCase& applyCrop(const LayerSpace<SkIRect>& crop
                        bool expectNewImage = false) {
        // Fill-in automated expectations. A crop won't change sampling unless it produces a new
        // image. Otherwise it inherits the prior action's expectation.
        SkSamplingOptions expectedSampling;
        if (expectNewImage || fActions.empty() || crop.empty()) {
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
        // assume the result's sampling equals what was passed to applyTransform().
        if (!expectedSampling.has_value()) {
            expectedSampling = sampling;
        }
        fActions.emplace_back(matrix, sampling, expectNewImage, *expectedSampling);
    }

    // TODO: applyColorFilter() etc. to maintain parity with FilterResult API

    void validate(const LayerSpace<SkIRect>& desiredOutput) const {
        SkASSERT(!fActions.empty()); // It's a bad test case if there aren't any actions

        // Every action has its own desired output, but they are calculated by propagating the root
        // bounds from the last action to the first.
        std::vector<LayerSpace<SkIRect>> desiredOutputs;
        desiredOutputs.resize(fActions.size(), desiredOutput);
        for (size_t i = fActions.size() - 2; i >= 0; --i) {
            desiredOutputs[i] = fActions[i+1].requiredInput(desiredOutputs[i+1]);
        }

        // Create the source image
        // FIXME should be workable with an empty source too
        sk_sp<SkSpecialSurface> sourceSurface = fSurfaceFactory(fSourceBounds.width(),
                                                                fSourceBounds.height());
        sourceSurface->getCanvas()->clear(fSourceColor, /*colorSpace=*/nullptr);

        FilterResult source{sourceSurface->makeImageSnapshot(), fSourceBounds.topLeft()};
        Context baseContext{Mapping(SkMatrix::I()),
                            LayerSpace<SkIRect>::Empty(),
                            /*cache=*/nullptr,
                            source->image()->colorType(),
                            source->image()->getColorSpace(),
                            source};

        // Applying modifiers to FilterResult might produce a new image, but hopefully it's
        // able to merge properties and even re-order operations to minimize the number of offscreen
        // surfaces that it creates. To validate that this is producing an equivalent image, we
        // track what to expect by rendering each action every time without any optimization.
        sk_sp<SkSpecialImage> expectedImage = source.image();
        LayerSpace<SkIPoint> expectedOrigin = source.layerBounds().topLeft();

        // Apply each action and validate, from first to last action
        for (size_t i = 0; i < fActions.size(); ++i) {
            auto ctx = baseContext.withNewDesiredOutput(desiredOutputs[i]);
            FilterResult output = fActions[i].apply(ctx, source);

            bool actualNewImage = output.image() &&
                    (!source.image() || output.image()->uniqueID() != source.image()->uniqueID());
            REPORTER_ASSERT(fReporter, actualNewImage == fActions[i].fExpectNewImage);

            LayerSpace<SkIRect> expectedLayerBounds =
                    fActions[i].expectedBounds(source.layerBounds());
            if (!expectedLayerBounds.intersect(desiredOutputs[i])) {
                // The output FilterResult should be empty
                REPORTER_ASSERT(fReporter, !SkToBool(output.image()));
            } else {
                REPORTER_ASSERT(fReporter, SkToBool(output.image()));
                REPORTER_ASSERT(fReporter, expectedLayerBounds == output.layerBounds());
                REPORTER_ASSERT(fReporter, output.sampling() == fActions[i].fExpectedSampling);
            }

            expectedImage = fActions[i].renderExpectedImage(std::move(expectedImage),
                                                            expectedOrigin,
                                                            desiredOutputs[i]);
            expectedOrigin = desiredOutputs[i].topLeft();
            compare_images(fReporter, expectedImage.get(), expectedOrigin, output);

            source = output;
        }
    }

private:
    skiatest::Reporter* fReporter;
    SurfaceFactory      fSurfaceFactory;

    // Constructs an SkSpecialImage of the given size/location filled with the known color so that
    // color filter and crop results can be asserted against.
    LayerSpace<SkIRect> fSourceBounds;
    SkColor4f           fSourceColor;

    std::vector<ApplyAction> fActions;
};

using TestSuite = void(*)(skiatest::Reporter*, SurfaceFactory);

void run_raster_suite(skiatest::Reporter* reporter, TestSuite suite) {
    auto factory = [](int width, int height) {
        SkImageInfo info = SkImageInfo::Make(width, height,
                                             kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);
        return SkSpecialSurface::MakeRaster(info, {});
    };
    suite(reporter, factory);
}

#if SK_SUPPORT_GPU
void run_gpu_suite(GrRecordingContext* context, skiatest::Reporter* reporter, TestSuite suite) {
    auto factory = [context](int width, int height) {
        SkImageInfo info = SkImageInfo::Make(width, height,
                                             kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);
        return SkSpecialSurface::MakeRenderTarget(context, info, {}, kTopLeft_GrSurfaceOrigin);
    };
    suite(reporter, factory);
}
#endif

#if SK_GRAPHITE_ENABLED
void run_graphite_suite(skgpu::graphite::Context* context,
                        skiatest::Reporter* reporter,
                        TestSuite suite) {
    auto factory = [recorder](int width, int height) {
        SkImageInfo info = SkImageInfo::Make(width, height,
                                             kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);
        return SkSpecialSurface::MakeGraphite(recorder, info, {});
    };

    auto recorder = context->makeRecorder();
    // FIXME: The test suite does read backs of intermediate images to verify that steps are
    // applied correctly. The verification may not be defined correctly to switch to async mode.
    suite(reporter, factory);

    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        ERRORF(reporter, "Failed to make recording");
        return;
    }

    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);
    context->submit(SyncToCpu::kYes);
}
#endif

// Assumes 'name' refers to a static function of type TestSuite.
#define DEF_TEST_SUITE(name, reporter, surfaceFactory) \
    static void test_suite_##name(skiatest::Reporter*, SurfaceFactory); \
#if SK_SUPPORT_GPU
    DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS( \
            FilterResult_##name##_gpu, \
            r, ctxInfo, CtsEnforcement::kApiLevel_T) { \
        run_gpu_suite(ctxInfo.directContext(), r, test_suite_##name);
    } \
#endif
#if SK_GRAPHITE_ENABLED
    DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS( \
            FilterResult_##name##_graphite, \
            r, context) { \
        run_graphite_suite(context, r, test_suite_##name); \
    } \
#endif
    DEF_TEST(FilterResult_##name##_raster, r) { \
        run_raster_suite(r, test_suite_##name); \
    } \
    void test_suite_##name(skiatest::Reporter* reporter, SurfaceFactory surfaceFactory)

// ----------------------------------------------------------------------------

DEF_TEST_SUITE(Single, r, s) {
    // A single crop

    // A single offset

    // A single scale transform

    // A single complex transform
}

DEF_TEST_SUITE(Empty, r, s) {
    // An empty source image

    // An empty desired output

    // A crop that doesn't intersect the source

    // A crop that doesn't intersect the desired output

    // A crop that is empty

    // Two crops that do not intersect

    // An transform applied to an empty source

    // An transform applied after the crop makes it empty

    // An transform such that the subsequent crop is clipped out

    // An transform such that the prior crop is clipped out
}

DEF_TEST_SUITE(OffsetAndCrop, r, s) {
    // Just offset
    TestCase(r, s).source()

    // Just crop

    // A crop that doesn't intersect the source

    // A crop that doesn't intersect the desired output

    // An empty crop

    // An offset applied to an empty image

    // A crop

    // Intersecting crops that collapse into a single operation

    // Offset and then crop

    // Crop and then offset

    // Crop, offset, then crop again

    // Offset such that subsequent crop is clipped out

    // Rotate and then crop that doesn't trigger a new image since crop is last

    // Rotate, crop, additional transform triggering a new image to resolve first rotate+crop

    // Rotate crop additional transform but with a desired output that means we can skip the crop

    // Crop and then rotate, not triggering an image since crop should have been elided in the beginning

    // Crop, rotate, and then crop again requiring the image to be made


    // Canonical operation order is:

    // sample with tiling
    // apply color filter
    // transformed
    // and then clipped
    // ugh this has benefits to put the crop last because that means we can also combine crops together
    // but it never seems to be the way I actually want to think about it geometrically, instead I want
    // to say that we crop "locally" and then transform that.

    // And if we never elide producing images, it doesn't really matter since they are independent operations
    // but if we do want to merge them, then it's important to choose the order.
    // And it's easy to know when we can bypass a transform and push the crop before it (axis-aligned)
    //   which means we can always combine transforms and pretty frequently combine crops.
    //
    // But then we always end up with the case where if we have one last crop that can't push early,
    // we can still delay baking a new image because we can just restrict our layer bounds and output image....

}

DEF_TEST_SUITE(ScaleAndCrop, r, s) {

}

DEF_TEST_SUITE(RotateAndCrop, r, s) {

}

} // anonymous namespace
