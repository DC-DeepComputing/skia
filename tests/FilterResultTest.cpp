/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "src/core/SkImageFilterTypes.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"


#if SK_GRAPHITE_ENABLED
#include "include/gpu/graphite/Context.h"
#endif

#include <functional>

using namespace skif;

namespace {

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
        SkASSERT(source);

        SkISize size(desiredOutput.size());
        if (desiredOutput.isEmpty()) {
            size = {1, 1};
        }

        auto surface = source->makeSurface(source->colorType(),
                                           source->getColorSpace(),
                                           size,
                                           kPremul_SkAlphaType, {});
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->translate(-desiredOutput.left(), -desiredOutput.top());

        LayerSpace<SkIRect> sourceBounds{
                SkIRect::MakeXYWH(origin.x(), origin.y(), source->width(), source->height())};
        LayerSpace<SkIRect> expectedBounds = this->expectedBounds(sourceBounds);

        canvas->clipIRect(SkIRect(expectedBounds), SkClipOp::kIntersect);

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

class TestRunner {
public:
    // Raster-backed TestRunner
    TestRunner(skiatest::Reporter* reporter)
            : fReporter(reporter) {}

    // Ganesh-backed TestRunner
#if SK_SUPPORT_GPU
    TestRunner(skiatest::Reporter* reporter, GrDirectContext* context)
            : fReporter(reporter)
            , fDirectContext(context) {}
#endif

    // Graphite-backed TestRunner
#if SK_GRAPHITE_ENABLED
    TestRunner(skiatest::Reporter* reporter, skgpu::graphite::Recorder* recorder)
            : fReporter(reporter)
            , fRecorder(recorder) {}
#endif

    // Let TestRunner be passed in to places that take a Reporter* or to REPORTER_ASSERT etc.
    operator skiatest::Reporter*() const { return fReporter; }
    skiatest::Reporter* operator->() const { return fReporter; }

    sk_sp<SkSpecialSurface> newSurface(int width, int height) const {
        SkImageInfo info = SkImageInfo::Make(width, height,
                                             kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);
#if SK_SUPPORT_GPU
        if (fDirectContext) {
            return SkSpecialSurface::MakeRenderTarget(fDirectContext, info, {},
                                                      kTopLeft_GrSurfaceOrigin);
        } else
#endif
#if SK_GRAPHITE_ENABLED
        if (fRecorder) {
            return SkSpecialSurface::MakeGraphite(fRecorder, info, {});
        } else
#endif
        {
            return SkSpecialSurface::MakeRaster(info, {});
        }
    }

    bool compareImages(SkSpecialImage* expectedImage,
                       SkIPoint expectedOrigin,
                       const FilterResult& actual) {
        SkASSERT(expectedImage);

        SkIPoint actualOrigin;
        sk_sp<SkSpecialImage> actualImage = actual.imageAndOffset(&actualOrigin);

        SkBitmap expectedBM = this->readPixels(expectedImage);
        SkBitmap actualBM = this->readPixels(actualImage.get()); // empty if actualImage is null
        SkTArray<SkIPoint> badPixels;
        if (!this->compareBitmaps(expectedBM, expectedOrigin, actualBM, actualOrigin, &badPixels)) {
            this->logBitmaps(expectedBM, actualBM, badPixels);
            return false;
        }

        return true;
    }

private:

