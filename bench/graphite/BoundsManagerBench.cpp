/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkRandom.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"

#if defined(SK_ENABLE_SVG)
#include "tools/SvgPathExtractor.h"
#endif

using namespace skia_private;

static DEFINE_string(boundsManagerFile, "",
                     "svg or skp for the BoundsManager bench to sniff paths from.");

#define PRINT_DRAWSET_COUNT 0 // set to 1 to display number of CompressedPaintersOrder groups

namespace skgpu::graphite {

class BoundsManagerBench : public Benchmark {
public:
    BoundsManagerBench(std::unique_ptr<BoundsManager> manager) : fManager(std::move(manager)) {}

protected:
    virtual void gatherRects(TArray<SkRect>* rects) = 0;

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    const char* onGetName() final { return fName.c_str(); }

    void onDelayedSetup() final {
        TArray<SkRect> rects;
        this->gatherRects(&rects);

        fRectCount = rects.size();
        fRects = fAlignedAllocator.makeArray<Rect>(fRectCount);
        for (int i = 0; i < fRectCount; ++i) {
            fRects[i] = rects[i];
        }
    }

    void onDraw(int loops, SkCanvas*) final {
        for (int i = 0; i < loops; ++i) {
            this->doBench();
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
#if PRINT_DRAWSET_COUNT
        SkDebugf("%s >> grouped %d draws into %d sets <<\n",
                 fName.c_str(), fRectCount, fMaxRead.bits());
#endif
    }

    void doBench() {
        CompressedPaintersOrder maxRead = CompressedPaintersOrder::First();
        for (int i = 0; i < fRectCount; ++i) {
            const Rect& drawBounds = fRects[i];
            CompressedPaintersOrder order = fManager->getMostRecentDraw(drawBounds).next();
            fManager->recordDraw(drawBounds, order);
            if (order > maxRead) {
                maxRead = order;
            }
        }

        fMaxRead = maxRead;
        fManager->reset();
    }

    std::unique_ptr<BoundsManager> fManager;
    SkString fName;
    SkArenaAlloc fAlignedAllocator{0};
    int fRectCount;
    Rect* fRects;

    CompressedPaintersOrder fMaxRead;
};

class RandomBoundsManagerBench : public BoundsManagerBench {
public:
    RandomBoundsManagerBench(std::unique_ptr<BoundsManager> manager,
                             const char* managerName,
                             int numRandomRects)
            : BoundsManagerBench(std::move(manager))
            , fNumRandomRects(numRandomRects) {
        fName.printf("BoundsManager_rand_%i_%s", numRandomRects, managerName);
    }

private:
    void gatherRects(TArray<SkRect>* rects) override {
        SkRandom rand;
        for (int i = 0; i < fNumRandomRects; ++i) {
            rects->push_back(SkRect::MakeXYWH(rand.nextRangeF(0, 2000),
                                              rand.nextRangeF(0, 2000),
                                              rand.nextRangeF(0, 70),
                                              rand.nextRangeF(0, 70)));
        }
    }

    int fNumRandomRects;
};

class FileBoundsManagerBench : public BoundsManagerBench {
public:
    FileBoundsManagerBench(std::unique_ptr<BoundsManager> manager,
                           const char* managerName)
            : BoundsManagerBench(std::move(manager)) {
        if (!FLAGS_boundsManagerFile.isEmpty()) {
            const char* filename = strrchr(FLAGS_boundsManagerFile[0], '/');
            if (filename) {
                ++filename;
            } else {
                filename = FLAGS_boundsManagerFile[0];
            }
            fName.printf("BoundsManager_file_%s_%s", filename, managerName);
        }
    }

private:
    bool isSuitableFor(Backend backend) final {
        if (FLAGS_boundsManagerFile.isEmpty()) {
            return false;
        }
        return BoundsManagerBench::isSuitableFor(backend);
    }

    void gatherRects(TArray<SkRect>* rects) override {
        if (FLAGS_boundsManagerFile.isEmpty()) {
            return;
        }
        SkRect fileBounds = SkRect::MakeEmpty();
        auto callback = [&](const SkMatrix& matrix,
                            const SkPath& path,
                            const SkPaint& paint) {
            if (!paint.canComputeFastBounds() || path.isInverseFillType()) {
                // These would pessimistically cover the entire canvas, but we don't have enough
                // info in the benchmark to handle that, so just skip these draws.
                return;
            }

            SkRect bounds = path.getBounds();
            SkRect drawBounds = matrix.mapRect(paint.computeFastBounds(bounds, &bounds));
            rects->push_back(drawBounds);

            fileBounds.join(drawBounds);
        };

        const char* path = FLAGS_boundsManagerFile[0];
        if (const char* ext = strrchr(path, '.'); ext && !strcmp(ext, ".svg")) {
#if defined(SK_ENABLE_SVG)
            ToolUtils::ExtractPathsFromSVG(path, callback);
#else
            SK_ABORT("must compile with svg backend to process svgs");
#endif
        } else {
            ToolUtils::ExtractPathsFromSKP(path, callback);
        }

#if PRINT_DRAWSET_COUNT
        SkDebugf("%s bounds are [%f %f %f %f]\n",
            FLAGS_boundsManagerFile[0],
            fileBounds.fLeft, fileBounds.fTop, fileBounds.fRight, fileBounds.fBottom);
#endif
    }

};

}  // namespace skgpu::graphite

#define DEF_BOUNDS_MANAGER_BENCH_SET(manager, name) \
    DEF_BENCH( return new skgpu::graphite::RandomBoundsManagerBench(manager, name, 100); ) \
    DEF_BENCH( return new skgpu::graphite::RandomBoundsManagerBench(manager, name, 500); ) \
    DEF_BENCH( return new skgpu::graphite::RandomBoundsManagerBench(manager, name, 1000); ) \
    DEF_BENCH( return new skgpu::graphite::RandomBoundsManagerBench(manager, name, 10000); ) \
    DEF_BENCH( return new skgpu::graphite::FileBoundsManagerBench(manager, name); )


DEF_BOUNDS_MANAGER_BENCH_SET(std::make_unique<skgpu::graphite::NaiveBoundsManager>(),      "naive")
DEF_BOUNDS_MANAGER_BENCH_SET(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "brute")
DEF_BOUNDS_MANAGER_BENCH_SET(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 128), "grid128")
DEF_BOUNDS_MANAGER_BENCH_SET(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 512), "grid512")
DEF_BOUNDS_MANAGER_BENCH_SET(std::make_unique<skgpu::graphite::HybridBoundsManager>(SkISize{1800, 1800}, 16, 64), "hybrid16x16n128")
DEF_BOUNDS_MANAGER_BENCH_SET(std::make_unique<skgpu::graphite::HybridBoundsManager>(SkISize{1800, 1800}, 16, 128), "hybrid16x16n256")
// Uncomment and adjust device size to match reported bounds from --boundsManagerFile
// DEF_BOUNDS_MANAGER_BENCH_SET(skgpu::graphite::GridBoundsManager::MakeRes({w, h}, 8), "gridRes8")

#undef DEF_BOUNDS_MANAGER_BENCH_SET



///////////////////////////////////////////////////////////////////////////////////////////////////

#include "src/gpu/graphite/geom/Rect.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkArenaAllocList.h"
#include "src/sksl/SkSLString.h"
#include "src/gpu/graphite/geom/IntersectionTree.h"

#include <algorithm>
#include <memory>

namespace skgpu::graphite {

// Stand-in for Renderer and implicitly RenderSteps
enum class MockRenderType {
    kNonAACoverageOnly,  // e.g. Convex/stroke path rendering or non-aa bounds filling
    kAnalyticCoverageOnly, // e.g. AA Rect, Per-edge AA quads, or text
    kMultiStepWithStencil, // e.g. complex path rendering with disjoint stencil draws
    kInnerFillRect, // e.g. non-AA fill step + analytic AA fill with the same Z value
};
static constexpr int kRenderTypeCount = 4;
static constexpr int kMaxRenderSteps = 2;

// Stand-in for DrawParams and DrawList::Draw, ignoring clipping for now
struct MockDraw {
    MockRenderType fRenderType;

    Rect fBounds;
    int fZ;

    int fPaintID;
    int fDataBinding;
    bool fIsPaintOpaque;

    int stepCount() const {
        if (fRenderType == MockRenderType::kNonAACoverageOnly ||
            fRenderType == MockRenderType::kAnalyticCoverageOnly) {
            return 1;
        } else if (fRenderType == MockRenderType::kInnerFillRect) {
            return fIsPaintOpaque ? 2 : 1; // Only one extra step if the paint is opaque
        } else {
            // Stencil count step + cover step
            return 2;
        }
    }

    int pipelineIndex(int step) const {
        SkASSERT(step < this->stepCount());
        int pipelineOffset = (int) fRenderType * kMaxRenderSteps + step;
        return fPaintID * kMaxRenderSteps * kRenderTypeCount + pipelineOffset;
    }

    bool dependsOnDst(int step) const {
        return !fIsPaintOpaque ||
               fRenderType == MockRenderType::kAnalyticCoverageOnly ||
               (fRenderType == MockRenderType::kInnerFillRect && step == 1);
    }
};

// Stand-in for DrawPass::Make's CommandList generation
class MockRenderer {
public:
    MockRenderer() = default;

    void draw(const MockDraw& draw, int step) {
        SkASSERT(step < draw.stepCount());

        fBusyWork += draw.fZ * step;

        if (fFirstPass) {
            // Validate that the incoming draw would not violate painter's order by an already
            // submitted draw if it depends on the dst.
            Rect::ComplementRect drawBounds{draw.fBounds};
            if (draw.dependsOnDst(step)) {
                for (auto [d, s] : fDraws) {
                    if (d->fBounds.intersects(drawBounds) &&
                        d->fZ > draw.fZ && d->dependsOnDst(s)) {
                        SK_ABORT("Painter's order violation between recorded draw %d|%d and "
                                 "new draw %d|%d\n", d->fZ, s, draw.fZ, step);
                    }
                }
            }

            // Validate that no intermediate draws corrupted the stencil rendering between first
            // and second steps for a kMultiStepWithStencil draw.
            if (draw.fRenderType == MockRenderType::kMultiStepWithStencil && step == 1) {
                bool stencilDrawFound = false;
                for (auto [d, s] : fDraws) {
                    if (d == &draw) {
                        SkASSERT_RELEASE(!stencilDrawFound && s == 0);
                        stencilDrawFound = true;
                    } else {
                        SkASSERT_RELEASE(d->fZ != draw.fZ);
                        if (stencilDrawFound &&
                            d->fRenderType == MockRenderType::kMultiStepWithStencil &&
                            d->fBounds.intersects(drawBounds)) {
                            SK_ABORT("Buffer corrupted between stencil and cover for %d by "
                                     "recorded draw %d|%d\n", draw.fZ, d->fZ, s);
                        }
                    }
                }
                SkASSERT_RELEASE(stencilDrawFound);
            }

            // Track "state" changes from this submitted draw ordering
            if (fDraws.empty()) {
                fNumPipelineChanges++;
            } else {
                auto [lastDraw, lastStep] = fDraws.back();
                if (draw.pipelineIndex(step) != lastDraw->pipelineIndex(lastStep)) {
                    fNumPipelineChanges++;
                } else if (draw.fDataBinding != lastDraw->fDataBinding) {
                    fNumBindingChanges++;
                }
            }

            // SkDebugf("draw %p (%dx%d) with %d + %d  (pipeline %d) in %f %f %f %f\n", &draw,
            //     (int) draw.fRenderType, step, draw.fPaintID, draw.fDataBinding,
            //     draw.pipelineIndex(step),
            //     draw.fBounds.left(), draw.fBounds.top(), draw.fBounds.right(), draw.fBounds.bot());

            fDraws.push_back({&draw, step});
        }
    }

