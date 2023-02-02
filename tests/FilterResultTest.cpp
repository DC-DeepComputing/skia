/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "src/core/SkImageFilterTypes.h"
#include "tests/Test.h"


#if SK_GRAPHITE_ENABLED
#include "include/gpu/graphite/Context.h"
#endif

#include <functional>

using namespace skif;

namespace {

void compare_image(skiatest::Reporter* reporter,
                   SkSpecialImage* expectedImage,
                   SkIPoint expectedOrigin,
                   const FilterResult& actual) {
    SkASSERT(expectedImage);
    // TODO: Implement readback for Ganesh and Graphite, but that requires plumbing their contexts
    // through to here.
    if (
#if SK_GRAPHITE_ENABLED
        expectedImage->isGraphiteBacked() ||
#endif
        expectedImage->isTextureBacked()) {
        return;
    }

    SkIPoint actualOrigin;
    sk_sp<SkSpecialImage> actualImage = actual.imageAndOffset(&actualOrigin);

    SkBitmap actualBM, expectedBM;

    if (actualImage) {
        // The intersection between expectedImage and actualImage should be approximately equal,
        // and the area outside actualImage should be transparent. actualImage should not have
        // pixels that are outside expectedImage's bounds.

    } else {
        // A null image in a FilterResult is equivalent to transparent black, so we should expect
        // the contents of 'expectedImage' to be transparent black.
        bool readSuccess = expectedImage->getROPixels(&expectedBM);
        REPORTER_ASSERT(reporter, readSuccess, "Unable to read expected image pixels\n");
        if (readSuccess) {
            for (int y = 0; y < expectedBM.height(); ++y) {
                for (int x = 0; x < expectedBM.width(); ++x) {
                    const bool isTransparentBlack =
                            SkColor4f{0.f, 0.f, 0.f, 0.f} == expectedBM.getColor4f(x, y);
                    REPORTER_ASSERT(reporter, isTransparentBlack, "(%d,%d) is not transparent",x,y);
                    if (!isTransparentBlack) {
                        return;
                    }
                }
            }
        }
    }
}


enum class Expect {
    kDeferredImage, // i.e. modified properties of FilterResult instead of rendering
    kNewImage,      // i.e. rendered a new image before modifying other properties
    kEmptyImage,    // i.e. everything is transparent black
};

class ApplyAction {
public:
    ApplyAction(const SkMatrix& matrix,
                const SkSamplingOptions& sampling,
                Expect expectation,
                const SkSamplingOptions& expectedSampling)
            : fType(Type::kTransform)
            , fTransformParams{LayerSpace<SkMatrix>(matrix), sampling}
            , fExpectation(expectation)
            , fExpectedSampling(expectedSampling) {}
    ApplyAction(const SkIRect& cropRect,
                Expect expectation,
                const SkSamplingOptions& expectedSampling)
            : fType(Type::kCrop)
            , fCropParams{LayerSpace<SkIRect>(cropRect)}
            , fExpectation(expectation)
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

    Expect expectation() const { return fExpectation; }
    const SkSamplingOptions& expectedSampling() const { return fExpectedSampling; }

    LayerSpace<SkIRect> expectedBounds(const LayerSpace<SkIRect>& inputBounds) const {
        // This assumes anything outside 'inputBounds' is transparent black.
        switch(fType) {
            case Type::kTransform: {
                if (inputBounds.isEmpty()) {
                    return LayerSpace<SkIRect>::Empty();
                }
                return fTransformParams.fMatrix.mapRect(inputBounds); }
            case Type::kCrop: {
                auto intersection = fCropParams.fRect;
                if (!intersection.intersect(inputBounds)) {
                    intersection = LayerSpace<SkIRect>::Empty();
                }
                return intersection; }
        }
        SkUNREACHABLE;
    }