    bool compareBitmaps(const SkBitmap& expected,
                        SkIPoint expectedOrigin,
                        const SkBitmap& actual,
                        SkIPoint actualOrigin,
                        SkTArray<SkIPoint>* badPixels) {
        SkIRect excludeTransparentCheck; // region in expectedBM that can be non-transparent
        if (actual.empty()) {
            // A null image in a FilterResult is equivalent to transparent black, so we should
            // expect the contents of 'expectedImage' to be transparent black.
            excludeTransparentCheck = SkIRect::MakeEmpty();
        } else {
            // The actual image bounds should be contained in the expected image's bounds.
            SkIRect actualBounds = SkIRect::MakeXYWH(actualOrigin.x(), actualOrigin.y(),
                                                     actual.width(), actual.height());
            SkIRect expectedBounds = SkIRect::MakeXYWH(expectedOrigin.x(), expectedOrigin.y(),
                                                       expected.width(), expected.height());
            const bool contained = expectedBounds.contains(actualBounds);
            REPORTER_ASSERT(fReporter, contained,
                    "actual image [%d %d %d %d] not contained within expected [%d %d %d %d]",
                    actualBounds.fLeft, actualBounds.fTop,
                    actualBounds.fRight, actualBounds.fBottom,
                    expectedBounds.fLeft, expectedBounds.fTop,
                    expectedBounds.fRight, expectedBounds.fBottom);
            if (!contained) {
                return false;
            }

            // The actual pixels should match fairly closely with the expected, allowing for minor
            // differences from consolidating actions into a single render, etc.
            int errorCount = 0;
            SkIPoint offset = actualOrigin - expectedOrigin;
            for (int y = 0; y < actual.height(); ++y) {
                for (int x = 0; x < actual.width(); ++x) {
                    SkIPoint ep = {x + offset.x(), y + offset.y()};
                    SkColor4f expectedColor = expected.getColor4f(ep.fX, ep.fY);
                    SkColor4f actualColor = actual.getColor4f(x, y);
                    if (actualColor != expectedColor &&
                        !this->approxColor(this->boxFilter(actual, x, y),
                                           this->boxFilter(expected, ep.fX, ep.fY))) {
                        badPixels->push_back(ep);
                        errorCount++;
                    }
                }
            }

            const int totalCount = expected.width() * expected.height();
            const float percentError = 100.f * errorCount / (float) totalCount;
            const bool approxMatch = percentError <= 2.f;
            REPORTER_ASSERT(fReporter, approxMatch,
                            "%d pixels were too different from %d total (%f %%)",
                            errorCount, totalCount, percentError);
            if (!approxMatch) {
                return false;
            }

            // The expected pixels outside of the actual bounds should be transparent, otherwise
            // the actual image is not returning enough data.
            excludeTransparentCheck = actualBounds.makeOffset(-expectedOrigin);
        }

        for (int y = 0; y < expected.height(); ++y) {
            for (int x = 0; x < expected.width(); ++x) {
                if (!excludeTransparentCheck.isEmpty() && excludeTransparentCheck.contains(x, y)) {
                    continue;
                }

                SkColor4f color = expected.getColor4f(x, y);
                const bool isTransparentBlack = SkColor4f{0.f, 0.f, 0.f, 0.f} == color;
                REPORTER_ASSERT(fReporter, isTransparentBlack,
                                "(%d,%d) is not transparent: (%f %f %f %f)",
                                x, y, color.fR, color.fG, color.fB, color.fA);
                if (!isTransparentBlack) {
                    badPixels->push_back({x, y});
                    return false;
                }
            }
        }

        return true;
    }

    bool approxColor(const SkColor4f& a, const SkColor4f& b) const {
        static constexpr float kRGBTolerance = 8.f / 255.f;

        SkPMColor4f apm = a.premul();
        SkPMColor4f bpm = b.premul();
        return SkScalarNearlyEqual(apm.fR, bpm.fR, kRGBTolerance) &&
               SkScalarNearlyEqual(apm.fG, bpm.fG, kRGBTolerance) &&
               SkScalarNearlyEqual(apm.fB, bpm.fB, kRGBTolerance) &&
               SkScalarNearlyEqual(apm.fA, bpm.fA, kRGBTolerance);
    }

    SkColor4f boxFilter(const SkBitmap& bm, int x, int y) const {
        static const float kKernel[3][3] = {{0.25f, 0.35f, 0.25f},
                                            {0.35, 0.45f,  0.35f},
                                            {0.25f, 0.35f, 0.25f}};
        SkPMColor4f sum = {0.f, 0.f, 0.f, 0.f};
        float netWeight = 0.f;
        for (int sy = y - 1; sy <= y + 1; ++sy) {
            for (int sx = x - 1; sx <= x + 1; ++sx) {
                float weight = kKernel[sy - y + 1][sx - x + 1];

                if (sx < 0 || sx >= bm.width() || sy < 0 || sy >= bm.height()) {
                    // Treat outside image as transparent black, this is necessary to get
                    // consistent comparisons between expected and actual images where the actual
                    // is cropped as tightly as possible.
                    netWeight += weight;
                    continue;
                }

                SkPMColor4f c = bm.getColor4f(sx, sy).premul() * weight;
                sum.fR += c.fR;
                sum.fG += c.fG;
                sum.fB += c.fB;
                sum.fA += c.fA;
                netWeight += weight;
            }
        }
        SkASSERT(netWeight > 0.f);
        return sum.unpremul() * (1.f / netWeight);
    }