    void endRenderPass(const char* label) {
        if (fFirstPass) {
            // SkDebugf("# recorded steps: %d, # pipeline changes: %d, # binding changes: %d\n",
            //          fDraws.size(), fNumPipelineChanges, fNumBindingChanges);
            // SkDebugf("%d,%s\n", fNumPipelineChanges + fNumBindingChanges, label);
            fDraws.clear();
        }

        fBusyWork = 0;
        fFirstPass = false;
    }

private:
    int fBusyWork = 0;

    // Stats and validation is tracked only on the first "render" then all the timed loops are
    // simply sums of the z value and step coming in to primarily measure cost of accessing the
    // sorted draw data.
    bool fFirstPass = true;

    skia_private::TArray<std::pair<const MockDraw*, int /*step*/>> fDraws;
    int fNumPipelineChanges = 0;
    int fNumBindingChanges = 0;
};

// Stand-in for SkDevice implementation that collects draw calls and then renders in bulk.
class MockDevice {
public:
    virtual ~MockDevice() = default;

    virtual std::string name() const = 0;

    virtual void recordDraw(const MockDraw& draw) = 0;

    virtual void render(MockRenderer* renderer) = 0;

    virtual void reset() = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

// Determines how generated draws are assigned paints
class PaintGenerator {
public:
    virtual ~PaintGenerator() = default;

    virtual std::string name() const = 0;

    virtual std::tuple<int /*paintID*/, int /*binding*/, bool /*paintIsOpaque*/> nextPaint() = 0;
};

// Determines how generated draws are geometrically arranged and their rendering type.
class GeometryGenerator {
public:
    virtual ~GeometryGenerator() = default;

    virtual std::string name() const = 0;

    virtual std::pair<MockRenderType, Rect /*bounds*/> nextGeometry() = 0;
};

class DrawGenerator {
public:
    DrawGenerator(const char* name,
                  std::unique_ptr<GeometryGenerator> geomGen,
                  std::unique_ptr<PaintGenerator> paintGen,
                  int numDraws)
            : fGeomGenerator(std::move(geomGen))
            , fPaintGenerator(std::move(paintGen))
            , fNumDraws(numDraws)
            , fName(name) {}

    std::string name() const {
        return SkSL::String::printf("%s,%d", fName, fNumDraws);
        // return SkSL::String::printf("geom(%s)-paint(%s)-%d",
        //                             fGeomGenerator->name().c_str(),
        //                             fPaintGenerator->name().c_str(),
        //                             fNumDraws);
    }

    int numDraws() const { return fNumDraws; }

    MockDraw nextDraw() {
        SkASSERT(fCurrentDraw < fNumDraws);

        auto [type, bounds] = fGeomGenerator->nextGeometry();
        auto [paintID, binding, isOpaque] = fPaintGenerator->nextPaint();

        // SkDebugf("draw %f %f %f %f, %d %d\n",
        //     bounds.left(), bounds.top(), bounds.right(), bounds.bot(),
        //     paintID, binding);

        return MockDraw{type, bounds, fCurrentDraw++, paintID, binding, isOpaque};
    }

private:
    std::unique_ptr<GeometryGenerator> fGeomGenerator;
    std::unique_ptr<PaintGenerator> fPaintGenerator;
    const int fNumDraws;
    const char* fName;

    int fCurrentDraw = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class RandomPaintGenerator : public PaintGenerator {
public:
    RandomPaintGenerator(int numPipelines, int numBindingsPerPipeline, float opaqueProbability)
            : fNumPipelines(numPipelines)
            , fNumBindings(numBindingsPerPipeline)
            , fOpaqueProbability(opaqueProbability) {}
    std::string name() const override {
        return SkSL::String::printf("rand(%dx%d @ %.1f%%)",
                                    fNumPipelines, fNumBindings, 100.f*fOpaqueProbability);
    }

    std::tuple<int, int, bool> nextPaint() override {
        uint32_t paintID = fRandom.nextULessThan(fNumPipelines);
        uint32_t binding = fRandom.nextULessThan(fNumBindings);

        uint64_t opaqueKey = (((uint64_t) paintID) << 32) | binding;
        bool* isOpaque = fOpaqueState.find(opaqueKey);
        if (!isOpaque) {
            isOpaque = fOpaqueState.set(opaqueKey, fRandom.nextBiasedBool(fOpaqueProbability));
        }

        return {paintID, binding, *isOpaque};
    }

private:
    const int fNumPipelines;
    const int fNumBindings;
    const float fOpaqueProbability;

    SkRandom fRandom;
    skia_private::THashMap<uint64_t, bool> fOpaqueState;
};

class SequentialPaintGenerator : public PaintGenerator {
public:
    SequentialPaintGenerator(int numPipelines, int numBindings, int repeatCount, bool evenIsOpaque, bool oddIsOpaque)
            : fNumPipelines(numPipelines)
            , fNumBindings(numBindings)
            , fRepeatCount(repeatCount)
            , fEvenBindingsAreOpaque(evenIsOpaque)
            , fOddBindingsAreOpaque(oddIsOpaque) {}

    std::string name() const override {
        return SkSL::String::printf("seq(%dx%d,%d,%d)",
                                    fNumPipelines, fNumBindings,
                                    fEvenBindingsAreOpaque, fOddBindingsAreOpaque);
    }

    std::tuple<int, int, bool> nextPaint() override {
        int paintIndex = fCurrentPaint / fRepeatCount;
        int paintID = (paintIndex / fNumBindings) % fNumPipelines;
        int binding = paintIndex % fNumBindings;
        bool isOpaque = (paintIndex & 0b1) ? fOddBindingsAreOpaque : fEvenBindingsAreOpaque;

        fCurrentPaint++;
        return {paintID, binding, isOpaque};
    }

private:
    const int fNumPipelines;
    const int fNumBindings;
    const int fRepeatCount;
    const bool fEvenBindingsAreOpaque;
    const bool fOddBindingsAreOpaque;

    int fCurrentPaint = 0;
};

class RandomGeomGenerator : public GeometryGenerator {
public:
    RandomGeomGenerator(SkISize minDrawSize, SkISize maxDrawSize, SkISize targetSize,
                        std::initializer_list<MockRenderType> types)
            : fMinSize(minDrawSize)
            , fMaxSize(maxDrawSize)
            , fTargetSize(targetSize)
            , fRenderTypes(types) {
        SkASSERT(minDrawSize.fWidth <= maxDrawSize.fWidth &&
                 minDrawSize.fHeight <= maxDrawSize.fHeight);
        SkASSERT(!types.empty());
    }

    std::string name() const override {
        return SkSL::String::printf("rand([%d-%d]x[%d-%d] in %dx%d)",
                                    fMinSize.fWidth, fMaxSize.fWidth,
                                    fMinSize.fHeight, fMaxSize.fHeight,
                                    fTargetSize.fWidth, fTargetSize.fHeight);
    }

    std::pair<MockRenderType, Rect> nextGeometry() override {
        float x = fRandom.nextRangeF(0.f, fTargetSize.fWidth);
        float y = fRandom.nextRangeF(0.f, fTargetSize.fHeight);
        float w = fRandom.nextRangeF(fMinSize.fWidth, fMaxSize.fWidth);
        float h = fRandom.nextRangeF(fMinSize.fHeight, fMaxSize.fHeight);

        MockRenderType type = fRenderTypes[fRandom.nextULessThan(fRenderTypes.size())];
        return {type, Rect::XYWH(x,y,w,h)};
    }

private:
    const SkISize fMinSize;
    const SkISize fMaxSize;

    const SkISize fTargetSize;

    const skia_private::TArray<MockRenderType> fRenderTypes;

    SkRandom fRandom;
};

class TilingGeomGenerator : public GeometryGenerator {
public:
    TilingGeomGenerator(SkISize drawSize, int tileShift, int inplaceRepeat, SkISize targetSize, MockRenderType type)
            : fDrawSize(drawSize)
            , fTargetSize(targetSize)
            , fTileShift(tileShift)
            , fInplaceRepeat(inplaceRepeat)
            , fRenderType(type) {}

    std::string name() const override {
        return SkSL::String::printf("tile(%dx%dx%d in %dx%d)",
                                    fDrawSize.fWidth, fDrawSize.fHeight, fTileShift,
                                    fTargetSize.fWidth, fTargetSize.fHeight);
    }

    std::pair<MockRenderType, Rect> nextGeometry() override {
        Rect bounds = Rect::XYWH(fNextX, fNextY, fDrawSize.fWidth, fDrawSize.fHeight);

        fCurrentRepeat++;
        if (fCurrentRepeat == fInplaceRepeat) {
            if (fNextX + fTileShift >= fTargetSize.fWidth) {
                fNextX = 0;
                if (fNextY + fTileShift >= fTargetSize.fHeight) {
                    fNextY = 0;
                } else {
                    fNextY += fTileShift;
                }
            } else {
                fNextX += fTileShift;
            }
            fCurrentRepeat = 0;
        }

        return {fRenderType, bounds};
    }

private:
    const SkISize fDrawSize;
    const SkISize fTargetSize;
    const int fTileShift;
    const int fInplaceRepeat;
    const MockRenderType fRenderType;

