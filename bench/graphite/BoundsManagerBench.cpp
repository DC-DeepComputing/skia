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

            // SkDebugf("draw %p (%dx%d) with %d + %d  (pipeline %d)\n", &draw,
            //     (int) draw.fRenderType, step, draw.fPaintID, draw.fDataBinding,
            //     draw.pipelineIndex(step));

            fDraws.push_back({&draw, step});
        }
    }

    void endRenderPass() {
        if (fFirstPass) {
            SkDebugf("# recorded steps: %d, # pipeline changes: %d, # binding changes: %d\n",
                     fDraws.size(), fNumPipelineChanges, fNumBindingChanges);
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
        return SkSL::String::printf("%s-%d", fName, fNumDraws);
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
    SequentialPaintGenerator(int numPipelines, int numBindings, bool evenIsOpaque, bool oddIsOpaque)
            : fNumPipelines(numPipelines)
            , fNumBindings(numBindings)
            , fEvenBindingsAreOpaque(evenIsOpaque)
            , fOddBindingsAreOpaque(oddIsOpaque) {}

    std::string name() const override {
        return SkSL::String::printf("seq(%dx%d,%d,%d)",
                                    fNumPipelines, fNumBindings,
                                    fEvenBindingsAreOpaque, fOddBindingsAreOpaque);
    }

    std::tuple<int, int, bool> nextPaint() override {
        int paintID = (fCurrentPaint / fNumBindings) % fNumPipelines;
        int binding = fCurrentPaint % fNumBindings;
        bool isOpaque = (fCurrentPaint & 0b1) ? fOddBindingsAreOpaque : fEvenBindingsAreOpaque;

        fCurrentPaint++;
        return {paintID, binding, isOpaque};
    }

private:
    const int fNumPipelines;
    const int fNumBindings;
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
    TilingGeomGenerator(SkISize drawSize, int tileShift, SkISize targetSize, MockRenderType type)
            : fDrawSize(drawSize)
            , fTargetSize(targetSize)
            , fTileShift(tileShift)
            , fRenderType(type) {}

    std::string name() const override {
        return SkSL::String::printf("tile(%dx%dx%d in %dx%d)",
                                    fDrawSize.fWidth, fDrawSize.fHeight, fTileShift,
                                    fTargetSize.fWidth, fTargetSize.fHeight);
    }

    std::pair<MockRenderType, Rect> nextGeometry() override {
        Rect bounds = Rect::XYWH(fNextX, fNextY, fDrawSize.fWidth, fDrawSize.fHeight);
        if (bounds.right() > fTargetSize.fWidth) {
            fNextX = 0;
            if (bounds.bot() > fTargetSize.fHeight) {
                fNextY = 0;
            } else {
                fNextY += fTileShift;
            }
        } else {
            fNextX += fTileShift;
        }

        return {fRenderType, bounds};
    }

private:
    const SkISize fDrawSize;
    const SkISize fTargetSize;
    const int fTileShift;
    const MockRenderType fRenderType;

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
        fName = SkSL::String::printf("GraphiteBatching-%s-%s",
                                     fDrawGenerator->name().c_str(), fDevice->name().c_str());
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
            fRenderer->endRenderPass();

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
        for (const Layer* currentLayer : fLayers) {
            // SkDebugf("Layer:\n");
            // for (int i = 0; i < currentLayer->fSlotsFilled; ++i) {
            //     Rect r = currentLayer->fSlots[i].fBounds;
            //     // SkDebugf("  slot %d: %f %f %f %f with %lld\n", i, r.left(), r.top(), r.right(), r.bot(), currentLayer->fSlots[i].fBinding);
            // }

            for (const BindingList* currentBinding : currentLayer->fBindings) {
                // SkDebugf("  binding %lld\n", currentBinding->fBinding);

                const DrawStep* firstStep = currentBinding->fDraws.head();
                if (firstStep) {
                    int stepCount = firstStep->fStep >= 0 ? 1 : firstStep->fDraw->stepCount();
                    for (int s = 0; s < stepCount; ++s) {
                        for (const DrawStep* currentStep : currentBinding->fDraws) {
                            // Rect r = currentStep->fDraw->fBounds;
                            int step = std::max(currentStep->fStep, s);
                            // int64_t binding = this->getBinding(*currentStep->fDraw, currentStep->fStep);
                            // SkDebugf("    draw %d|%d with %f %f %f %f and dst %d / binding %lld\n",
                            //         currentStep->fDraw->fZ, step,
                            //         r.left(), r.top(), r.right(), r.bot(),
                            //         currentStep->fDraw->dependsOnDst(step), binding);
                            renderer->draw(*currentStep->fDraw, step);
                        }
                    }
                }
            }
        }
    }

    void reset() override {
        fLayers.reset();
        fStorage.reset();
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

    struct Layer {
        std::array<Bounds, 64> fSlots; // FIXME: how many?
        int fSlotsFilled = 0;

        int fInsertIndex = 0; // remember best insertion index during test()
        bool fBindingMatch = 0; // remember if we saw a matching binding during test()

        SkTInternalLList<BindingList> fBindings;

        // Managed by Device
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

        BoundsTest test(Rect drawBounds, int64_t binding) {
            bool intersectsWithBindingMatch = false;

            fInsertIndex = fSlotsFilled;
            fBindingMatch = false;
            float minArea = std::numeric_limits<float>::max();

            Rect::ComplementRect testBounds{drawBounds};
            for (int i = 0; i < fSlotsFilled; ++i) {
                if (fSlots[i].fBinding == binding) {
                    fBindingMatch = true;
                }

                if (fSlots[i].fBounds.intersects(testBounds)) {
                    if (fSlots[i].fBinding != binding) {
                        // The draw overlaps a non-compatible draw so it shouldn't belong to this
                        // layer at all.
                        return BoundsTest::kIncompatibleOverlap;
                    } else {
                        // The draw overlaps a compatible draw, so it could be merged into this
                        // layer but it cannot be added to any prior layers.
                        intersectsWithBindingMatch = true;
                    }
                }

                if (fSlotsFilled == (int) fSlots.size()) {
                    // Keep track of the best bounds to join with
                    float mergedArea = fSlots[i].fBounds.makeJoin(drawBounds).area();
                    if (mergedArea < minArea) {
                        minArea = mergedArea;
                        fInsertIndex = i;
                    }
                }
            }
            return intersectsWithBindingMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint;
        }

        void add(SkArenaAlloc* alloc, int64_t binding, const MockDraw& draw, int step) {
            SkASSERT(fInsertIndex >= 0 && fInsertIndex < (int) fSlots.size());
            // Record the bounds in the approximate layer bounds
            if (fSlots[fInsertIndex].fBounds.isEmptyNegativeOrNaN()) {
                fSlots[fInsertIndex].fBounds = draw.fBounds;
                fSlots[fInsertIndex].fBinding = binding;
                SkASSERT(fSlotsFilled < (int) fSlots.size());
                fSlotsFilled++;
            } else {
                SkASSERT(fSlotsFilled == (int) fSlots.size());
                fSlots[fInsertIndex].fBounds.join(draw.fBounds);
                if (fSlots[fInsertIndex].fBinding != binding) {
                    fSlots[fInsertIndex].fBinding = -1;
                }
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

    void recordStep(const MockDraw& draw, int step) {
        // Search backwards through the layers to find a layer as early as possible. If we are
        // occluded or find a layer with the same binding, then we stop there.
        int64_t binding = this->getBinding(draw, step);

        SkTInternalLList<Layer>::Iter iter;
        Layer* current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
        while (current) {
            // SkDebugf("checking layer\n");
            BoundsTest test = current->test(draw.fBounds, binding);
            if (draw.fRenderType == MockRenderType::kMultiStepWithStencil && test == BoundsTest::kCompatibleOverlap) {
                test = BoundsTest::kIncompatibleOverlap;
            }
            // SkDebugf("test %d == %d\n", draw.fZ, (int) test);

            if (test == BoundsTest::kIncompatibleOverlap) {
                // We cannot go earlier than this layer. If there was a newer layer, we can add to
                // that one. If not, then take any matched bindings into a new layer with this new
                // draw.
                if (current->fPrev) {
                    current->fPrev->add(&fStorage, binding, draw, step);
                    return;
                } else {
                    // ugh, moving the binding list to the new layer is messy because the bounds
                    // are possibly consolidated
                    break;
                }
            } else if (test == BoundsTest::kCompatibleOverlap || current->fBindingMatch) {
                // This is the best we can do.  Put it into this layer.
                current->add(&fStorage, binding, draw, step);
                return;
            }  // else keep searching backwards for a layer with matching draws

            current = iter.prev();
        }

        // SkDebugf("adding new layer\n");

        // If we get here, we don't have a layer to put it in so make a new one.
        Layer* newLayer = fStorage.make<Layer>();
        newLayer->add(&fStorage, binding, draw, step);
        fLayers.addToTail(newLayer);
    }

    int64_t getBinding(const MockDraw& draw, int step) {
        return (((int64_t) draw.pipelineIndex(step)) << 32) | draw.fPaintID;
    }

    SkTInternalLList<Layer> fLayers;

    SkArenaAllocWithReset fStorage{0};
};

}  // namespace skgpu::graphite

///////////////////////////////////////////////////////////////////////////////////////////////////

using namespace skgpu::graphite;

static std::unique_ptr<DrawGenerator> solid_color_text(SkISize bounds, int numDraws) {
    // Text runs are often arranged in a tight grid with not uncommonly a 1px overlap due to AA
    // outsets. Solid color has no binding change.
    return std::make_unique<DrawGenerator>(
            "solid_color_text",
            std::make_unique<TilingGeomGenerator>(SkISize{100, 100}, 99, bounds,
                                                  MockRenderType::kAnalyticCoverageOnly),
            std::make_unique<SequentialPaintGenerator>(1, 1, true, true),
            numDraws);
}

static std::unique_ptr<DrawGenerator> random_color_paths(SkISize bounds, int numDraws) {
    // Somewhat like motionmark paths, lots of relatively small path bounds that are random that
    // require the stencil buffer.
    return std::make_unique<DrawGenerator>(
            "random_color_paths",
            std::make_unique<RandomGeomGenerator>(SkISize{50, 50}, SkISize{200, 200}, bounds,
                                                std::initializer_list<MockRenderType>{MockRenderType::kMultiStepWithStencil}),
            std::make_unique<SequentialPaintGenerator>(1, 1, true, true),
            numDraws);
}

static std::unique_ptr<DrawGenerator> random_color_strokes(SkISize bounds, int numDraws) {
    // Somewhat like motionmark strokes, lots of relatively small path bounds that are random
    // that do no require the stencil buffer (e.g. stroked).
    return std::make_unique<DrawGenerator>(
            "random_color_strokes",
            std::make_unique<RandomGeomGenerator>(SkISize{50, 50}, SkISize{200, 200}, bounds,
                                                std::initializer_list<MockRenderType>{MockRenderType::kNonAACoverageOnly}),
            std::make_unique<SequentialPaintGenerator>(1, 1, true, true),
            numDraws);
}

static std::unique_ptr<DrawGenerator> random_images(SkISize bounds, int numDraws) {
    // Somewhat like motionmark bouncing images, overlapping random rectangles with different bindings
    return std::make_unique<DrawGenerator>(
            "random_images",
            std::make_unique<RandomGeomGenerator>(SkISize{100, 100}, SkISize{100, 100}, bounds,
                                                std::initializer_list<MockRenderType>{MockRenderType::kInnerFillRect}),
            std::make_unique<SequentialPaintGenerator>(1, 5, true, true),
            numDraws);
}

static std::unique_ptr<DrawGenerator> random_icons(SkISize bounds, int numDraws) {
    // Somewhat like motionmark leaves, very small random rectangles (no inner fill) with different bindings
    return std::make_unique<DrawGenerator>(
            "random_icons",
            std::make_unique<RandomGeomGenerator>(SkISize{16, 16}, SkISize{32, 32}, bounds,
                                                std::initializer_list<MockRenderType>{MockRenderType::kAnalyticCoverageOnly}),
            std::make_unique<SequentialPaintGenerator>(1, 10, false, false),
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

BATCH_BENCH(solid_color_text, 12000)
BATCH_BENCH(random_color_paths, 12000)
BATCH_BENCH(random_color_strokes, 12000)
BATCH_BENCH(random_images, 12000)
BATCH_BENCH(random_icons, 12000)