    sk_sp<SkSpecialImage> renderExpectedImage(sk_sp<SkSpecialImage> source,
                                              const LayerSpace<SkIPoint>& origin,
                                              const LayerSpace<SkIRect>& desiredOutput) const {
        SkDebugf("checking source: %p\n", source.get());
        SkASSERT(source);

        SkISize size(desiredOutput.size());
        if (desiredOutput.isEmpty()) {
            SkASSERT(fExpectation == Expect::kEmptyImage);
            size = {1, 1};
        }

        SkDebugf("About to make surface: %d x %d\n", desiredOutput.width(), desiredOutput.height());
        auto surface = source->makeSurface(source->colorType(),
                                           source->getColorSpace(),
                                           size,
                                           kPremul_SkAlphaType, {});
        SkDebugf("made surface: %p\n", surface.get());
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->translate(-desiredOutput.left(), -desiredOutput.top());

        // FIXME do we need to clip to the expected bounds here?

        if (fExpectation != Expect::kEmptyImage) {
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
        }
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
    Expect fExpectation;
    SkSamplingOptions fExpectedSampling;
    // The expected desired outputs and layer bounds are calculated automatically based on the
    // action type and parameters to simplify test case specification.
};

using SurfaceFactory = std::function<sk_sp<SkSpecialSurface>(int /*width*/, int /*height*/)>;

class TestCase {
public:
    TestCase(skiatest::Reporter* reporter, SurfaceFactory surfaceFactory)
            : fReporter(reporter)
            , fSurfaceFactory(surfaceFactory)
            , fSourceBounds(LayerSpace<SkIRect>::Empty())
            , fSourceColor(SkColors::kTransparent) {}

    TestCase& source(const SkIRect& bounds, const SkColor4f& color) {
        fSourceBounds = LayerSpace<SkIRect>(bounds);
        fSourceColor = color;
        return *this;
    }

    TestCase& applyCrop(const SkIRect& crop,
                        Expect expectation) {
        // Fill-in automated expectations. A crop won't change sampling unless it produces a new
        // image. Otherwise it inherits the prior action's expectation.
        SkSamplingOptions expectedSampling;
        if (expectation == Expect::kNewImage || fActions.empty() || crop.isEmpty()) {
            expectedSampling = FilterResult::kDefaultSampling;
        } else {
            expectedSampling = fActions[fActions.size() - 1].expectedSampling();
        }
        fActions.emplace_back(crop, expectation, expectedSampling);
        return *this;
    }

    TestCase& applyTransform(const SkMatrix& matrix,
                             const SkSamplingOptions& sampling,
                             Expect expectation,
                             std::optional<SkSamplingOptions> expectedSampling = {}) {
        // Fill-in automated expectations, which is simply that if it's not explicitly provided we
        // assume the result's sampling equals what was passed to applyTransform().
        if (!expectedSampling.has_value()) {
            expectedSampling = sampling;
        }
        fActions.emplace_back(matrix, sampling, expectation, *expectedSampling);
        return *this;
    }

    // TODO: applyColorFilter() etc. to maintain parity with FilterResult API