    SkBitmap readPixels(const SkSpecialImage* specialImage) const {
        if (!specialImage) {
            return SkBitmap(); // an empty bitmap
        }

        int srcX = specialImage->subset().fLeft;
        int srcY = specialImage->subset().fTop;
        SkImageInfo ii = SkImageInfo::Make(specialImage->dimensions(),
                                           specialImage->colorInfo());
        SkBitmap bm;
        bm.allocPixels(ii);
#if SK_SUPPORT_GPU
        if (fDirectContext) {
            // Ganesh backed, just use the SkImage::readPixels API
            SkASSERT(specialImage->isTextureBacked());
            sk_sp<SkImage> image = specialImage->asImage();
            SkAssertResult(image->readPixels(fDirectContext, bm.pixmap(), srcX, srcY));
        } else
#endif
#if SK_GRAPHITE_ENABLED
        if (fRecorder) {
            // Graphite backed, so use the private testing-only synchronous API
            SkASSERT(specialImage->isGraphiteBacked());
            auto view = specialImage->textureProxyView();
            SkAssertResult(fRecorder->priv().context()->priv().readPixels(
                    bm.pixmap(), view.proxy(), ii.makeWH(view.width(), view.height()), srcX, srcY));
        } else
#endif
        {
            // Assume it's raster backed, so use getROPixels directly
            SkAssertResult(specialImage->getROPixels(&bm));
        }

        return bm;
    }

    void logBitmaps(const SkBitmap& expected,
                    const SkBitmap& actual,
                    const SkTArray<SkIPoint>& badPixels) {
        if (fLoggedErrorImage) {
            return; // no more spam
        }

        SkString expectedURL;
        BitmapToBase64DataURI(expected, &expectedURL);
        SkDebugf("Expected:\n%s\n\n", expectedURL.c_str());

        if (!actual.empty()) {
            SkString actualURL;
            BitmapToBase64DataURI(actual, &actualURL);
            SkDebugf("Actual:\n%s\n\n", actualURL.c_str());
        } else {
            SkDebugf("Actual: null (fully transparent)\n\n");
        }

        if (!badPixels.empty()) {
            for (auto p : badPixels) {
                expected.erase(SkColors::kRed, nullptr, SkIRect::MakeXYWH(p.fX, p.fY, 1, 1));
            }
            SkString markedURL;
            BitmapToBase64DataURI(expected, &markedURL);
            SkDebugf("Errors:\n%s\n\n", markedURL.c_str());
        }

        fLoggedErrorImage = true;
    }

    skiatest::Reporter* fReporter;
#if SK_SUPPORT_GPU
    GrDirectContext* fDirectContext = nullptr;
#endif
#if SK_GRAPHITE_ENABLED
    skgpu::graphite::Recorder* fRecorder = nullptr;
#endif

    bool fLoggedErrorImage = false; // only do this once per test runner
};

class TestCase {
public:
    TestCase(TestRunner& runner, std::string name)
            : fRunner(runner)
            , fName(name)
            , fSourceBounds(LayerSpace<SkIRect>::Empty())
            , fSourceColor(SkColors::kTransparent)
            , fDesiredOutput(LayerSpace<SkIRect>::Empty()) {}

    TestCase& source(const SkIRect& bounds, const SkColor4f& color) {
        fSourceBounds = LayerSpace<SkIRect>(bounds);
        fSourceColor = color;
        return *this;
    }

