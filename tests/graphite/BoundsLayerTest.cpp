/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

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

#include <cmath>
#include <memory>
#include <unordered_map>
#include <vector>

namespace skgpu::graphite {

class SimpleBoundsLayerSortTest {
public:
    struct DrawTestMetadata {
        bool usesStencil;
        bool dependsOnDst;
        PaintersDepth paintersDepth;
        DrawListLayer::LayerKey key;
    };
    using TestMetadataMap =
            std::unordered_map<const DrawListLayer::RecordedDraw*, DrawTestMetadata>;

    static void Run(skiatest::Reporter* reporter, Context* context) {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        const RendererProvider* rendererProvider = recorder->priv().rendererProvider();

        // Add the tessellating renderers into the mix for depth stenciling
        const Renderer* rRectRenderer = rendererProvider->analyticRRect();
        const Renderer* quadRenderer = rendererProvider->perEdgeAAQuad();
        const Renderer* tessWedgesRenderer =
                rendererProvider->stencilTessellatedWedges(SkPathFillType::kWinding);
        const Renderer* tessCurvesRenderer =
                rendererProvider->stencilTessellatedCurvesAndTris(SkPathFillType::kWinding);

        SkRandom rand;
        PipelineDataGatherer gatherer(Layout::kStd140);

        constexpr int kNumDraws = 1000;
        constexpr int kCanvasSize = 1000;

        DrawListLayer drawLayer;
        SkTDArray<std::pair<DrawListLayer::LayerKey, const DrawListLayer::RecordedDraw*>> history;
        // Attach the hook into drawLayer
#if defined(GPU_TEST_UTILS)
        drawLayer.setDrawObserver(
                [&](const DrawListLayer::LayerKey& key, const DrawListLayer::RecordedDraw* draw) {
                    history.push_back({key, draw});
                });
#endif

        PaintersDepth currentDepth;
        DisjointStencilIndex stencil;
        TestMetadataMap drawMetadata;
        drawLayer.reset(LoadOp::kClear, SkColors::kTransparent);

        uint32_t stepCount = 0;
        for (int i = 0; i < kNumDraws; ++i) {
            float x = rand.nextRangeF(0, kCanvasSize - 50);
            float y = rand.nextRangeF(0, kCanvasSize - 50);
            float w = rand.nextRangeF(10, 100);
            float h = rand.nextRangeF(10, 100);
            Rect bounds = Rect(x, y, x + w, y + h);

            PaintersDepth pDepth = currentDepth.next();
            UniquePaintParamsID paintID = UniquePaintParamsID(rand.nextULessThan(1024));
            bool dependsOnDst = (rand.nextF() < 0.5f);

            bool usesStencil = false;
            const Renderer* selectedRenderer = nullptr;
            Shape shape = [&]() {
                uint32_t type = rand.nextULessThan(4);
                switch (type) {
                    case 0:
                        selectedRenderer = quadRenderer;
                        return Shape(bounds.asSkRect());
                    case 1:
                        selectedRenderer = rRectRenderer;
                        return Shape(SkRRect::MakeOval(bounds.asSkRect()));
                    case 2:
                        selectedRenderer = tessWedgesRenderer;
                        usesStencil = (selectedRenderer->depthStencilFlags() &
                                       DepthStencilFlags::kStencil) != DepthStencilFlags::kNone;
                        return Shape(SkRRect::MakeRectXY(bounds.asSkRect(), 10.f, 10.f));
                    case 3:
                    default:
                        selectedRenderer = tessCurvesRenderer;
                        usesStencil = (selectedRenderer->depthStencilFlags() &
                                       DepthStencilFlags::kStencil) != DepthStencilFlags::kNone;
                        return Shape(SkRRect::MakeRectXY(bounds.asSkRect(), 10.f, 10.f));
                }
            }();

            DrawOrder order(pDepth);
            if (usesStencil) {
                order.dependsOnStencil(stencil.next());
            }

            Geometry geom(shape);
            Clip clip(bounds, bounds, {}, {}, nullptr);
            Transform localToDevice = Transform::Identity();

            SkEnumBitMask<DstUsage> dstUsage =
                    dependsOnDst ? DstUsage::kDependsOnDst : DstUsage::kNone;
            drawLayer.recordDraw(selectedRenderer,
                                 localToDevice,
                                 geom,
                                 clip,
                                 order,
                                 paintID,
                                 dstUsage,
                                 BarrierType::kNone,
                                 &gatherer,
                                 nullptr);
            currentDepth = order.depth();

            // track some extra details on each draw
            for (auto& step : history) {
                drawMetadata[step.second] = {usesStencil, dependsOnDst, pDepth, step.first};
            }

            history.clear();
            stepCount += selectedRenderer->numRenderSteps();
        }

        REPORTER_ASSERT(reporter, drawMetadata.size() == stepCount);
        for (const auto& layer : drawLayer.fLayers) {
            for (const auto* bindingList : layer->fBindings) {
                ValidateBindingList(reporter, bindingList, drawMetadata);
            }
        }
    }