    int fCurrentRepeat = 0;
    int fNextX = 0;
    int fNextY = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BatchingBenchmark : public Benchmark {
public:
    BatchingBenchmark(std::unique_ptr<MockDevice> device, std::unique_ptr<DrawGenerator> generator)
            : fRenderer(std::make_unique<MockRenderer>())
            , fDrawGenerator(std::move(generator))
            , fDevice(std::move(device)) {
        fName = SkSL::String::printf("GraphiteBatching,%s,%s",
                                      fDevice->name().c_str(),
                                      fDrawGenerator->name().c_str());
    }

private:
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    const char* onGetName() final { return fName.c_str(); }

    void onDelayedSetup() final {
        const int numDraws = fDrawGenerator->numDraws();
        for (int i = 0; i < numDraws; ++i) {
            fDraws.push_back(fDrawGenerator->nextDraw());
        }
    }

    void onDraw(int loops, SkCanvas*) final {
        for (int i = 0; i < loops; ++i) {
            // First measure how long it takes to record all of the draws
            for (const MockDraw& d : fDraws) {
                fDevice->recordDraw(d);
            }
            // Second measure how long it takes to sort and prepare those draws for rendering
            fDevice->render(fRenderer.get());
            fRenderer->endRenderPass(fName.c_str());

            fDevice->reset();
        }
    }

    std::string fName;

    std::unique_ptr<MockRenderer> fRenderer;
    std::unique_ptr<DrawGenerator> fDrawGenerator;
    std::unique_ptr<MockDevice> fDevice;

    skia_private::TArray<MockDraw> fDraws;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BruteForceDevice final : public MockDevice {
public:

    std::string name() const override { return "brute"; }

    void recordDraw(const MockDraw& draw) override {
        MockDraw& recordedDraw = *fStorage.make<MockDraw>(draw);

        int compressedPaintersOrder = -1;
        for (int s = 0; s < draw.stepCount(); ++s) {
            int finalOrder = 0; // earliest layer for opaque draws
            if (draw.dependsOnDst(s)) {
                if (compressedPaintersOrder < 0) {
                    // Haven't calculated it yet, so determine backwards merge for this draw
                    compressedPaintersOrder = this->mostRecentDraw(draw.fBounds);
                }
                finalOrder = compressedPaintersOrder;
            } else if (draw.fRenderType == MockRenderType::kMultiStepWithStencil) {
                // Must make sure that the draw is disjoint from every other stencil draw in this
                // painters order (implicitly guaranteed when the paint depends on the dst).
                if (compressedPaintersOrder < 0) {
                    compressedPaintersOrder = this->disjointStencilOrder(draw.fBounds);
                }
                finalOrder = compressedPaintersOrder;
            }
            fSteps.push_back({&recordedDraw, finalOrder, s});

            if (fPrint) {
                SkDebugf("record draw %d %p (%dx%d) with %d + %d\n", finalOrder, &recordedDraw, (int)draw.fRenderType,
                        s, draw.fPaintID, draw.fDataBinding);
            }
        }

        // if (fPrint) {
        //     SkDebugf("Current draws:\n");
        //     for (const DrawStep& s : fSteps) {
        //         SkDebugf("  %p (%dx%d) with %d + %d (%d)\n",
        //             s.fDrawParams, (int) s.fDrawParams->fRenderType, s.fStep,
        //             s.fDrawParams->fPaintID, s.fDrawParams->fDataBinding, s.fDrawParams->pipelineIndex(s.fStep));
        //     }
        // }
    }

    void render(MockRenderer* renderer) override {

        if (fPrint) {
            SkDebugf("All draws before forward merging:\n");
            for (const DrawStep& s : fSteps) {
                SkDebugf("  %d %p (%dx%d) with %d + %d (%d)\n",
                    s.fCompressedPaintersOrder, s.fDrawParams, (int) s.fDrawParams->fRenderType, s.fStep,
                    s.fDrawParams->fPaintID, s.fDrawParams->fDataBinding, s.fDrawParams->pipelineIndex(s.fStep));
            }
        }

        // Forwards merge, increase compressed painters order if it is not occluded and there are
        // later draws with the same pipeline and data binding.
        for (int i = fSteps.size() - 1; i >= 0; --i) {
            DrawStep& currentStep = fSteps[i];
            if (!currentStep.fDrawParams->dependsOnDst(currentStep.fStep)) {
                // All draws that don't depend on the dst are already moved as early as possible,
                // so that already does a good job maximizing the batching potential.
                continue;
            }

            int currentPipeline = currentStep.fDrawParams->pipelineIndex(currentStep.fStep);
            int currentBinding = currentStep.fDrawParams->fDataBinding;

            Rect::ComplementRect drawBounds{currentStep.fDrawParams->fBounds};
            int earliestOccluder = std::numeric_limits<int>::max();
            bool occluderBindingMatch = true;

            for (int j = i + 1; j < fSteps.size(); ++j) {
                if (fSteps[j].fCompressedPaintersOrder > currentStep.fCompressedPaintersOrder) {
                    const bool occluded = fSteps[j].fDrawParams->fBounds.intersects(drawBounds);
                    const bool bindingMatch =
                            fSteps[j].fDrawParams->pipelineIndex(fSteps[j].fStep) == currentPipeline
                         && fSteps[j].fDrawParams->fDataBinding == currentBinding;

                    if (occluded) {
                        if (fSteps[j].fCompressedPaintersOrder < earliestOccluder) {
                            earliestOccluder = fSteps[j].fCompressedPaintersOrder;
                            occluderBindingMatch = bindingMatch;
                        } else if (fSteps[j].fCompressedPaintersOrder == earliestOccluder) {
                            occluderBindingMatch &= bindingMatch;
                        }
                    }
                }
            }

            SkASSERT(earliestOccluder >= currentStep.fCompressedPaintersOrder);
            if (occluderBindingMatch) {
                // There are other draws that this will match with by moving forwards so do it.
                // The Z sorting within a bindable batch will order the group correctly.
                currentStep.fCompressedPaintersOrder = earliestOccluder;
            } else {
                // Optimistically move the draw to the order immediately preceding the occluder.
                // If there are other earlier batched draws, they will also hopefully get carried
                // forwards to this same level in later iterations of the outer loop.
                currentStep.fCompressedPaintersOrder = earliestOccluder - 1;
            }
        }

        if (fPrint) {
            SkDebugf("All draws after forward merging:\n");
            for (const DrawStep& s : fSteps) {
                SkDebugf("  %d %p (%dx%d) with %d + %d (%d)\n",
                    s.fCompressedPaintersOrder, s.fDrawParams, (int) s.fDrawParams->fRenderType, s.fStep,
                    s.fDrawParams->fPaintID, s.fDrawParams->fDataBinding, s.fDrawParams->pipelineIndex(s.fStep));
            }
        }

        std::sort(fSteps.begin(), fSteps.end());

        if (fPrint) {
            SkDebugf("All draws after sorting:\n");
            for (const DrawStep& s : fSteps) {
                SkDebugf("  %d %p (%dx%d) with %d + %d (%d)\n",
                    s.fCompressedPaintersOrder, s.fDrawParams, (int) s.fDrawParams->fRenderType, s.fStep,
                    s.fDrawParams->fPaintID, s.fDrawParams->fDataBinding, s.fDrawParams->pipelineIndex(s.fStep));
            }
        }

        for (const DrawStep& s : fSteps) {
            renderer->draw(*s.fDrawParams, s.fStep);
        }
    }

    void reset() override {
        fSteps.clear();
        fStorage.reset();
        fPrint = false;
    }

private:
    struct DrawStep {
        const MockDraw* fDrawParams;
        int fCompressedPaintersOrder;
        int fStep;

        bool operator<(const DrawStep& d) const {
            if (fCompressedPaintersOrder == d.fCompressedPaintersOrder) {
                if (fStep == d.fStep) {
                    if (fDrawParams->fPaintID == d.fDrawParams->fPaintID) {
                        if (fDrawParams->fDataBinding == d.fDrawParams->fDataBinding) {
                            return fDrawParams->fZ < d.fDrawParams->fZ;
                        } else {
                            return fDrawParams->fDataBinding < d.fDrawParams->fDataBinding;
                        }
                    } else {
                        return fDrawParams->fPaintID < d.fDrawParams->fPaintID;
                    }
                } else {
                    return fStep < d.fStep;
                }
            } else {
                return fCompressedPaintersOrder < d.fCompressedPaintersOrder;
            }
        }
    };

    int mostRecentDraw(Rect bounds) {
        Rect::ComplementRect test{bounds};
        int compressedPaintersOrder = -1;
        for (const DrawStep& s : fSteps) {
            if (s.fCompressedPaintersOrder >= compressedPaintersOrder &&
                s.fDrawParams->fBounds.intersects(test)) {
                compressedPaintersOrder = s.fCompressedPaintersOrder;
            }
        }
        return compressedPaintersOrder + 1;
    }

    int disjointStencilOrder(Rect bounds) {
        Rect::ComplementRect test{bounds};

        int order = -1;
        bool noIntersection;
        do {
            order++;
            noIntersection = true;
            for (const DrawStep& s : fSteps) {
                if (s.fDrawParams->fRenderType != MockRenderType::kMultiStepWithStencil) {
                    continue; // Doesn't damage the stencil buffer
                }
                if (s.fCompressedPaintersOrder == order &&
                    s.fDrawParams->fBounds.intersects(test)) {
                    noIntersection = false;
                    break;
                }
            }
        } while(!noIntersection);

        return order;
    }

    skia_private::TArray<DrawStep> fSteps;
    SkArenaAllocWithReset fStorage{0};

    bool fPrint = false;
};

class GaneshDevice final : public MockDevice {
public:
    std::string name() const override { return "ganesh"; }

    void recordDraw(const MockDraw& draw) override {
        if (fOps.tail() != fOps.end()) {
            // Backwards merging
            Rect::ComplementRect drawTest{draw.fBounds};
            Op* current = &(*fOps.tail());
            int limit = 10; // GrOpsTask::kMaxOpMergeDistance
            while(current && limit > 0) {
                SkASSERT(current->fDraws.begin() != current->fDraws.end());
                MockRenderType type = current->fDraws.begin()->fRenderType;
                int paint = current->fDraws.begin()->fPaintID;
                int binding = current->fDraws.begin()->fDataBinding;

                if (draw.fRenderType == type &&
                    draw.fPaintID == paint &&
                    draw.fDataBinding == binding) {
                    // Combinable ops
                    current->fDraws.append(&fStorage, draw);
                    current->fBounds.join(draw.fBounds);
                    return;
                } else {
                    // See if the new draw would overlap any previous *op* bounds
                    if (current->fBounds.intersects(drawTest)) {
                        // Cannot continue searching so break out and add a new op
                        break;
                    }
                }
                current = current->fPrev;
                limit--;
            }
        }

        // Must add a new Op
        Op* tail = fOps.tail() != fOps.end() ? &(*fOps.tail()) : nullptr;
        Op& newOp = fOps.append(&fStorage, draw.fBounds, tail);
        newOp.fDraws.append(&fStorage, draw);
    }

    void render(MockRenderer* renderer) override {
        // Forward merge ops that match and aren't occluded
        for (auto current = fOps.begin(); current != fOps.end(); ++current) {
            if (current->fDraws.empty()) {
                continue; // Already merged
            }

            Rect::ComplementRect currentBounds{current->fBounds};

            MockRenderType currentType = current->fDraws.begin()->fRenderType;
            int currentPaint = current->fDraws.begin()->fPaintID;
            int currentBinding = current->fDraws.begin()->fDataBinding;

            int limit = 10; // GrOpsTask::kMaxOpMergeDistance
            auto next = current; ++next;
            while(next != fOps.end() && limit > 0) {
                MockRenderType nextType = next->fDraws.begin()->fRenderType;
                int nextPaint = next->fDraws.begin()->fPaintID;
                int nextBinding = next->fDraws.begin()->fDataBinding;
                const bool compat = nextType == currentType &&
                                    nextPaint == currentPaint &&
                                    nextBinding == currentBinding;

                if (compat) {
                    // Next op is compatible; use prepend() to combine draw lists while preserving
                    // painters order to automatically handle the case where next occludes current.
                    next->fDraws.prepend(std::move(current->fDraws));
                    break;
                } else if (next->fBounds.intersects(currentBounds)) {
                    // current op is occluded by next so no more forward merge candidates
                    break;
                }

                ++next;
                --limit;
            }
        }

        for (const Op& op : fOps) {
            SkASSERT(op.fDraws.begin() != op.fDraws.end());
            [[maybe_unused]] MockRenderType type = op.fDraws.begin()->fRenderType;
            [[maybe_unused]] int paint = op.fDraws.begin()->fPaintID;
            [[maybe_unused]] int binding = op.fDraws.begin()->fDataBinding;

            for (const MockDraw& draw : op.fDraws) {
                SkASSERT(draw.fRenderType == type &&
                         draw.fPaintID == paint &&
                         draw.fDataBinding == binding);

                // Ganesh doesn't handle the inner fill as a separate draw, so don't count the
                // pipeline change that that would trigger.
                int stepCount = draw.fRenderType == MockRenderType::kInnerFillRect ? 1 : draw.stepCount();
                for (int s = 0; s < stepCount; ++s) {
                    renderer->draw(draw, s);
                }
            }
        }
    }

    void reset() override {
        fOps.reset();
        fStorage.reset();
    }

private:
    struct Op {
        Op(Rect bounds, Op* prev) : fBounds(bounds), fPrev(prev) {}

        Rect fBounds;
        SkArenaAllocList<MockDraw> fDraws;

        Op* fPrev; // add reverse iteration ot arena-alloc-list
    };

    SkArenaAllocWithReset fStorage{0};

    SkArenaAllocList<Op> fOps;
};

class GraphiteDevice final : public MockDevice {
public:
    GraphiteDevice(SkISize size)
            : fBoundsManager(std::make_unique<HybridBoundsManager>(size, 16, 64, 32)) // from Device
            // : fBoundsManager(std::make_unique<BruteForceBoundsManager>())
            , fIntersectionSet(std::make_unique<IntersectionTreeSet>()) {}

    std::string name() const override { return "graphite"; }

    void recordDraw(const MockDraw& draw) override {
        CompressedPaintersOrder order = DrawOrder::kNoIntersection.next();
        DisjointStencilIndex stencilIndex = DrawOrder::kUnassigned;

        if (draw.dependsOnDst(draw.stepCount() - 1)) {
            order = fBoundsManager->getMostRecentDraw(draw.fBounds).next();
        }
        if (draw.fRenderType == MockRenderType::kMultiStepWithStencil) {
            stencilIndex = fIntersectionSet->add(order, draw.fBounds);
        }

        fDraws.push_back(draw);
        int drawIndex = fDraws.count() - 1;
        for (int s = 0 ; s < draw.stepCount(); ++s) {
            if (draw.fRenderType == MockRenderType::kInnerFillRect &&
                s == 0 && draw.fIsPaintOpaque) {
                // Inner fill draw uses the opaque draw order and the transparent second step
                // must come after
                this->record(drawIndex, DrawOrder::kNoIntersection, DrawOrder::kUnassigned, s,
                             draw.fPaintID, draw.fDataBinding);
                // order = max(kUnassigned.next(), order) except that we are at 0 for inner fill and
                // 'order' is already at least 1.
            } else {
                this->record(drawIndex, order, stencilIndex, s, draw.fPaintID, draw.fDataBinding);
            }
        }

        fBoundsManager->recordDraw(draw.fBounds, order);
    }

    void render(MockRenderer* renderer) override {
        std::sort(fSteps.begin(), fSteps.end());

        for (uint64_t step : fSteps) {
            auto [d, s] = this->unpack(step);
            renderer->draw(d, s);
        }
    }

    void reset() override {
        fDraws.reset();
        fBoundsManager->reset();
        fIntersectionSet->reset();
        fSteps.clear();
    }

private:
    class IntersectionTreeSet {
    public:
        IntersectionTreeSet() = default;

        DisjointStencilIndex add(CompressedPaintersOrder drawOrder, Rect rect) {
            auto& trees = fTrees[drawOrder];
            DisjointStencilIndex stencil = DrawOrder::kUnassigned;
            for (auto&& tree : trees) {
                if (tree->add(rect)) {
                    return stencil;
                }
                stencil = stencil.next(); // advance to the next tree's index
            }

            // If here, no existing intersection tree can hold the rect so add a new one
            IntersectionTree* newTree = this->makeTree();
            SkAssertResult(newTree->add(rect));
            trees.push_back(newTree);
            return stencil;
        }

        void reset() {
            fTrees.clear();
            fTreeStore.reset();
        }

    private:
        struct Hash {
            size_t operator()(const CompressedPaintersOrder& o) const noexcept { return o.bits(); }
        };

        IntersectionTree* makeTree() {
            return fTreeStore.make<IntersectionTree>();
        }

        // Each compressed painters order defines a barrier around draws so each order's set of draws
        // are independent, even if they may intersect. Within each order, the list of trees holds the
        // IntersectionTrees representing each disjoint set.
        // TODO: This organization of trees is logically convenient but may need to be optimized based
        // on real world data (e.g. how sparse is the map, how long is each vector of trees,...)
        std::unordered_map<CompressedPaintersOrder, std::vector<IntersectionTree*>, Hash> fTrees;
        SkSTArenaAllocWithReset<4 * sizeof(IntersectionTree)> fTreeStore;
    };


    // Helper to manage packed fields within a uint64_t
    template <uint64_t Bits, uint64_t Offset>
    struct Bitfield {
        static constexpr uint64_t kMask = ((uint64_t) 1 << Bits) - 1;
        static constexpr uint64_t kOffset = Offset;
        static constexpr uint64_t kBits = Bits;

        static uint32_t get(uint64_t v) { return static_cast<uint32_t>((v >> kOffset) & kMask); }
        static uint64_t set(uint32_t v) { return (v & kMask) << kOffset; }
    };

    // For convenience we are going to assume that there won't be more than 1024 unique pipelines
    // or bindings, and max 16k draws.
    using PaintOrder = Bitfield<14, 50>;
    using StencilIndex = Bitfield<14, 36>;
    using Step = Bitfield<2, 34>;
    using Pipeline = Bitfield<10, 24>;
    using Binding = Bitfield<10, 14>;
    using Draw = Bitfield<14, 0>;

    void record(int drawIndex,
                CompressedPaintersOrder paintOrder,
                DisjointStencilIndex stencilIndex,
                int step,
                int paintID,
                int dataBinding) {
        uint64_t key = PaintOrder::set(paintOrder.bits()) |
                       StencilIndex::set(stencilIndex.bits()) |
                       Step::set(step) |
                       Pipeline::set(paintID) |
                       Binding::set(dataBinding) |
                       Draw::set(drawIndex);
        fSteps.push_back(key);
    }

    std::pair<const MockDraw&, int /*step*/> unpack(uint64_t key) const {
        int drawIndex = Draw::get(key);
        int step = Step::get(key);
        return {fDraws.item(drawIndex), step};
    }

    SkTBlockList<MockDraw> fDraws{16, SkBlockAllocator::GrowthPolicy::kFibonacci};
    skia_private::TArray<uint64_t> fSteps; // use primitive type for BlockQuickSort impl.

    std::unique_ptr<BoundsManager> fBoundsManager;
    std::unique_ptr<IntersectionTreeSet> fIntersectionSet;
};

class BoundsTree {
public:
    // enum class SplitType : bool {
    //     kX,
    //     kY
    // };

    BoundsTree(SkArenaAlloc* arena);

    void add(SkArenaAlloc* arena, Rect rect, int64_t matchData) {
        fRoot = fRoot->add(rect, matchData, arena);
    }

    std::pair<bool, bool> test(Rect rect, int64_t matchData) {
        // SkDebugf("root tree test: %lld\n", matchData);
        return fRoot->intersects(rect, matchData);
    }

    std::tuple<int, int, int> dump() const {
        return fRoot->dump(0);
    }

private:
    class Node {
    public:
        virtual ~Node() = default;

        // <intersects, bindingMatch>
        virtual std::pair<bool, bool> intersects(Rect, int64_t) = 0;
        virtual Node* add(Rect, int64_t, SkArenaAlloc*) = 0;

        // <ct, depth, nodeCt>
        virtual std::tuple<int, int, int> dump(int indent) const = 0;

        int64_t fDataMatch = -1;
        Rect fBounds = Rect::InfiniteInverted();
    };

    class TreeNode;
    class LeafNode;

    // The TreeNode size is made of a vtable (i.e. sizeof(void*)), float, and two Node* pointers.
    // We also align between the Node* and the float which may add some extra padding.
    // constexpr static int kTreeNodeSize = SkAlignTo(sizeof(void*) + sizeof(float), alignof(void*)) +
    //                                      2 * sizeof(Node*);
    // constexpr static int kLeafNodeSize = 16 + (2 + 64) * sizeof(Rect);
    // constexpr static int kPadSize = 256;  // For footers and alignment.
    // SkArenaAlloc fArena{kLeafNodeSize + kTreeNodeSize + kPadSize*2};
    Node* fRoot;
};


// BSP node. Space is partitioned by an either vertical or horizontal line. Note that if a rect
// straddles the partition line, it will need to go on both sides of the tree.
// template<BoundsTree::SplitType kSplitType>
class BoundsTree::TreeNode final : public Node {
public:
    TreeNode(Node* lo, Node* hi)
            : fLo(lo)
            , fHi(hi) {
        fDataMatch = lo->fDataMatch == hi->fDataMatch ? lo->fDataMatch : -1;
        fBounds = lo->fBounds.makeJoin(hi->fBounds);
        // SkDebugf("creating tree node\n");
    }

    std::pair<bool, bool> intersects(Rect rect, int64_t matchData) override {
          if (matchData >= 0 && matchData == fDataMatch) {
            // Overlaps are okay, so accept it as-is since everything in this node matches.
            return {false, true};
        }

        if (!fBounds.intersects(rect)) {
            // Here we are disjoint, and have an inconsistent data match. However, we could have
            // a binding match in a leaf node still that's worth checking for. But if the outer
            // layer logic just checks for a binding list, then this isn't worth it to do here.
            return {false, false};
        }

        // FIXME: How can we signal that a data match modifies the stencil buffer but is otherwise
        // not dependent on color, so we could ignore data mismatches on rects that didn't use
        // the stencil buffer.
        // SkDebugf("tree test: %lld vs %lld\n", fDataMatch, matchData);
        auto [loIntersect, loMatch] = fLo->intersects(rect, matchData);
        // SkDebugf("lo result %d %d\n", loIntersect, loMatch);

        auto [hiIntersect, hiMatch] = fHi->intersects(rect, matchData);
        // SkDebugf("hi result %d %d\n", hiIntersect, hiMatch);

        // Data match should be true if there is a matched drawing and there isn't an intersection
        // with a mismatched drawing.
        bool hasMatch = matchData >= 0 && (loMatch || hiMatch);
        bool matchBlocked = (loIntersect && !loMatch) || (hiIntersect && !hiMatch);

        // SkDebugf("tree node result %d %d\n", loIntersect || hiIntersect, hasMatch && !matchBlocked);
        return {loIntersect || hiIntersect, hasMatch && !matchBlocked};
    }

    Node* add(Rect rect, int64_t matchData, SkArenaAlloc* arena) override {
        fBounds.join(rect);
        if (matchData != fDataMatch) {
            fDataMatch = -1;
        }

        if (fLo->fBounds.makeJoin(rect).area() < fHi->fBounds.makeJoin(rect).area()) {
            fLo = fLo->add(rect, matchData, arena);
        } else {
            fHi = fHi->add(rect, matchData, arena);
        }
        return this;
    }

    std::tuple<int, int, int> dump(int indent) const override {
        SkDebugf("%*cnode depth %d - %f %f %f %f, data %lld", 2*indent, ' ',
            indent, fBounds.left(), fBounds.top(), fBounds.right(), fBounds.bot(),
            fDataMatch);
        SkDebugf("%*clo\n", 2*indent, ' ');
        auto [lowCt, loMaxDepth, loNodes] = fLo->dump(indent + 1);
        SkDebugf("%*chi\n", 2*indent, ' ');
        auto [hiCt, hiMaxDepth, hiNodes] = fHi->dump(indent + 1);

        return {lowCt + hiCt, std::max(loMaxDepth, hiMaxDepth) + 1, loNodes + hiNodes + 1};
    }

private:
    // FIXME generalize this to N nodes.
    Node* fLo;
    Node* fHi;
};

// Leaf node. Rects are kept in a simple list and intersection testing is performed by brute force.
class BoundsTree::LeafNode final : public Node {
public:
    // Max number of rects to store in this node before splitting. With SSE/NEON optimizations, ~64
    // brute force rect comparisons seems to be the optimal number.
    constexpr static int kMaxRectsInList = 64;

    LeafNode() {
        this->popAll();
        // Initialize our arrays with maximally negative rects. These have the advantage of always
        // failing intersection tests, thus allowing us to test for intersection beyond fNumRects
        // without failing.
        constexpr static float infinity = std::numeric_limits<float>::infinity();
        std::fill_n(fLefts, kMaxRectsInList, infinity);
        std::fill_n(fTops, kMaxRectsInList, infinity);
        std::fill_n(fNegRights, kMaxRectsInList, infinity);
        std::fill_n(fNegBots, kMaxRectsInList, infinity);
    }

    void popAll() {
        fNumRects = 0;
        fBounds = Rect::InfiniteInverted();
        fDataMatch = -1;
        // Leave the rect arrays untouched. Since we know they are either already valid in the tree,
        // or else maximally negative, this allows the future list to check for intersection beyond
        // fNumRects without failing.
    }

    std::tuple<int, int, int> dump(int indent) const override {
        SkDebugf("%*cleaf depth %d, # %d, %f %f %f %f, data %lld\n",
            2*indent, ' ', indent, fNumRects, fBounds.left(), fBounds.top(), fBounds.right(), fBounds.bot(), fDataMatch);
        return {fNumRects, 1, 1};
    }

    std::pair<bool,bool> intersects(Rect rect, int64_t matchData) override {
        // Test for intersection in sets of 4. Since all the data in our rect arrays is either
        // maximally negative, or valid from somewhere else in the tree, we can test beyond
        // fNumRects without failing.
        static_assert(kMaxRectsInList % 4 == 0);
        SkASSERT(fNumRects <= kMaxRectsInList);

        if (matchData >= 0 && matchData == fDataMatch) {
            // Disregard intersections for a node with entirely homogenous data matches
            return {false, true};
        }

        if (!fBounds.intersects(rect)) {
            return {false, false};
        }

        bool hasMatch = false;

        bool intersects = false;
        bool intersectionMismatch = false;

        auto comp = Rect::ComplementRect(rect).fVals;
        auto match = skvx::long4(matchData);
        // SkDebugf("leaf check(%d) %lld vs %lld\n", fNumRects, fDataMatch, matchData);

        for (int i = 0; i < fNumRects; i += 4) {
            auto m = skvx::long4::Load(fDataMatches + i);
            auto matchMask = skvx::cast<int32_t>((match < 0) | (m != match)); // true if care about intersection

            bool iterMatch = all(m == match);
            hasMatch |= iterMatch;

            auto l = skvx::float4::Load(fLefts + i);
            auto t = skvx::float4::Load(fTops + i);
            auto nr = skvx::float4::Load(fNegRights + i);
            auto nb = skvx::float4::Load(fNegBots + i);

            if (any((l < comp[0]) &
                    (t < comp[1]) &
                    (nr < comp[2]) &
                    (nb < comp[3]))) {
                intersects = true;
                intersectionMismatch |= any(matchMask);
                // SkDebugf("leaf test %d intersects, mismatch = %d\n", i, any(matchMask));
                // if (any((l < comp[0]) & (t < comp[1]) & (nr < comp[2]) & (nb < comp[3]) & matchMask)) {
                //     intersects = true;
                //     intersectionMismatch |= !iterMatch;
                //     SkDebugf("i %d, %d, %d\n", i, hasMatch, intersectionMismatch);
                //     // FIXME not quite right since this is aggregating x4
                // } else {
                //     SkDebugf("intersection that is hidden by match mask\n");
                // }

            }
        }

        // We have a match if `hasMatch` is true and there are no intersections or there are no
        // mismatches that do intersect.
        // SkDebugf("leaf result = %d match = %d\n", intersects, !intersectionMismatch && hasMatch && matchData >= 0);
        return {intersects, !intersectionMismatch && hasMatch && matchData >= 0};
    }

    Node* add(Rect rect, int64_t matchData, SkArenaAlloc* arena) override {
        // SkDebugf("adding to leaf: %d\n", fNumRects);
        if (fNumRects == kMaxRectsInList) {
            // The new rect doesn't fit. Split our rect list first and then add.
            // SkDebugf("going to call split: %p %p\n", this, arena);
            Node* split = this->split(arena);
            // SkDebugf("split result = %p\n", split);
            if (!split) {
                // SkDebugf("node failure...\n");
                return nullptr;
            } else {
                return split->add(rect, matchData, arena);
            }
            // return this->split(arena)->addNonIntersecting(rect, matchData, arena);
        }
        this->appendToList(rect, matchData);
        return this;
    }

private:
    void appendToList(Rect rect, int64_t matchData) {
        SkASSERT(fNumRects < kMaxRectsInList);
        int i = fNumRects++;
        // [maxLeft, maxTop, -minRight, -minBot]
        fLefts[i] = rect.vals()[0];
        fTops[i] = rect.vals()[1];
        fNegRights[i] = rect.vals()[2];
        fNegBots[i] = rect.vals()[3];
        fDataMatches[i] = matchData;

        if (i == 0) {
            fDataMatch = matchData; // first entry
        } else if (matchData != fDataMatch) {
            fDataMatch = -1; // heterogenous
        }

        fBounds.join(rect);
    }

    Rect loadRect(int i) const {
        return Rect::FromVals({fLefts[i], fTops[i], fNegRights[i], fNegBots[i]});
    }

    // Splits this node with a new LeafNode, then returns a TreeNode that reuses our "this" pointer
    // along with the new node.
    BoundsTree::Node* split(SkArenaAlloc* arena) {
        // This should only get called when our list is full.
        SkASSERT(fNumRects == kMaxRectsInList);
        // SkDebugf("split called\n");

        // Split "this", leaving all rects below "splitCoord" in this, and placing all rects above
        // splitCoord in "hiNode". There may be some reduncancy between lists, but we made sure to
        // select a split that would leave both lists strictly smaller than the original.
        LeafNode* hiNode = arena->make<LeafNode>();
        if (!hiNode) {
            // SkDebugf("hi node failed\n");
            return nullptr;
        }
        int numCombinedRects = fNumRects;
        this->popAll();


        for (int i = 0; i < numCombinedRects; ++i) {
            int64_t data = fDataMatches[i];
            Rect rect = this->loadRect(i);

            // FIXME is there a better way to split these up? Maximize disjoint ness of the two nodes
            // or collapse children that are contained within each other?
            if (i & 0b1) { // FIXME bad split value here, but must repro logic error first
                this->appendToList(rect, data);
            } else {
                hiNode->appendToList(rect, data);
            }
        }

        // SkDebugf("lo rects = %d, hi rects = %d\n", fNumRects, hiNode->fNumRects);

        SkASSERT(0 < fNumRects && fNumRects < numCombinedRects);
        SkASSERT(0 < hiNode->fNumRects && hiNode->fNumRects < numCombinedRects);

        Node* r = (Node*)arena->make<TreeNode>(this, hiNode);
        // SkDebugf("split tree node: %p, hi: %p, lo: %p\n", r, hiNode, this);
        return r;
    }

    int fNumRects;
    // skvx::float4 fSplittableBounds;  // [maxLeft, maxTop, -minRight, -minBot]
    // skvx::float4 fRectValsSum;  // [sum(left), sum(top), -sum(right), -sum(bot)]
    alignas(Rect) float fLefts[kMaxRectsInList];
    alignas(Rect) float fTops[kMaxRectsInList];
    alignas(Rect) float fNegRights[kMaxRectsInList];
    alignas(Rect) float fNegBots[kMaxRectsInList];
    int64_t fDataMatches[kMaxRectsInList];
    static_assert((kMaxRectsInList * sizeof(float)) % sizeof(Rect) == 0);
};

BoundsTree::BoundsTree(SkArenaAlloc* arena): fRoot(arena->make<LeafNode>()) {
        // static_assert(kTreeNodeSize == sizeof(TreeNode<SplitType::kX>));
        // static_assert(kTreeNodeSize == sizeof(TreeNode<SplitType::kY>));
        // static_assert(kLeafNodeSize == sizeof(LeafNode));
    }

class ExperimentDevice : public MockDevice {
public:

    std::string name() const override { return "experiment"; }


    void recordDraw(const MockDraw& draw) override {
        if (draw.fRenderType == MockRenderType::kInnerFillRect && draw.stepCount() > 1) {
            // Must record inner fill and coverage draws separately
            this->recordStep(draw, 0);
            this->recordStep(draw, 1);
        } else {
            // Record in aggregate and allow the fact that layers are disjoint to handle iterating
            // across the steps with minimal pipeline changes.
            this->recordStep(draw, -1);
        }
    }

    void render(MockRenderer* renderer) override {
        int totalCount = 0;
        int depthCount = 0;
        int nodeCount = 0;
        int layerCount = 0;
        for (const Layer* currentLayer : fLayers) {
            if (fPrint) {
                // SkDebugf("Layer:\n");
                // for (int i = 0; i < currentLayer->fSlotsFilled; ++i) {
                //     Rect r = currentLayer->fSlots[i].fBounds;
                //     SkDebugf("  slot %d: %f %f %f %f with %ld\n", i, r.left(), r.top(), r.right(), r.bot(), currentLayer->fSlots[i].fBinding);
                // }

                // auto [ct, depth, nodes] = currentLayer->fTree.dump();
                // SkDebugf("tree summary: # %d in %d levels (%d nodes)\n", ct, depth, nodes);
                // totalCount += ct;
                // depthCount += depth;
                // nodeCount += nodes;
                totalCount += currentLayer->fAddedCount;
                depthCount += currentLayer->fAddedCount;
                layerCount++;
            }

            for (const auto* currentBinding : currentLayer->fBindings) {
                if (fPrint) {
                    // SkDebugf("  binding %lld\n", currentBinding->fBinding);
                }

                const DrawStep* firstStep = currentBinding->fDraws.head();
                if (firstStep) {
                    int stepCount = firstStep->fStep >= 0 ? 1 : firstStep->fDraw->stepCount();
                    for (int s = 0; s < stepCount; ++s) {
                        for (const DrawStep* currentStep : currentBinding->fDraws) {
                            int step = std::max(currentStep->fStep, s);
                            if (fPrint) {
                                // Rect r = currentStep->fDraw->fBounds;
                                // int64_t binding = this->getBinding(*currentStep->fDraw, currentStep->fStep);
                                // SkDebugf("    draw %d|%d with %f %f %f %f and dst %d / binding %lld\n",
                                //         currentStep->fDraw->fZ, step,
                                //         r.left(), r.top(), r.right(), r.bot(),
                                //         currentStep->fDraw->dependsOnDst(step), binding);
                            }
                            renderer->draw(*currentStep->fDraw, step);
                        }
                    }
                }
            }
        }

        if (fPrint) {
            SkDebugf("Total layers %d, total count %d, avg depth %d, total nodes %d\n", layerCount, totalCount, depthCount / layerCount, nodeCount);
        }
    }

    void reset() override {
        fLayers.reset();
        fStorage.reset();
        fPrint = false;
    }

private:
    struct DrawStep {
        DrawStep(const MockDraw* draw, int step) : fDraw(draw), fStep(step) {}

        const MockDraw* fDraw;
        int fStep; // < 0 means do all the steps in sequence


    private:
        // Managed by BindingList
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(DrawStep);
    };

    struct BindingList {
        BindingList(int64_t binding) : fBinding(binding) {}

        // All draw steps will have the same render type, step, paint ID, and data binding if they
        // are in the same linked list.
        SkTInternalLList<DrawStep> fDraws;

        int64_t fBinding;

    private:
        // Managed by Layer
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingList);
    };

    struct Bounds {
        Bounds() : fBounds(Rect::InfiniteInverted()), fBinding(-1) {}

        Rect fBounds;
        int64_t fBinding; // or -1 if there are mixed bindings held in a bounds union
    };

    enum BoundsTest {
        kDisjoint,
        kCompatibleOverlap,
        kIncompatibleOverlap
    };

    struct Layer1 {
        std::array<Bounds, 64> fSlots; // FIXME: how many?
        int fSlotsFilled = 0;

        int fAddedCount = 0;

        int fInsertIndex = 0; // remember best insertion index during test()
        bool fBindingMatch = 0; // remember if we saw a matching binding during test()

        SkTInternalLList<BindingList> fBindings;

        // Managed by Device
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer1);

        Layer1(SkArenaAlloc*) { }

        BoundsTest test(const MockDraw& draw, int64_t binding) {
            bool intersectsWithBindingMatch = false;

            fInsertIndex = fSlotsFilled;
            fBindingMatch = false;
            float minArea = std::numeric_limits<float>::max();

            Rect::ComplementRect testBounds{draw.fBounds};
            for (int i = 0; i < fSlotsFilled; ++i) {
                if (fSlots[i].fBinding == binding) {
                    fBindingMatch = true;
                }

                // SkDebugf("testing slot %d, %f %f %f %f intersects? %d match %d\n",
                //         i, fSlots[i].fBounds.left(), fSlots[i].fBounds.top(), fSlots[i].fBounds.right(), fSlots[i].fBounds.bot(),
                //         fSlots[i].fBounds.intersects(testBounds), fSlots[i].fBinding == binding);

                if (fSlots[i].fBounds.intersects(testBounds)) {
                    if (fSlots[i].fBinding != binding) {
                        // The draw overlaps a non-compatible draw so it shouldn't belong to this
                        // layer at all.
                        return BoundsTest::kIncompatibleOverlap;
                    } else {
                        // The draw overlaps a compatible draw, so it could be merged into this
                        // layer but it cannot be added to any prior layers.
                        intersectsWithBindingMatch = true;
                        // SkDebugf("Found intersection with binding match\n");
                    }
                }

                if (fSlotsFilled == (int) fSlots.size()) {
                    // Keep track of the best bounds to join with
                    float mergedArea = fSlots[i].fBounds.makeJoin(draw.fBounds).area();
                    if (mergedArea < minArea) {
                        minArea = mergedArea;
                        fInsertIndex = i;
                    }
                }
            }
            // SkDebugf("test insert index = %d, min area = %f, match = %d\n", fInsertIndex, minArea, fBindingMatch);
            auto test = intersectsWithBindingMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint;
            if (draw.fRenderType == MockRenderType::kMultiStepWithStencil && test == BoundsTest::kCompatibleOverlap) {
                test = BoundsTest::kIncompatibleOverlap;
            }
            return test;
        }

        void trackOpaqueDraw(Rect drawBounds, int64_t binding) {
            fInsertIndex = fSlotsFilled;
            fBindingMatch = false;

            if (fSlotsFilled == (int) fSlots.size()) {
                Rect::ComplementRect testBounds{drawBounds};
                float minArea = std::numeric_limits<float>::max();
                for (int i = 0; i < fSlotsFilled; ++i) {
                    // SkDebugf("testing slot %d, %f %f %f %f intersects? %d match %d\n",
                    //             i, fSlots[i].fBounds.left(), fSlots[i].fBounds.top(), fSlots[i].fBounds.right(), fSlots[i].fBounds.bot(),
                    //             fSlots[i].fBounds.intersects(testBounds), fSlots[i].fBinding == binding);
                    if (fSlotsFilled == (int) fSlots.size()) {
                        // Keep track of the best bounds to join with
                        float mergedArea = fSlots[i].fBounds.makeJoin(drawBounds).area();
                        if (mergedArea < minArea) {
                            minArea = mergedArea;
                            fInsertIndex = i;
                        }
                    }
                }
            }
        }

        void add(SkArenaAlloc* alloc, int64_t binding, const MockDraw& draw, int step) {
            fAddedCount++;

            SkASSERT(fInsertIndex >= 0 && fInsertIndex < (int) fSlots.size());
            // Record the bounds in the approximate layer bounds
            if (fSlots[fInsertIndex].fBounds.isEmptyNegativeOrNaN()) {
                // SkDebugf("adding draw to slot %d\n", fInsertIndex);
                fSlots[fInsertIndex].fBounds = draw.fBounds;
                fSlots[fInsertIndex].fBinding = binding;
                SkASSERT_RELEASE(fSlotsFilled < (int) fSlots.size());
                fSlotsFilled++;
            } else {
                SkASSERT_RELEASE(fSlotsFilled == (int) fSlots.size());
                fSlots[fInsertIndex].fBounds.join(draw.fBounds);
                if (fSlots[fInsertIndex].fBinding != binding) {
                    fSlots[fInsertIndex].fBinding = -1;
                }
                // SkDebugf("adding draw to slot %d, %ld\n", fInsertIndex, fSlots[fInsertIndex].fBinding);
            }

            // Create a DrawStep to store
            const MockDraw* recordedDraw = alloc->make<MockDraw>(draw);
            DrawStep* drawStep = alloc->make<DrawStep>(recordedDraw, step);

            // Look for a compatible binding list to append to
            for (BindingList* list : fBindings) {
                if (list->fBinding == binding) {
                    // Found it, so we can include the draw in this list. If the draw is opaque it
                    // should be in the front of the list to maximize early z rejection. Otherwise
                    // it should go to the tail.
                    bool dependsOnDst = step < 0 ? draw.dependsOnDst(draw.stepCount() - 1)
                                                 : draw.dependsOnDst(step);

                    if (dependsOnDst) {
                        list->fDraws.addToTail(drawStep);
                    } else {
                        list->fDraws.addToHead(drawStep);
                    }
                    return;
                } // else keep looking
            }

            // If we get here, we don't have a binding list with this draw's binding.
            BindingList* newList = alloc->make<BindingList>(binding);
            newList->fDraws.addToHead(drawStep);
            fBindings.addToTail(newList);
        }
    };