    TestCase& desiredOutput(const SkIRect& bounds) {
        fDesiredOutput = LayerSpace<SkIRect>(bounds);
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

    TestCase& applyTransform(const SkMatrix& matrix, Expect expectation) {
        return this->applyTransform(matrix, FilterResult::kDefaultSampling, expectation);
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

    void run() const {
        skiatest::ReporterContext caseLabel(fRunner, fName);
        this->run(true);
        this->run(false);
    }

    void run(bool backPropagateDesiredOutput) const {
        SkASSERT(!fActions.empty()); // It's a bad test case if there aren't any actions

        skiatest::ReporterContext backPropagate(
                fRunner, SkStringPrintf("backpropagate output: %d", backPropagateDesiredOutput));

        std::vector<LayerSpace<SkIRect>> desiredOutputs;
        desiredOutputs.resize(fActions.size(), fDesiredOutput);
        if (backPropagateDesiredOutput) {
            // Every action has its own desired output, but they are calculated by propagating the
            // root bounds from the last action to the first.
            for (int i = (int) fActions.size() - 2; i >= 0; --i) {
                desiredOutputs[i] = fActions[i+1].requiredInput(desiredOutputs[i+1]);
            }
        } else {
            // Set the desired output to be equal to the expected output so that there is no
            // further restriction of what's computed for early actions to then be ruled out by
            // subsequent actions.
            auto inputBounds = fSourceBounds;
            for (int i = 0; i < (int) fActions.size() - 1; ++i) {
                inputBounds = fActions[i].expectedBounds(inputBounds);
                desiredOutputs[i] = inputBounds;
            }
        }

        // Create the source image
        FilterResult source;
        if (!fSourceBounds.isEmpty()) {
            sk_sp<SkSpecialSurface> sourceSurface = fRunner.newSurface(fSourceBounds.width(),
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
            sk_sp<SkSpecialSurface> expectedSurface = fRunner.newSurface(1, 1);
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
            skiatest::ReporterContext actionLabel(fRunner, SkStringPrintf("action %d", i));
            auto ctx = baseContext.withNewDesiredOutput(desiredOutputs[i]);
            FilterResult output = fActions[i].apply(ctx, source);
            // Validate consistency of the output
            REPORTER_ASSERT(fRunner, SkToBool(output.image()) == !output.layerBounds().isEmpty());

            LayerSpace<SkIRect> expectedBounds = fActions[i].expectedBounds(source.layerBounds());
            Expect correctedExpectation = fActions[i].expectation();
            if (!expectedBounds.intersect(desiredOutputs[i])) {
                // Test cases should provide image expectations for the case where desired output
                // is not back-propagated. When desired output is back-propagated, it can lead to
                // earlier actions becoming empty actions.
                SkASSERT(fActions[i].expectation() == Expect::kEmptyImage ||
                         backPropagateDesiredOutput);
                expectedBounds = LayerSpace<SkIRect>::Empty();
                correctedExpectation = Expect::kEmptyImage;
            }

            bool actualNewImage = output.image() &&
                    (!source.image() || output.image()->uniqueID() != source.image()->uniqueID());
            switch(correctedExpectation) {
                case Expect::kNewImage:
                    REPORTER_ASSERT(fRunner, actualNewImage);
                    break;
                case Expect::kDeferredImage:
                    REPORTER_ASSERT(fRunner, !actualNewImage && output.image());
                    break;
                case Expect::kEmptyImage:
                    REPORTER_ASSERT(fRunner, !actualNewImage && !output.image());
                    break;
            }

            // Validate layer bounds and sampling when we expect a new or deferred image
            if (output.image()) {
                REPORTER_ASSERT(fRunner, !expectedBounds.isEmpty());
                REPORTER_ASSERT(fRunner, SkIRect(output.layerBounds()) == SkIRect(expectedBounds));
                REPORTER_ASSERT(fRunner, output.sampling() == fActions[i].expectedSampling());
            }

            expectedImage = fActions[i].renderExpectedImage(std::move(expectedImage),
                                                            expectedOrigin,
                                                            desiredOutputs[i]);
            expectedOrigin = desiredOutputs[i].topLeft();
            if (!fRunner.compareImages(expectedImage.get(), SkIPoint(expectedOrigin), output)) {
                // If one iteration is incorrect, its failures will likely cascade to further
                // actions so end now as the test has failed.
                break;
            }
            source = output;
        }
    }

private:
    TestRunner& fRunner;
    std::string fName;

    // Constructs an SkSpecialImage of the given size/location filled with the known color so that
    // color filter and crop results can be asserted against.
    LayerSpace<SkIRect> fSourceBounds;
    SkColor4f           fSourceColor;

    // The intended area to fill with the result, controlled by outside factors (e.g. clip bounds)
    LayerSpace<SkIRect> fDesiredOutput;

    std::vector<ApplyAction> fActions;
};

#if SK_SUPPORT_GPU
#define DEF_GANESH_TEST_SUITE(name) \
    DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS( \
            FilterResult_##name##_gpu, \
            r, ctxInfo, CtsEnforcement::kApiLevel_T) { \
        TestRunner runner(r, ctxInfo.directContext()); \
        test_suite_##name(runner); \
    }
#else
#define DEF_GANESH_TEST_SUITE(name) // do nothing
#endif

#if SK_GRAPHITE_ENABLED
#define DEF_GRAPHITE_TEST_SUITE(name) \
    DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(FilterResult_##name##_graphite, r, context) { \
        using namespace skgpu::graphite; \
        auto recorder = context->makeRecorder(); \
        TestRunner runner(r, recorder.get()); \
        test_suite_##name(runner); \
        std::unique_ptr<Recording> recording = recorder->snap(); \
        if (!recording) { \
            ERRORF(reporter, "Failed to make recording"); \
            return; \
        } \
        InsertRecordingInfo insertInfo; \
        insertInfo.fRecording = recording.get(); \
        context->insertRecording(insertInfo); \
        context->submit(SyncToCpu::kYes); \
    }
#else
#define DEF_GRAPHITE_TEST_SUITE(name) // do nothing
#endif

// Assumes 'name' refers to a static function of type TestSuite.
#define DEF_TEST_SUITE(name, runner) \
    static void test_suite_##name(TestRunner&); \
    DEF_GRAPHITE_TEST_SUITE(name) \
    DEF_GANESH_TEST_SUITE(name) \
    DEF_TEST(FilterResult_##name##_raster, reporter) { \
        TestRunner runner(reporter); \
        test_suite_##name(runner); \
    } \
    void test_suite_##name(TestRunner& runner)

// ----------------------------------------------------------------------------

DEF_TEST_SUITE(EmptySource, r) {
    TestCase(r, "applyCrop() to empty source")
            .source(SkIRect::MakeEmpty(), SkColors::kRed)
            .applyCrop({0, 0, 10, 10}, Expect::kEmptyImage)
            .desiredOutput({0, 0, 20, 20})
            .run();

    TestCase(r, "applyTransform() to empty source")
            .source(SkIRect::MakeEmpty(), SkColors::kRed)
            .applyTransform(SkMatrix::Translate(10.f, 10.f), Expect::kEmptyImage)
            .desiredOutput({10, 10, 20, 20})
            .run();
}

DEF_TEST_SUITE(EmptyDesiredOutput, r) {
    TestCase(r, "applyCrop() + empty output becomes empty")
            .source({0, 0, 10, 10}, SkColors::kRed)
            .applyCrop({2, 2, 8, 8}, Expect::kEmptyImage)
            .desiredOutput(SkIRect::MakeEmpty())
            .run();

    TestCase(r, "applyTransform() + empty output becomes empty")
            .source({0, 0, 10, 10}, SkColors::kRed)
            .applyTransform(SkMatrix::RotateDeg(10.f), Expect::kEmptyImage)
            .desiredOutput(SkIRect::MakeEmpty())
            .run();
}

DEF_TEST_SUITE(Crop, r) {
    TestCase(r, "applyCrop() contained in source and output")
            .source({0, 0, 20, 20}, SkColors::kGreen)
            .applyCrop({8, 8, 12, 12}, Expect::kDeferredImage)
            .desiredOutput({4, 4, 16, 16})
            .run();

    TestCase(r, "applyCrop() contained in source, intersects output")
            .source({0, 0, 20, 20}, SkColors::kGreen)
            .applyCrop({4, 4, 12, 12}, Expect::kDeferredImage)
            .desiredOutput({8, 8, 16, 16})
            .run();

    TestCase(r, "applyCrop() intersects source, contained in output")
            .source({10, 10, 20, 20}, SkColors::kGreen)
            .applyCrop({4, 4, 16, 16}, Expect::kDeferredImage)
            .desiredOutput({0, 0, 20, 20})
            .run();

    TestCase(r, "applyCrop() intersects source and output")
            .source({0, 0, 10, 10}, SkColors::kGreen)
            .applyCrop({5, -5, 15, 5}, Expect::kDeferredImage)
            .desiredOutput({7, -2, 12, 8})
            .run();

    TestCase(r, "applyCrop() contains source and output")
            .source({0, 0, 10, 10}, SkColors::kGreen)
            .applyCrop({-5, -5, 15, 15}, Expect::kDeferredImage)
            .desiredOutput({1, 1, 9, 9})
            .run();

    TestCase(r, "applyCrop() contains source, intersects output")
            .source({4, 4, 16, 16}, SkColors::kGreen)
            .applyCrop({0, 0, 20, 20}, Expect::kDeferredImage)
            .desiredOutput({-5, -5, 18, 18})
            .run();

    TestCase(r, "applyCrop() intersects source, contains output")
            .source({0, 0, 20, 20}, SkColors::kGreen)
            .applyCrop({-5, 5, 25, 15}, Expect::kDeferredImage)
            .desiredOutput({0, 5, 20, 15})
            .run();
}

DEF_TEST_SUITE(CropDisjointFromSourceAndOutput, r) {
    TestCase(r, "applyCrop() disjoint from source, intersects output")
            .source({0, 0, 10, 10}, SkColors::kBlue)
            .applyCrop({11, 11, 20, 20}, Expect::kEmptyImage)
            .desiredOutput({0, 0, 15, 15})
            .run();

    TestCase(r, "applyCrop() disjoint from source, intersects output disjoint from source")
            .source({0, 0, 10, 10}, SkColors::kBlue)
            .applyCrop({11, 11, 20, 20}, Expect::kEmptyImage)
            .desiredOutput({12, 12, 18, 18})
            .run();

    TestCase(r, "applyCrop() intersects source, disjoint from output")
            .source({0, 0, 10, 10}, SkColors::kBlue)
            .applyCrop({-5, -5, 5, 5}, Expect::kEmptyImage)
            .desiredOutput({6, 6, 12, 12})
            .run();

    TestCase(r, "applyCrop() intersects source, disjoint from output disjoint from source")
            .source({0, 0, 10, 10}, SkColors::kBlue)
            .applyCrop({-5, -5, 5, 5}, Expect::kEmptyImage)
            .desiredOutput({12, 12, 18, 18})
            .run();

    TestCase(r, "applyCrop() disjoint from source and output")
            .source({0, 0, 10, 10}, SkColors::kBlue)
            .applyCrop({12, 12, 18, 18}, Expect::kEmptyImage)
            .desiredOutput({-1, -1, 11, 11})
            .run();

    TestCase(r, "applyCrop() disjoint from source and output disjoint from source")
            .source({0, 0, 10, 10}, SkColors::kBlue)
            .applyCrop({-10, 10, -1, -1}, Expect::kEmptyImage)
            .desiredOutput({11, 11, 20, 20})
            .run();
}

DEF_TEST_SUITE(EmptyCrop, r) {
    TestCase(r, "applyCrop() is empty")
            .source({0, 0, 10, 10}, SkColors::kYellow)
            .applyCrop(SkIRect::MakeEmpty(), Expect::kEmptyImage)
            .desiredOutput({0, 0, 10, 10})
            .run();

    TestCase(r, "applyCrop() emptiness propagates")
            .source({0, 0, 10, 10}, SkColors::kYellow)
            .applyCrop({1, 1, 9, 9}, Expect::kDeferredImage)
            .applyCrop(SkIRect::MakeEmpty(), Expect::kEmptyImage)
            .desiredOutput({0, 0, 10, 10})
            .run();
}

DEF_TEST_SUITE(DisjointCrops, r) {
    TestCase(r, "Disjoint applyCrops() become empty")
            .source({0, 0, 10, 10}, SkColors::kCyan)
            .applyCrop({0, 0, 4, 4}, Expect::kDeferredImage)
            .applyCrop({6, 6, 10, 10}, Expect::kEmptyImage)
            .desiredOutput({0, 0, 10, 10})
            .run();
}

DEF_TEST_SUITE(IntersectingCrops, r) {
    TestCase(r, "Consecutive applyCrops() combine")
            .source({0, 0, 20, 20}, SkColors::kMagenta)
            .applyCrop({5, 5, 15, 15}, Expect::kDeferredImage)
            .applyCrop({10, 10, 20, 20}, Expect::kDeferredImage)
            .desiredOutput({0, 0, 20, 20})
            .run();
}

DEF_TEST_SUITE(Transform, r) {
    TestCase(r, "applyTransform() integer translate")
            .source({0, 0, 10, 10}, SkColors::kRed)
            .applyTransform(SkMatrix::Translate(5, 5), Expect::kDeferredImage)
            .desiredOutput({0, 0, 10, 10})
            .run();

    TestCase(r, "applyTransform() fractional translate")
            .source({0, 0, 10, 10}, SkColors::kRed)
            .applyTransform(SkMatrix::Translate(1.5f, 3.24f), Expect::kDeferredImage)
            .desiredOutput({0, 0, 10, 10})
            .run();

    TestCase(r, "applyTransform() scale")
            .source({0, 0, 4, 4}, SkColors::kRed)
            .applyTransform(SkMatrix::Scale(2.2f, 3.5f), Expect::kDeferredImage)
            .desiredOutput({0, 0, 8, 8})
            .run();

    TestCase(r, "applyTransform() with complex transform")
            .source({0, 0, 8, 8}, SkColors::kRed)
            .applyTransform(SkMatrix::RotateDeg(10.f, {4.f, 4.f}), Expect::kDeferredImage)
            .desiredOutput({0, 0, 16, 16})
            .run();
}

DEF_TEST_SUITE(CompatibleSamplingConcatsTransforms, r) {
    TestCase(r, "linear + linear combine")
            .source({0, 0, 8, 8}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kLinear}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kLinear}, Expect::kDeferredImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "equiv. bicubics combine")
            .source({0, 0, 8, 8}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkCubicResampler::Mitchell()}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkCubicResampler::Mitchell()}, Expect::kDeferredImage)
            .desiredOutput({0, 0, 16, 16})
            .run();