    void validate(const SkIRect& desiredOutput) const {
        SkASSERT(!fActions.empty()); // It's a bad test case if there aren't any actions

        SkDebugf("starting validate\n");
        // Every action has its own desired output, but they are calculated by propagating the root
        // bounds from the last action to the first.
        std::vector<LayerSpace<SkIRect>> desiredOutputs;
        desiredOutputs.resize(fActions.size(), LayerSpace<SkIRect>{desiredOutput});
        for (int i = (int) fActions.size() - 2; i >= 0; --i) {
            desiredOutputs[i] = fActions[i+1].requiredInput(desiredOutputs[i+1]);
        }
        SkDebugf("computed desired outputs\n");

        // Create the source image
        FilterResult source;
        if (!fSourceBounds.isEmpty()) {
            sk_sp<SkSpecialSurface> sourceSurface = fSurfaceFactory(fSourceBounds.width(),
                                                                    fSourceBounds.height());
            sourceSurface->getCanvas()->clear(fSourceColor);
            source = FilterResult(sourceSurface->makeImageSnapshot(), fSourceBounds.topLeft());
        }

        // Applying modifiers to FilterResult might produce a new image, but hopefully it's
        // able to merge properties and even re-order operations to minimize the number of offscreen
        // surfaces that it creates. To validate that this is producing an equivalent image, we
        // track what to expect by rendering each action every time without any optimization.
        sk_sp<SkSpecialImage> expectedImage = source.refImage();
        LayerSpace<SkIPoint> expectedOrigin = source.layerBounds().topLeft();
        // The expected image can't ever be null, so we produce a transparent black image instead.
        if (!expectedImage) {
            sk_sp<SkSpecialSurface> expectedSurface = fSurfaceFactory(1, 1);
            expectedSurface->getCanvas()->clear(SK_ColorTRANSPARENT);
            expectedImage = expectedSurface->makeImageSnapshot();
            expectedOrigin = LayerSpace<SkIPoint>({0, 0});
        }
        SkASSERT(expectedImage);

        Context baseContext{Mapping(SkMatrix::I()),
                            LayerSpace<SkIRect>::Empty(),
                            /*cache=*/nullptr,
                            expectedImage->colorType(),
                            expectedImage->getColorSpace(),
                            source};

        // Apply each action and validate, from first to last action
        for (int i = 0; i < (int) fActions.size(); ++i) {
            SkDebugf("starting action %d\n", i);
            auto ctx = baseContext.withNewDesiredOutput(desiredOutputs[i]);
            FilterResult output = fActions[i].apply(ctx, source);
            // Validate consistency of the output
            REPORTER_ASSERT(fReporter, SkToBool(output.image()) == !output.layerBounds().isEmpty());
            SkDebugf("finished apply\n");

            bool actualNewImage = output.image() &&
                    (!source.image() || output.image()->uniqueID() != source.image()->uniqueID());
            switch(fActions[i].expectation()) {
                case Expect::kNewImage:
                    REPORTER_ASSERT(fReporter, actualNewImage);
                    break;
                case Expect::kDeferredImage:
                    REPORTER_ASSERT(fReporter, !actualNewImage && output.image());
                    break;
                case Expect::kEmptyImage:
                    REPORTER_ASSERT(fReporter, !actualNewImage && !output.image());
                    break;
            }

            // Validate layer bounds when we expect a new or deferred image
            if (output.image()) {
                SkIRect expectedBounds(fActions[i].expectedBounds(source.layerBounds()));
                REPORTER_ASSERT(fReporter, SkIRect(output.layerBounds()) == expectedBounds);
                REPORTER_ASSERT(fReporter, output.sampling() == fActions[i].expectedSampling());
            }
            SkDebugf("checked basic expectations\n");

            expectedImage = fActions[i].renderExpectedImage(std::move(expectedImage),
                                                            expectedOrigin,
                                                            desiredOutputs[i]);
            expectedOrigin = desiredOutputs[i].topLeft();
            compare_image(fReporter, expectedImage.get(), SkIPoint(expectedOrigin), output);
            SkDebugf("finished rendering expected image\n");
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
#define DEF_GANESH_TEST_SUITE(name) \
    DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS( \
            FilterResult_##name##_gpu, \
            r, ctxInfo, CtsEnforcement::kApiLevel_T) { \
        run_gpu_suite(ctxInfo.directContext(), r, test_suite_##name); \
    }
#else
#define DEF_GANESH_TEST_SUITE(name) // do nothing
#endif

#if SK_GRAPHITE_ENABLED
void run_graphite_suite(skgpu::graphite::Context* context,
                        skiatest::Reporter* reporter,
                        TestSuite suite) {
    using namespace skgpu::graphite;
    auto recorderKeepAlive = context->makeRecorder();
    auto recorder = recorderKeepAlive.get();

    auto factory = [recorder](int width, int height) {
        SkImageInfo info = SkImageInfo::Make(width, height,
                                             kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);
        return SkSpecialSurface::MakeGraphite(recorder, info, {});
    };

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
#define DEF_GRAPHITE_TEST_SUITE(name) \
    DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(FilterResult_##name##_graphite, r, context) { \
        run_graphite_suite(context, r, test_suite_##name); \
    }
#else
#define DEF_GRAPHITE_TEST_SUITE(name) // do nothing
#endif

// Assumes 'name' refers to a static function of type TestSuite.
#define DEF_TEST_SUITE(name, reporter, surfaceFactory) \
    static void test_suite_##name(skiatest::Reporter*, SurfaceFactory); \
    DEF_GANESH_TEST_SUITE(name) \
    DEF_TEST(FilterResult_##name##_raster, r) { \
        run_raster_suite(r, test_suite_##name); \
    } \
    void test_suite_##name(skiatest::Reporter* reporter, SurfaceFactory surfaceFactory)

// ----------------------------------------------------------------------------

DEF_TEST_SUITE(EmptySource, r, s) {
    // applyCrop -> stays empty
    TestCase(r, s).source(SkIRect::MakeEmpty(), SkColors::kRed)
                  .applyCrop({0, 0, 10, 10}, Expect::kEmptyImage)
                  .validate({0, 0, 20, 20});
    // applyTransform -> stays empty
    TestCase(r, s).source(SkIRect::MakeEmpty(), SkColors::kRed)
                  .applyTransform(SkMatrix::Translate(10.f, 10.f), {}, Expect::kEmptyImage)
                  .validate({10, 10, 20, 20});
}

DEF_TEST_SUITE(EmptyDesiredOutput, r, s) {
    // applyCrop -> becomes empty
    TestCase(r, s).source({0, 0, 10, 10}, SkColors::kRed)
                  .applyCrop({2, 2, 8, 8}, Expect::kEmptyImage)
                  .validate(SkIRect::MakeEmpty());
    // applyTransform -> becomes empty
    TestCase(r, s).source({0, 0, 10, 10}, SkColors::kRed)
                  .applyTransform(SkMatrix::RotateDeg(10.f), {}, Expect::kEmptyImage)
                  .validate(SkIRect::MakeEmpty());
}

DEF_TEST_SUITE(Crop, r, s) {
    // Contained within source and dest

    // Contained within source, but intersects dest

    // Contained within dest, but intersects source

    // Intersects both source and dest

    // Contains both dest and source

    // Contains source, but intersects dest

    // Contains dest, but intersects source
}

DEF_TEST_SUITE(CropDisjointFromSourceAndOutput, r, s) {
    // Source and dest are not disjoint, crop intersects dest but not source

    // Source and dest are also disjoint, crop intersects dest but not source

    // Source and dest are not disjoint, crop intersects source but not dest

    // Source and dest are also disjoint, crop intersects source but not dest

    // Crop, source, and dest are all disjoint
}

DEF_TEST_SUITE(EmptyCrop, r, s) {
    // Empty crops always produce an empty image, regardless of prior actions
}

DEF_TEST_SUITE(DisjointCrops, r, s) {
    // Two consecutive crops (both independently intersecting the original source and dest) that
    // do not intersect each other should produce an empty image.

}

DEF_TEST_SUITE(IntersectingCrops, r, s) {
    // Two consecutive crops intersect each other so should be merged into one operation.
}

DEF_TEST_SUITE(Transform, r, s) {

}

DEF_TEST_SUITE(CompatibleSamplingConcatsTransforms, r, s) {

}

DEF_TEST_SUITE(IntegerOffsetIgnoresSampling, r, s) {

}

DEF_TEST_SUITE(IncompatibleSamplingResolvesImages, r, s) {

}

DEF_TEST_SUITE(TransformAvoidsDesiredOutput, r, s) {

}

DEF_TEST_SUITE(TransformAvoidsCrop, r, s) {

}

DEF_TEST_SUITE(TransformPostCrop, r, s) {
    // A transform applied after a crop triggers a new image

    // unless the transform is an integer translation

    // or the desired output of the transform action would not sample any edge
    // of the crop.
}

DEF_TEST_SUITE(TransformPostCropOffscreen, r, s) {
    // If the desired output
}

DEF_TEST_SUITE(TransformPreCrop, r, s) {
    // A crop after a transform should always apply since it adjusts the layer
    // bounds.
}

} // anonymous namespace