    struct Layer2 {
        BoundsTree fTree;

        int fInsertIndex = 0; // remember best insertion index during test()
        bool fBindingMatch = 0; // remember if we saw a matching binding during test()

        SkTInternalLList<BindingList> fBindings;

        // Managed by Device
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer2);

        Layer2(SkArenaAlloc* alloc) : fTree(alloc) {}

        BoundsTest test(const MockDraw& draw, int64_t binding) {
            if (draw.fRenderType == MockRenderType::kMultiStepWithStencil) {
                binding = -1;
            }
            // SkDebugf("testing %lld, has more layers = %d\n", binding, SkToBool(fPrev));
            auto [intersects, match] = fTree.test(draw.fBounds, binding);
            // SkDebugf("test result = %d %d\n", intersects, match);
            fBindingMatch = match;
            if (intersects) {
                return match ? BoundsTest::kCompatibleOverlap : BoundsTest::kIncompatibleOverlap;
            } else {
                return BoundsTest::kDisjoint;
            }
        }

        void trackOpaqueDraw(Rect drawBounds, int64_t binding) {}

        void add(SkArenaAlloc* alloc, int64_t binding, const MockDraw& draw, int step) {
            // SkDebugf("add %lld\n", binding);
            fTree.add(alloc, draw.fBounds, binding);

            // Create a DrawStep to store
            const MockDraw* recordedDraw = alloc->make<MockDraw>(draw);
            DrawStep* drawStep = alloc->make<DrawStep>(recordedDraw, step);

            // Look for a compatible binding list to append to
            for (BindingList* list : fBindings) {
                if (list->fBinding == binding) {
                    // Found it, so we can include the draw in this list. If the draw is opaque it
                    // should be in the front of the list to maximize early z rejection. Otherwise
                    // it should go to the tail.
                    bool dependsOnDst = step < 0 ? draw.dependsOnDst(draw.stepCount() - 1)
                                                 : draw.dependsOnDst(step);

                    if (dependsOnDst) {
                        list->fDraws.addToTail(drawStep);
                    } else {
                        list->fDraws.addToHead(drawStep);
                    }
                    return;
                } // else keep looking
            }

            // If we get here, we don't have a binding list with this draw's binding.
            BindingList* newList = alloc->make<BindingList>(binding);
            newList->fDraws.addToHead(drawStep);
            fBindings.addToTail(newList);
        }
    };

