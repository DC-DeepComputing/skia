/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/base/SkRandom.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawListLayer.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "tests/Test.h"
#include <vector>
#include <cmath>
#include <memory>

namespace skgpu::graphite {

namespace {

struct TestDraw {
    int originalIndex;
    Rect bounds;
    UniquePaintParamsID paintID;
    bool dependsOnDst;
    bool hasDependency;
};

} // namespace

class SimpleBoundsLayerSortTest {
public:
    static void Run(skiatest::Reporter* reporter, Context* context) {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        const RendererProvider* rendererProvider = recorder->priv().rendererProvider();

        const Renderer* rRectRenderer = rendererProvider->analyticRRect();
        const Renderer* quadRenderer = rendererProvider->perEdgeAAQuad();

        SkRandom rand;
        PipelineDataGatherer gatherer(Layout::kStd140);

        const int kNumDraws = 50;
        const int kCanvasSize = 1000;

        DrawListLayer drawLayer;
        drawLayer.reset(LoadOp::kClear, SkColors::kTransparent);

        static constexpr int kGridCellSize = 16;
        static constexpr int kMaxBruteForceN = 64;
        static constexpr int kMaxGridSize = 32;
        std::unique_ptr<BoundsManager> boundsManager = std::make_unique<HybridBoundsManager>(
                  SkISize::Make(kCanvasSize, kCanvasSize),
                  kGridCellSize,
                  kMaxBruteForceN,
                  kMaxGridSize);
        PaintersDepth currentDepth;

        std::vector<TestDraw> inputs;
        inputs.reserve(kNumDraws);

        for (int i = 0; i < kNumDraws; ++i) {
            TestDraw draw;
            draw.originalIndex = i;

            float x = rand.nextRangeF(0, kCanvasSize - 50);
            float y = rand.nextRangeF(0, kCanvasSize - 50);
            float w = rand.nextRangeF(10, 100);
            float h = rand.nextRangeF(10, 100);
            draw.bounds = Rect(x, y, x + w, y + h);

            draw.paintID = UniquePaintParamsID(rand.nextRangeU(1, 5));
            draw.dependsOnDst = (rand.nextF() < 0.1f);

            DrawOrder order(currentDepth.next());
            bool isOpaque = !draw.dependsOnDst && rand.nextBool();

            if (!isOpaque) {
                CompressedPaintersOrder prevDraw = boundsManager->getMostRecentDraw(draw.bounds);
                order.dependsOnPaintersOrder(prevDraw);
                draw.hasDependency = (prevDraw != CompressedPaintersOrder());
            } else {
                draw.hasDependency = false;
            }

            inputs.push_back(draw);
            const Renderer* selectedRenderer = nullptr;

            auto shape = [&]() {
                if (rand.nextBool()) {
                    selectedRenderer = rRectRenderer;
                    SkRRect rrect = SkRRect::MakeOval(draw.bounds.asSkRect());
                    return Shape(rrect);
                } else {
                    selectedRenderer = quadRenderer;
                    return Shape(draw.bounds.asSkRect());
                }
            }();

            Geometry geom(shape);
            Clip clip(draw.bounds, draw.bounds, {}, {}, nullptr);
            Transform localToDevice = Transform::Identity();

            SkEnumBitMask<DstUsage> dstUsage = draw.dependsOnDst ? DstUsage::kDependsOnDst
                                                                 : DstUsage::kNone;
            drawLayer.recordDraw(selectedRenderer,
                                 localToDevice,
                                 geom,
                                 clip,
                                 order,
                                 draw.paintID,
                                 dstUsage,
                                 BarrierType::kNone,
                                 &gatherer,
                                 nullptr);
            boundsManager->recordDraw(draw.bounds, order.paintOrder());
            currentDepth = order.depth();
        }

        std::vector<const DrawListLayer::RecordedDraw*> flattenedDraws;

        for (const auto& layer : drawLayer.fLayers) {
            for (const auto* binding : layer->fBindings) {
                for (const auto* recordedDraw : binding->fDraws) {
                    flattenedDraws.push_back(recordedDraw);
                }
            }
        }

        REPORTER_ASSERT(reporter, flattenedDraws.size() == (size_t)kNumDraws);

        // Verify Painter's Order
        for (size_t i = 0; i < flattenedDraws.size(); ++i) {
            const DrawListLayer::RecordedDraw* drawA = flattenedDraws[i];
            int originalIndexA = FindOriginalIndex(inputs, drawA);

            for (size_t j = i + 1; j < flattenedDraws.size(); ++j) {
                const DrawListLayer::RecordedDraw* drawB = flattenedDraws[j];
                int originalIndexB = FindOriginalIndex(inputs, drawB);

                bool intersects = drawA->fDrawParams.drawBounds().intersects(drawB->fDrawParams.drawBounds());

                // TODO failing probably due to reordering
                if (intersects) {
                    if (originalIndexA > originalIndexB) {
                        ERRORF(reporter, "Painter's order violation! \n"
                                         "Executed: Draw %d then Draw %d\n"
                                         "Recorded: Draw %d was recorded AFTER Draw %d\n"
                                         "Bounds Intersect: Yes",
                                         originalIndexA, originalIndexB,
                                         originalIndexA, originalIndexB);
                    }
                }
            }
        }
    }

private:
    // TODO find a better way to track this.
    static int FindOriginalIndex(const std::vector<TestDraw>& inputs,
                                 const DrawListLayer::RecordedDraw* recorded) {
        SkRect r = recorded->fDrawParams.drawBounds().asSkRect();
        for (const auto& input : inputs) {
            SkRect inputBounds = input.bounds.asSkRect();
            if (std::abs(inputBounds.x() - r.x()) < 0.001f &&
                std::abs(inputBounds.y() - r.y()) < 0.001f &&
                std::abs(inputBounds.width() - r.width()) < 0.001f &&
                std::abs(inputBounds.height() - r.height()) < 0.001f) {

                return input.originalIndex;
            }
        }
        return -1;
    }
};

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(BoundsLayerSortTest, reporter, context, CtsEnforcement::kNextRelease) {
    SimpleBoundsLayerSortTest::Run(reporter, context);
}

} // namespace skgpu::graphite