    static void ValidateBindingList(skiatest::Reporter* reporter,
                                    const DrawListLayer::BindingList* list,
                                    const TestMetadataMap& metadataMap) {
        if (!list || list->fDraws.isEmpty()) return;

        bool seenDependent = false;
        const DrawListLayer::RecordedDraw* current = list->fDraws.head();
        const DrawListLayer::LayerKey listKey = list->fKey;

        while (current) {
            auto c = metadataMap.find(current);
            // We use a strict check here; if missing, we can't really proceed safely.
            bool foundMetadata = (c != metadataMap.end());
            REPORTER_ASSERT(reporter, foundMetadata, "RecordedDraw missing from test metadata!");
            if (!foundMetadata) return;

            const DrawTestMetadata& curr = c->second;
            REPORTER_ASSERT(reporter, curr.key == listKey, "Mismatched draw found in list.");

            bool isDependent = curr.dependsOnDst;
            if (seenDependent && !isDependent) {
                REPORTER_ASSERT(reporter,
                                false,
                                "Opaque draw found after non-opaque draw. Sort order broken.");
            }
            if (isDependent) seenDependent = true;

            const DrawListLayer::RecordedDraw* previous = list->fDraws.head();
            while (previous != current) {
                if (previous->fDrawParams.drawBounds().intersects(
                            current->fDrawParams.drawBounds())) {
                    auto p = metadataMap.find(previous);
                    if (p == metadataMap.end()) {
                        REPORTER_ASSERT(reporter, false, "Previous draw missing from metadata");
                        return;
                    }
                    const DrawTestMetadata& prev = p->second;

                    PaintersDepth prevDepth = prev.paintersDepth;
                    PaintersDepth currDepth = curr.paintersDepth;
                    bool prevIsDep = prev.dependsOnDst;

                    if (curr.usesStencil && prev.usesStencil) {
                        REPORTER_ASSERT(reporter,
                                        false,
                                        "Two overlapping Stencil draws found in same BindingList!");
                    }

                    // If both are non-opaque, must obey painter's order
                    if (prevIsDep && isDependent) {
                        REPORTER_ASSERT(reporter,
                                        prevDepth <= currDepth,
                                        "Overlapping non-opaque draws out of order!");
                    }

                    // If both are opaque xpect reverse submission order
                    if (!prevIsDep && !isDependent) {
                        REPORTER_ASSERT(reporter,
                                        prevDepth >= currDepth,
                                        "Overlapping opaque draws not in Front-to-Back order");
                    }
                }
                previous = previous->fNext;
            }

            current = current->fNext;
        }
    }
};

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(BoundsLayerSortTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    SimpleBoundsLayerSortTest::Run(reporter, context);
}

}  // namespace skgpu::graphite