    TestCase(r, "linear + bicubic becomes bicubic")
            .source({0, 0, 8, 8}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kLinear}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkCubicResampler::Mitchell()}, Expect::kDeferredImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "bicubic + linear becomes bicubic")
            .source({0, 0, 8, 8}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkCubicResampler::Mitchell()}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kLinear}, Expect::kDeferredImage,
                            SkSamplingOptions{SkCubicResampler::Mitchell()})
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "aniso picks max level to combine")
            .source({0, 0, 8, 8}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(4.f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(2.f), Expect::kDeferredImage,
                            SkSamplingOptions::Aniso(4.f))
            .desiredOutput({0, 0, 16, 16})
            .run();
    TestCase(r, "aniso picks max level to combine (other direction)")
            .source({0, 0, 8, 8}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(2.f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(4.f), Expect::kDeferredImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "linear + aniso becomes aniso")
            .source({0, 0, 8, 8}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kLinear}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(2.f), Expect::kDeferredImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "aniso + linear stays aniso")
            .source({0, 0, 8, 8}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(4.f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kLinear}, Expect::kDeferredImage,
                            SkSamplingOptions::Aniso(4.f))
            .desiredOutput({0, 0, 16, 16})
            .run();

    // TODO: Add cases for mipmapping once that becomes relevant (SkSpecialImage does not have
    // mipmaps right now).
}