    struct Bounds2 {
        Bounds2(const Rect& bounds) : fBounds(bounds), fBinding(nullptr) {}

        Rect fBounds;
        BindingList* fBinding;
        // int64_t fBinding; // or -1 if there are mixed bindings held in a bounds union

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Bounds2);
    };

    struct Layer3 {
        // std::array<Bounds, 64> fSlots; // FIXME: how many?
        // int fSlotsFilled = 0;
        SkTInternalLList<Bounds2> fBounds;

        int fAddedCount = 0;

        // int fInsertIndex = 0; // remember best insertion index during test()
        // bool fBindingMatch = 0; // remember if we saw a matching binding during test()
        BindingList* fBindingMatch = nullptr;
        Bounds2* fMerge = nullptr;

        SkTInternalLList<BindingList> fBindings;

        // Managed by Device
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer3);

        Layer3(SkArenaAlloc*) { }

        BoundsTest test(const MockDraw& draw, int64_t binding) {
            if (draw.fRenderType != MockRenderType::kMultiStepWithStencil) {
                // We can disregard overlaps if there's no stencil overlap and a trivial binding
                // match. If there are multiple bindings then we need to test for overlaps.
                if (fBindings.head() == fBindings.tail() && fBindings.head()->fBinding == binding) {
                    fBindingMatch = fBindings.head();
                    return BoundsTest::kCompatibleOverlap;
                }
            }

            bool intersectsWithBindingMatch = false;

            fBindingMatch = nullptr;
            fMerge = nullptr;
            float minArea = std::numeric_limits<float>::max();
            int count = 0;

            Rect::ComplementRect testBounds{draw.fBounds};

            for (Bounds2* b : fBounds) {
                int64_t data = b->fBinding->fBinding;
                if (data == binding) {
                    fBindingMatch = b->fBinding;

                    count++;
                    // This join cost doubles the paths bounds checking logic time, even if we don't
                    // use the result to change the node count...
                    // float mergedArea = b->fBounds.makeJoin(draw.fBounds).area();
                    // if (mergedArea < minArea) {
                    //     minArea = mergedArea;
                    //     fMerge = b;
                    // }
                }

                if (b->fBounds.intersects(testBounds)) {
                    if (data != binding) {
                        return BoundsTest::kIncompatibleOverlap;
                    } else {
                        intersectsWithBindingMatch = true;
                    }
                }
            }

            // At this point we've checked every bounds, so if we have a binding match (regardless
            // of intersection or not), it will be stored for add(). If add() sees nullptr there,
            // it knows to add a new entry to BindingList.
            auto test = intersectsWithBindingMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint;
            if (draw.fRenderType == MockRenderType::kMultiStepWithStencil && test == BoundsTest::kCompatibleOverlap) {
                test = BoundsTest::kIncompatibleOverlap;
                fMerge = nullptr;
            }
            if (fMerge && minArea > 1.05f * fMerge->fBounds.area()) {
                fMerge = nullptr;
            }
                            // fMerge = nullptr;

            return test;
        }

