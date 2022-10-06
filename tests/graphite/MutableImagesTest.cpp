/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_GRAPHITE_ENABLED

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

using namespace skgpu::graphite;

namespace {

// We draw the larger image into the smaller surface to force mipmapping
const SkISize kImageSize = { 32, 32 };
constexpr int kNumMipLevels = 6;
const SkISize kSurfaceSize = { 16, 16 };

/*
 * We have 4 use cases. In each case there is a mutating task which changes the contents of an
 * image somehow and a shared redraw task which just creates a single Recording which draws the
 * image that is being mutated. The mutation tasks are:

 *  1) The client has wrapped a backend texture in an image and is changing the backend texture's
 *     contents.
 *  2) The client has a pool of backend textures and updates which one backs the image every frame
 *  3) The client has a surface and image which both wrap the same backend texture
 *  4) The client has a surface and has snapped an image w/o a copy but keeps drawing to the
 *     surface
 *
 * There are also two scenarios for the mutation and redrawing tasks:
 *  a) Both are on the same thread and use the same recorder
 *  b) They are on different threads and use separate recorders
 * The latter, obviously, requires more synchronization.
 */
class Mutator {
public:
    Mutator(skiatest::Reporter* reporter, Recorder* recorder)
            : fReporter(reporter)
            , fRecorder(recorder) {
    }

    virtual void init(Context*, SkColor4f initColor) = 0;
    virtual void mutate(Context*, SkColor4f newColor) = 0;
    virtual SkImage* getMutatingImage() = 0;

    virtual ~Mutator() = default;

protected:
    skiatest::Reporter* fReporter;
    Recorder* fRecorder;
};

class Redrawer {
public:
    Redrawer(skiatest::Reporter* reporter) : fReporter(reporter) {
        SkImageInfo ii = SkImageInfo::Make(kSurfaceSize,
                                           kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);
        fReadbackBM.allocPixels(ii);
        SkAssertResult(fReadbackBM.peekPixels(&fReadbackPM));
    }

    void init(Recorder* recorder, Mutator* mutator) {
        SkImageInfo ii = SkImageInfo::Make(kSurfaceSize,
                                           kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);

        fImageDrawSurface = SkSurface::MakeGraphite(recorder, ii, Mipmapped::kYes);
        REPORTER_ASSERT(fReporter, fImageDrawSurface);

        SkSamplingOptions sampling = SkSamplingOptions(SkFilterMode::kLinear,
                                                       SkMipmapMode::kNearest);

        SkCanvas* canvas = fImageDrawSurface->getCanvas();

        canvas->drawImageRect(mutator->getMutatingImage(),
                              SkRect::MakeWH(kSurfaceSize.width(), kSurfaceSize.height()),
                              sampling);

        fImgDrawRecording = recorder->snap();
    }

    void redraw(Context* context) {
        skgpu::graphite::InsertRecordingInfo info;
        info.fRecording = fImgDrawRecording.get();
        context->insertRecording(info);
    }

    SkSurface* destSurface() {
        return fImageDrawSurface.get();
    }

    void checkResult(Context* context,
                     Recorder* recorder,
                     int testcaseID,
                     char testScenario,
                     const SkColor4f& expectedColor) {

        // For now, we cast and call directly into Surface. Once we have a better idea of
        // what the public API for synchronous graphite readPixels we can update this call to use
        // that instead.
        auto surface = static_cast<skgpu::graphite::Surface*>(fImageDrawSurface.get());
        if (!surface->onReadPixels(context, recorder, fReadbackPM, 0, 0)) {
            ERRORF(fReporter, "readPixels failed");
        }

        auto error = std::function<ComparePixmapsErrorReporter>(
                [&](int x, int y, const float diffs[4]) {
                    ERRORF(fReporter,
                           "case %d%c: expected (%.1f %.1f %.1f %.1f) got (%.1f, %.1f, %.1f, %.1f)",
                           testcaseID, testScenario,
                           expectedColor.fR, expectedColor.fG, expectedColor.fB, expectedColor.fA,
                           expectedColor.fR-diffs[0], expectedColor.fG-diffs[1],
                           expectedColor.fB-diffs[2], expectedColor.fA-diffs[3]);
                });

        static constexpr float kTol[] = {0, 0, 0, 0};
        CheckSolidPixels(expectedColor, fReadbackPM, kTol, error);
    }
private:
    skiatest::Reporter* fReporter;
    sk_sp<SkSurface> fImageDrawSurface;
    std::unique_ptr<Recording> fImgDrawRecording;