DEF_TEST_SUITE(IncompatibleSamplingResolvesImages, r) {
    TestCase(r, "different bicubics do not combine")
            .source({0, 0, 8, 8}, SkColors::kBlue)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkCubicResampler::Mitchell()}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkCubicResampler::CatmullRom()}, Expect::kNewImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "nearest + linear do not combine")
            .source({0, 0, 8, 8}, SkColors::kBlue)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kNearest}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kLinear}, Expect::kNewImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "linear + nearest do not combine")
            .source({0, 0, 8, 8}, SkColors::kBlue)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kLinear}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kNearest}, Expect::kNewImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "bicubic + aniso do not combine")
            .source({0, 0, 8, 8}, SkColors::kBlue)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkCubicResampler::Mitchell()}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(4.f), Expect::kNewImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "aniso + bicubic do not combine")
            .source({0, 0, 8, 8}, SkColors::kBlue)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(4.f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkCubicResampler::Mitchell()}, Expect::kNewImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "nearest + nearest do not combine")
            .source({0, 0, 8, 8}, SkColors::kBlue)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kNearest}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkFilterMode::kNearest}, Expect::kNewImage)
            .desiredOutput({0, 0, 16, 16})
            .run();
}

DEF_TEST_SUITE(IntegerOffsetIgnoresNearestSampling, r) {
    // Bicubic is used here to reflect that it should use the non-NN sampling and just needs to be
    // something other than the default to detect that it got carried through.
    TestCase(r, "integer translate+NN then bicubic combines")
            .source({0, 0, 8, 8}, SkColors::kCyan)
            .applyTransform(SkMatrix::Translate(2, 2),
                            SkSamplingOptions{SkFilterMode::kNearest}, Expect::kDeferredImage,
                            FilterResult::kDefaultSampling)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkCubicResampler::Mitchell()}, Expect::kDeferredImage)
            .desiredOutput({0, 0, 16, 16})
            .run();
    TestCase(r, "bicubic then integer translate+NN combines")
    .source({0, 0, 8, 8}, SkColors::kCyan)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions{SkCubicResampler::Mitchell()}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::Translate(2, 2),
                            SkSamplingOptions{SkFilterMode::kNearest}, Expect::kDeferredImage,
                            SkSamplingOptions{SkCubicResampler::Mitchell()})
            .desiredOutput({0, 0, 16, 16})
            .run();
}