        void trackOpaqueDraw(Rect drawBounds, int64_t binding) {
            fBindingMatch = nullptr;
            for (BindingList* b : fBindings) {
                if (b->fBinding == binding) {
                    fBindingMatch = b;
                    break;
                }
            }
        }

        void add(SkArenaAlloc* alloc, int64_t binding, const MockDraw& draw, int step) {
            fAddedCount++;

            // Create a DrawStep to store
            const MockDraw* recordedDraw = alloc->make<MockDraw>(draw);
            DrawStep* drawStep = alloc->make<DrawStep>(recordedDraw, step);

            BindingList* list = fBindingMatch;
            if (fBindingMatch) {
                // Found a compatible list during add(). If the draw is opaque it should be in the
                // front of the list to maximize early z rejection. Otherwise it should go to the
                // tail.
                bool dependsOnDst = step < 0 ? draw.dependsOnDst(draw.stepCount() - 1)
                                                : draw.dependsOnDst(step);

                if (dependsOnDst) {
                    fBindingMatch->fDraws.addToTail(drawStep);
                } else {
                    fBindingMatch->fDraws.addToHead(drawStep);
                }
            } else {
                // If we get here, we don't have a binding list with this draw's binding.
                list = alloc->make<BindingList>(binding);
                list->fDraws.addToHead(drawStep);
                fBindings.addToTail(list);
            }

            if (fMerge) {
                SkASSERT(fMerge->fBinding == list);
                fMerge->fBounds.join(draw.fBounds);
            } else {
                Bounds2* bounds = alloc->make<Bounds2>(draw.fBounds);
                fBounds.addToHead(bounds);
                bounds->fBinding = list;
            }
        }
    };