    SkBitmap fReadbackBM;
    SkPixmap fReadbackPM;
};

void update_backend_texture(Recorder* recorder,
                            const BackendTexture& backendTex,
                            SkColorType ct,
                            bool withMips,
                            SkColor4f color) {
    SkPixmap pixmaps[6];
    std::unique_ptr<char[]> memForPixmaps;

    const SkColor4f colors[6] = { color, color, color, color, color, color };

    int numMipLevels = ToolUtils::make_pixmaps(ct, kPremul_SkAlphaType, withMips, colors, pixmaps,
                                               &memForPixmaps);
    SkASSERT(numMipLevels == 1 || numMipLevels == kNumMipLevels);
    SkASSERT(kImageSize == pixmaps[0].dimensions());

    recorder->updateBackendTexture(backendTex, pixmaps, numMipLevels);
}

// case 1
class UpdateBackendTextureMutator : public Mutator {
public:
    static std::unique_ptr<Mutator> Make(skiatest::Reporter* reporter,
                                         Recorder* recorder) {
        return std::make_unique<UpdateBackendTextureMutator>(reporter, recorder);
    }

    UpdateBackendTextureMutator(skiatest::Reporter* reporter, Recorder* recorder)
            : Mutator(reporter, recorder) {
    }
    ~UpdateBackendTextureMutator() override {
        fRecorder->deleteBackendTexture(fBETexture);
    }

    void init(Context* context, SkColor4f initColor) override {
        auto caps = context->priv().caps();

        // Note: not renderable
        // TODO: exercise mipmaps!
        TextureInfo info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                              /*levelCount=*/ 1,
                                                              skgpu::Protected::kNo,
                                                              Renderable::kNo);
        REPORTER_ASSERT(fReporter, info.isValid());

        fBETexture = fRecorder->createBackendTexture(kImageSize, info);
        REPORTER_ASSERT(fReporter, fBETexture.isValid());

        update_backend_texture(fRecorder, fBETexture, kRGBA_8888_SkColorType,
                               /* withMips= */ false, initColor);

        fMutatingImg = SkImage::MakeGraphiteFromBackendTexture(fRecorder,
                                                               fBETexture,
                                                               kRGBA_8888_SkColorType,
                                                               kPremul_SkAlphaType,
                                                               /* colorSpace= */ nullptr);
    }

    void mutate(Context* context, SkColor4f newColor) override {
        update_backend_texture(fRecorder, fBETexture, kRGBA_8888_SkColorType,
                               /* withMips= */ false, newColor);
    }

    SkImage* getMutatingImage() override {
        return fMutatingImg.get();
    }

private:
    sk_sp<SkImage> fMutatingImg;
    BackendTexture fBETexture;
};

// case 4
class SurfaceMutator : public Mutator {
public:
    static std::unique_ptr<Mutator> Make(skiatest::Reporter* reporter,
                                         Recorder* recorder) {
        return std::make_unique<SurfaceMutator>(reporter, recorder);
    }

    SurfaceMutator(skiatest::Reporter* reporter, Recorder* recorder) : Mutator(reporter, recorder) {}

    void init(Context* context, SkColor4f initColor) override {
        SkImageInfo ii = SkImageInfo::Make(kImageSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        fMutatingSurface = SkSurface::MakeGraphite(fRecorder, ii);
        REPORTER_ASSERT(fReporter, fMutatingSurface);

        fMutatingSurface->getCanvas()->clear(initColor);

        fMutatingImg = fMutatingSurface->experimental_makeImageSnapshotNoCopy();
        REPORTER_ASSERT(fReporter, fMutatingImg);

        std::unique_ptr<Recording> imgCreationRecording = fRecorder->snap();

        skgpu::graphite::InsertRecordingInfo info;
        info.fRecording = imgCreationRecording.get();
        context->insertRecording(info);
    }

    void mutate(Context* context, SkColor4f newColor) override {
        fMutatingSurface->getCanvas()->clear(newColor);

        std::unique_ptr<Recording> recording = fRecorder->snap();

        skgpu::graphite::InsertRecordingInfo info;
        info.fRecording = recording.get();
        context->insertRecording(info);
    }

    SkImage* getMutatingImage() override {
        return fMutatingImg.get();
    }

private:
    sk_sp<SkSurface> fMutatingSurface;
    sk_sp<SkImage> fMutatingImg;
};

using FactoryT = std::unique_ptr<Mutator> (*)(skiatest::Reporter*, Recorder*);

void run_test(skiatest::Reporter* reporter,
              Context* context,
              int testCase,
              char testScenario,
              FactoryT f) {

    auto recorder = context->makeRecorder();

    std::unique_ptr<Mutator> mutator = f(reporter, recorder.get());

    mutator->init(context, SkColors::kRed);

    Redrawer redrawer(reporter);

    redrawer.init(recorder.get(), mutator.get());

    redrawer.redraw(context);
    redrawer.checkResult(context, recorder.get(), testCase, testScenario, SkColors::kRed);

    for (auto newColor : { SkColors::kGreen, SkColors::kBlue }) {
        mutator->mutate(context, newColor);
        redrawer.redraw(context);
        redrawer.checkResult(context, recorder.get(), testCase, testScenario, newColor);
    }
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(MutableImagesTest, reporter, context) {
    run_test(reporter, context, 1, 'a', UpdateBackendTextureMutator::Make);
    run_test(reporter, context, 4, 'a', SurfaceMutator::Make);
}

#endif // SK_GRAPHITE_ENABLED