DEF_TEST_SUITE(TransformBecomesEmpty, r) {
    TestCase(r, "Transform beyond output")
            .source({0, 0, 8, 8}, SkColors::kMagenta)
            .applyTransform(SkMatrix::Translate(10.f, 10.f), Expect::kEmptyImage)
            .desiredOutput({0, 0, 8, 8})
            .run();

    TestCase(r, "Transform avoids crop")
            .source({0, 0, 8, 8}, SkColors::kMagenta)
            .applyTransform(SkMatrix::Translate(10.f, 10.f), Expect::kDeferredImage)
            .applyCrop({2, 2, 6, 6}, Expect::kEmptyImage)
            .desiredOutput({0, 0, 20, 20})
            .run();

    TestCase(r, "Transform moves crop offscreen")
            .source({0, 0, 8, 8}, SkColors::kMagenta)
            .applyCrop({1, 1, 4, 4}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::Translate(-5.f, -5.f), Expect::kEmptyImage)
            .desiredOutput({0, 0, 8, 8})
            .run();
}

DEF_TEST_SUITE(TransformAndCrop, r) {
    TestCase(r, "Crop after transform can always apply")
            .source({0, 0, 16, 16}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(45.f, {3.f, 4.f}), Expect::kDeferredImage)
            .applyCrop({2, 2, 15, 15}, Expect::kDeferredImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    // TODO: Expand this test case to be arbitrary float S+T transforms when FilterResult tracks
    // both a srcRect and dstRect.
    TestCase(r, "Crop after translate is lifted to image subset")
            .source({0, 0, 16, 16}, SkColors::kGreen)
            .applyTransform(SkMatrix::Translate(6.f, 4.f), Expect::kDeferredImage)
            .applyCrop({8, 8, 12, 12}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(45.f, {8.f, 8.f}), Expect::kDeferredImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "Transform after unlifted crop triggers new image")
            .source({0, 0, 16, 16}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(45.f, {8.f, 8.f}), Expect::kDeferredImage)
            .applyCrop({1, 1, 15, 15}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(-10.f, {8.f, 4.f}), Expect::kNewImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "Transform after unlifted crop with interior output does not trigger new image")
            .source({0, 0, 16, 16}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(45.f, {8.f, 8.f}), Expect::kDeferredImage)
            .applyCrop({1, 1, 15, 15}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(-10.f, {8.f, 4.f}), Expect::kDeferredImage)
            .desiredOutput({4, 4, 12, 12})
            .run();

    TestCase(r, "Translate after unlifted crop does not trigger new image")
            .source({0, 0, 16, 16}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(5.f, {8.f, 8.f}), Expect::kDeferredImage)
            .applyCrop({2, 2, 14, 14}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::Translate(4.f, 6.f), Expect::kDeferredImage)
            .desiredOutput({0, 0, 16, 16})
            .run();

    TestCase(r, "Transform after large no-op crop does not trigger new image")
            .source({0, 0, 64, 64}, SkColors::kGreen)
            .applyTransform(SkMatrix::RotateDeg(45.f, {32.f, 32.f}), Expect::kDeferredImage)
            .applyCrop({-64, -64, 128, 128}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(-30.f, {32.f, 32.f}), Expect::kDeferredImage)
            .desiredOutput({0, 0, 64, 64})
            .run();
}



} // anonymous namespace