    struct BindingList2 {
        BindingList2(int64_t binding) : fBinding(binding) {}

        // All draw steps will have the same render type, step, paint ID, and data binding if they
        // are in the same linked list.
        SkTInternalLList<DrawStep> fDraws;

        int64_t fBinding;

        Rect fBounds = Rect::InfiniteInverted();

    private:
        // Managed by Layer
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingList2);
    };

    struct Layer4 {
        int fAddedCount = 0;

        // int fInsertIndex = 0; // remember best insertion index during test()
        // bool fBindingMatch = 0; // remember if we saw a matching binding during test()
        BindingList2* fBindingMatch = nullptr;

        SkTInternalLList<BindingList2> fBindings;

        // Managed by Device
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer4);

        Layer4(SkArenaAlloc*) { }

        BoundsTest test(const MockDraw& draw, int64_t binding) {
            if (draw.fRenderType != MockRenderType::kMultiStepWithStencil) {
                // We can disregard overlaps if there's no stencil overlap and a trivial binding
                // match. If there are multiple bindings then we need to test for overlaps.
                if (fBindings.head() == fBindings.tail() && fBindings.head()->fBinding == binding) {
                    fBindingMatch = fBindings.head();
                    return BoundsTest::kCompatibleOverlap;
                }
            }

            fBindingMatch = nullptr;
            Rect::ComplementRect testBounds{draw.fBounds};

            for (BindingList2* list : fBindings) {
                if (list->fBinding == binding) {
                    fBindingMatch = list;
                    // Don't bother checking bounds in this case if the step doesn't require
                    // disjoint use of the stencil buffer.
                    if (draw.fRenderType != MockRenderType::kMultiStepWithStencil) {
                        continue;
                    }
                }

                // Check for overlap
                if (list->fBounds.intersects(testBounds)) {
                    for (const DrawStep* d : list->fDraws) {
                        if (d->fDraw->fBounds.intersects(testBounds)) {
                            return BoundsTest::kIncompatibleOverlap;
                        }
                    }
                }
            }

            // Could technically be disjoint with a binding match, but that will be handled the
            // same as compatible with overlap.
            return fBindingMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint;
        }

        void trackOpaqueDraw(Rect drawBounds, int64_t binding) {
            fBindingMatch = nullptr;
            for (BindingList2* b : fBindings) {
                if (b->fBinding == binding) {
                    fBindingMatch = b;
                    break;
                }
            }
        }

        void add(SkArenaAlloc* alloc, int64_t binding, const MockDraw& draw, int step) {
            fAddedCount++;

            // Create a DrawStep to store
            const MockDraw* recordedDraw = alloc->make<MockDraw>(draw);
            DrawStep* drawStep = alloc->make<DrawStep>(recordedDraw, step);

            BindingList2* list = fBindingMatch;
            if (fBindingMatch) {
                // Found a compatible list during add(). If the draw is opaque it should be in the
                // front of the list to maximize early z rejection. Otherwise it should go to the
                // tail.
                bool dependsOnDst = step < 0 ? draw.dependsOnDst(draw.stepCount() - 1)
                                                : draw.dependsOnDst(step);

                if (dependsOnDst) {
                    fBindingMatch->fDraws.addToTail(drawStep);
                } else {
                    fBindingMatch->fDraws.addToHead(drawStep);
                }
                list->fBounds.join(draw.fBounds);
            } else {
                // If we get here, we don't have a binding list with this draw's binding.
                list = alloc->make<BindingList2>(binding);
                list->fDraws.addToHead(drawStep);
                list->fBounds = draw.fBounds;
                fBindings.addToTail(list);
            }
        }
    };



    using Layer = Layer4;

    void recordStep(const MockDraw& draw, int step) {
        // Search backwards through the layers to find a layer as early as possible. If we are
        // occluded or find a layer with the same binding, then we stop there.
        int64_t binding = this->getBinding(draw, step);

        // SkDebugf("recording draw %d %lld %f %f %f %f\n", draw.fZ, binding, draw.fBounds.left(), draw.fBounds.top(), draw.fBounds.right(), draw.fBounds.bot());

        SkTInternalLList<Layer>::Iter iter;
        Layer* current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
        Layer* targetLayer = nullptr;

        bool dependsOnDst = step < 0 ? draw.dependsOnDst(draw.stepCount() - 1)
                                        : draw.dependsOnDst(step);
        if (!dependsOnDst && draw.fRenderType != MockRenderType::kMultiStepWithStencil) {
            // No disjoint dependencies (for blending or for stencil buffer access), so just go to
            // the earliest layer.
            targetLayer = fLayers.head();
            current = nullptr;
            if (targetLayer) {
                targetLayer->trackOpaqueDraw(draw.fBounds, binding);
            }
        }

        int limit = 10;
        while (current && limit > 0) {
            // SkDebugf("checking layer\n");
            BoundsTest test = current->test(draw, binding);

            // SkDebugf("test %d == %d\n", draw.fZ, (int) test);

            if (test == BoundsTest::kIncompatibleOverlap) {
                if (dependsOnDst) {
                    // We cannot go earlier than this layer. If there was a newer layer, we can add to
                    // that one. If not, then take any matched bindings into a new layer with this new
                    // draw.
                    if (current->fNext) {
                        targetLayer = current->fNext;
                        break;
                    } else {
                        // ugh, moving the binding list to the new layer is messy because the bounds
                        // are possibly consolidated
                        break;
                    }
                }
            } else {
                // This layer could take the draw
                targetLayer = current;
                if (test == BoundsTest::kCompatibleOverlap || current->fBindingMatch) {
                    // This is the best we can do.  Put it into this layer.
                    break;
                } // else keep searching backwards for a layer with matching draws
            }

            current = iter.prev();
        }

        if (!targetLayer) {
            // If we get here, we don't have a layer to put it in so make a new one.
            // SkDebugf("making new layer for draw\n");
            targetLayer = fStorage.make<Layer>(&fStorage);
            fLayers.addToTail(targetLayer);
        }

        targetLayer->add(&fStorage, binding, draw, step);
    }

    int64_t getBinding(const MockDraw& draw, int step) {
        return (((int64_t) draw.pipelineIndex(std::max(0, step))) << 32) | draw.fDataBinding;
    }

    SkTInternalLList<Layer> fLayers;

    SkArenaAllocWithReset fStorage{0};

    bool fPrint = false;
};

}  // namespace skgpu::graphite

///////////////////////////////////////////////////////////////////////////////////////////////////

using namespace skgpu::graphite;

static std::unique_ptr<DrawGenerator> solid_color_text(SkISize bounds, int numDraws) {
    // Text runs are often arranged in a tight grid with not uncommonly a 1px overlap due to AA
    // outsets. Solid color has no binding change.
    return std::make_unique<DrawGenerator>(
            "solid_color_text",
            std::make_unique<TilingGeomGenerator>(SkISize{100, 100}, 99, 1, bounds,
                                                  MockRenderType::kAnalyticCoverageOnly),
            std::make_unique<SequentialPaintGenerator>(1, 1, 1, true, true),
            numDraws);
}

static std::unique_ptr<DrawGenerator> random_color_paths(SkISize bounds, int numDraws) {
    // Somewhat like motionmark paths, lots of relatively small path bounds that are random that
    // require the stencil buffer.
    return std::make_unique<DrawGenerator>(
            "random_color_paths",
            std::make_unique<RandomGeomGenerator>(SkISize{50, 50}, SkISize{200, 200}, bounds,
                                                std::initializer_list<MockRenderType>{MockRenderType::kMultiStepWithStencil}),
            std::make_unique<SequentialPaintGenerator>(1, 1, 1, true, true),
            numDraws);
}

static std::unique_ptr<DrawGenerator> random_color_strokes(SkISize bounds, int numDraws) {
    // Somewhat like motionmark strokes, lots of relatively small path bounds that are random
    // that do no require the stencil buffer (e.g. stroked).
    return std::make_unique<DrawGenerator>(
            "random_color_strokes",
            std::make_unique<RandomGeomGenerator>(SkISize{50, 50}, SkISize{200, 200}, bounds,
                                                std::initializer_list<MockRenderType>{MockRenderType::kNonAACoverageOnly}),
            std::make_unique<SequentialPaintGenerator>(1, 1, 1, true, true),
            numDraws);
}

static std::unique_ptr<DrawGenerator> random_images(SkISize bounds, int numDraws) {
    // Somewhat like motionmark bouncing images, overlapping random rectangles with different bindings
    return std::make_unique<DrawGenerator>(
            "random_images",
            std::make_unique<RandomGeomGenerator>(SkISize{100, 100}, SkISize{100, 100}, bounds,
                                                std::initializer_list<MockRenderType>{MockRenderType::kInnerFillRect}),
            std::make_unique<SequentialPaintGenerator>(1, 5, 1, true, true),
            numDraws);
}

static std::unique_ptr<DrawGenerator> random_icons(SkISize bounds, int numDraws) {
    // Somewhat like motionmark leaves, very small random rectangles (no inner fill) with different bindings
    return std::make_unique<DrawGenerator>(
            "random_icons",
            std::make_unique<RandomGeomGenerator>(SkISize{16, 16}, SkISize{32, 32}, bounds,
                                                std::initializer_list<MockRenderType>{MockRenderType::kAnalyticCoverageOnly}),
            std::make_unique<SequentialPaintGenerator>(1, 10, 1, false, false),
            numDraws);
}

static std::unique_ptr<DrawGenerator> total_random(SkISize bounds, int numDraws) {
    // Everything is randomized
    return std::make_unique<DrawGenerator>(
            "total_random",
            std::make_unique<RandomGeomGenerator>(SkISize{16, 16}, SkISize{320, 320}, bounds,
                                                   std::initializer_list<MockRenderType>{MockRenderType::kAnalyticCoverageOnly,
                                                                                         MockRenderType::kInnerFillRect,
                                                                                         MockRenderType::kMultiStepWithStencil,
                                                                                         MockRenderType::kNonAACoverageOnly}),
            std::make_unique<RandomPaintGenerator>(10, 10, 0.5f),
            numDraws);
}

// Good for ganesh, bad for graphite
static std::unique_ptr<DrawGenerator> pathologic1(SkISize bounds, int numDraws) {
    // Disjoint stacks of draws; each draw within a stack has the same binding; each stack has a
    // different binding. The draws for a stack come in sequence. Nothing is opaque. 5x5 stacks.
    int numStacks = 25;
    int stackDepth = 10;
    int stackWidth = std::max(sk_float_ceil2int(bounds.fWidth / 5.f), sk_float_ceil2int(bounds.fHeight / 5.f));
    return std::make_unique<DrawGenerator>(
            "pathologic1",
            std::make_unique<TilingGeomGenerator>(SkISize{stackWidth - 10, stackWidth - 10}, stackWidth, stackDepth, bounds,
                                                MockRenderType::kAnalyticCoverageOnly),
            std::make_unique<SequentialPaintGenerator>(1, numStacks, stackDepth, false, false),
            numDraws);
}

// Bad for ganesh and bad for graphite
static std::unique_ptr<DrawGenerator> pathologic2(SkISize bounds, int numDraws) {
    // Disjoint stacks of draws; each draw within a stack has the same binding; each stack has a
    // different binding. The draws come in Z order across all stacks. Nothing is opaque. 5x5 stacks.
    int numStacks = 25;
    int stackDepth = 10;
    int stackWidth = std::max(sk_float_ceil2int(bounds.fWidth / 5.f), sk_float_ceil2int(bounds.fHeight / 5.f));
    return std::make_unique<DrawGenerator>(
            "pathologic2",
            std::make_unique<TilingGeomGenerator>(SkISize{stackWidth - 10, stackWidth - 10}, stackWidth, 1, bounds,
                                                MockRenderType::kAnalyticCoverageOnly),
            std::make_unique<SequentialPaintGenerator>(1, numStacks, 1, false, false),
            numDraws);
}

// Bad for ganesh, good for graphite (w/ brute force bounds manager)
static std::unique_ptr<DrawGenerator> pathologic3(SkISize bounds, int numDraws) {
    // Disjoint stacks of draws; each draw within a stack has a different binding. The draws for a
    // stack come in sequence. Nothing is opaque. 5x5 stacks.
    int numStacks = 25;
    int stackDepth = 10;
    int stackWidth = std::max(sk_float_ceil2int(bounds.fWidth / 5.f), sk_float_ceil2int(bounds.fHeight / 5.f));
    return std::make_unique<DrawGenerator>(
            "pathologic3",
            std::make_unique<TilingGeomGenerator>(SkISize{stackWidth - 10, stackWidth - 10}, stackWidth, stackDepth, bounds,
                                                MockRenderType::kAnalyticCoverageOnly),
            std::make_unique<SequentialPaintGenerator>(1, stackDepth, 1, false, false),
            numDraws);
}

// Good for ganesh, good for graphite
static std::unique_ptr<DrawGenerator> pathologic4(SkISize bounds, int numDraws) {
    // Disjoint stacks of draws; each draw within a stack a different binding; The draws come in Z
    // order across all stacks. Nothing is opaque. 5x5 stacks.
    int numStacks = 25;
    int stackDepth = 10;
    int stackWidth = std::max(sk_float_ceil2int(bounds.fWidth / 5.f), sk_float_ceil2int(bounds.fHeight / 5.f));
    return std::make_unique<DrawGenerator>(
            "pathologic4",
            std::make_unique<TilingGeomGenerator>(SkISize{stackWidth - 10, stackWidth - 10}, stackWidth, 1, bounds,
                                                MockRenderType::kAnalyticCoverageOnly),
            std::make_unique<SequentialPaintGenerator>(1, stackDepth, numStacks, false, false),
            numDraws);
}

static std::unique_ptr<DrawGenerator> pathologic5(SkISize bounds, int numDraws) {
    // Lots of overlapping, very large draws with lots of different bindings.
    return std::make_unique<DrawGenerator>(
            "pathologic5",
            std::make_unique<RandomGeomGenerator>(SkISize{bounds.fWidth / 2, bounds.fHeight / 2}, bounds, bounds,
                                                std::initializer_list<MockRenderType>{MockRenderType::kAnalyticCoverageOnly}),
            std::make_unique<RandomPaintGenerator>(5, 5, 0.0f),
            numDraws);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static constexpr SkISize kDefaultBounds{1000, 1000};

template <typename DrawFn>
static BatchingBenchmark* ganesh_bench(DrawFn drawFn, int numDraws) {
    std::unique_ptr<DrawGenerator> draws = drawFn(kDefaultBounds, numDraws);
    std::unique_ptr<MockDevice> device = std::make_unique<GaneshDevice>();
    return new BatchingBenchmark(std::move(device), std::move(draws));
}

template <typename DrawFn>
static BatchingBenchmark* bruteforce_bench(DrawFn drawFn, int numDraws) {
    std::unique_ptr<DrawGenerator> draws = drawFn(kDefaultBounds, numDraws);
    std::unique_ptr<MockDevice> device = std::make_unique<BruteForceDevice>();
    return new BatchingBenchmark(std::move(device), std::move(draws));
}

template <typename DrawFn>
static BatchingBenchmark* graphite_bench(DrawFn drawFn, int numDraws) {
    std::unique_ptr<DrawGenerator> draws = drawFn(kDefaultBounds, numDraws);
    std::unique_ptr<MockDevice> device = std::make_unique<GraphiteDevice>(kDefaultBounds);
    return new BatchingBenchmark(std::move(device), std::move(draws));
}

template <typename DrawFn>
static BatchingBenchmark* experiment_bench(DrawFn drawFn, int numDraws) {
    std::unique_ptr<DrawGenerator> draws = drawFn(kDefaultBounds, numDraws);
    std::unique_ptr<MockDevice> device = std::make_unique<ExperimentDevice>();
    return new BatchingBenchmark(std::move(device), std::move(draws));
}

// TODO: experimental


#define BATCH_BENCH(drawFn, numDraws) \
    DEF_BENCH(return experiment_bench(drawFn, numDraws); ) \
    DEF_BENCH(return graphite_bench(drawFn, numDraws); ) \
    DEF_BENCH(return ganesh_bench(drawFn, numDraws); ) \
    DEF_BENCH(return bruteforce_bench(drawFn, numDraws); )

///////////////////////////////////////////////////////////////////////////////////////////////////

BATCH_BENCH(solid_color_text, 250)
BATCH_BENCH(random_color_paths, 250)
BATCH_BENCH(random_color_strokes, 250)
BATCH_BENCH(random_images, 250)
BATCH_BENCH(random_icons, 250)
BATCH_BENCH(total_random, 250)

BATCH_BENCH(solid_color_text, 1000)
BATCH_BENCH(random_color_paths, 1000)
BATCH_BENCH(random_color_strokes, 1000)
BATCH_BENCH(random_images, 1000)
BATCH_BENCH(random_icons, 1000)
BATCH_BENCH(total_random, 1000)

BATCH_BENCH(solid_color_text, 2500)
BATCH_BENCH(random_color_paths, 2500)
BATCH_BENCH(random_color_strokes, 2500)
BATCH_BENCH(random_images, 2500)
BATCH_BENCH(random_icons, 2500)
BATCH_BENCH(total_random, 2500)

BATCH_BENCH(solid_color_text, 5000)
BATCH_BENCH(random_color_paths, 5000)
BATCH_BENCH(random_color_strokes, 5000)
BATCH_BENCH(random_images, 5000)
BATCH_BENCH(random_icons, 5000)
BATCH_BENCH(total_random, 5000)

BATCH_BENCH(solid_color_text, 10000)
BATCH_BENCH(random_color_paths, 10000)
BATCH_BENCH(random_color_strokes, 10000)
BATCH_BENCH(random_images, 10000)
BATCH_BENCH(random_icons, 10000)
BATCH_BENCH(total_random, 10000)

BATCH_BENCH(pathologic1, 250)
BATCH_BENCH(pathologic2, 250)
BATCH_BENCH(pathologic3, 250)
BATCH_BENCH(pathologic4, 250)
BATCH_BENCH(pathologic5, 250)

BATCH_BENCH(pathologic1, 1000)
BATCH_BENCH(pathologic2, 1000)
BATCH_BENCH(pathologic3, 1000)
BATCH_BENCH(pathologic4, 1000)
BATCH_BENCH(pathologic5, 1000)

BATCH_BENCH(pathologic1, 2500)
BATCH_BENCH(pathologic2, 2500)
BATCH_BENCH(pathologic3, 2500)
BATCH_BENCH(pathologic4, 2500)
BATCH_BENCH(pathologic5, 2500)

BATCH_BENCH(pathologic1, 5000)
BATCH_BENCH(pathologic2, 5000)
BATCH_BENCH(pathologic3, 5000)
BATCH_BENCH(pathologic4, 5000)
BATCH_BENCH(pathologic5, 5000)

BATCH_BENCH(pathologic1, 10000)
BATCH_BENCH(pathologic2, 10000)
BATCH_BENCH(pathologic3, 10000)
BATCH_BENCH(pathologic4, 10000)
BATCH_BENCH(pathologic5, 10000)
