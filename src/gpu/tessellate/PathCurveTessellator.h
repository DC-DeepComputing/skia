/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_PathCurveTessellator_DEFINED
#define tessellate_PathCurveTessellator_DEFINED

#include "src/core/SkArenaAlloc.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/PathTessellator.h"

namespace skgpu {

// Draws an array of "outer curve" patches. Each patch is an independent 4-point curve, representing
// either a cubic or a conic. Quadratics are converted to cubics and triangles are converted to
// conics with w=Inf.
class PathCurveTessellator final : public PathTessellator {
public:
    using Writer = PatchWriter<Optional<PatchAttribs::kColor>,
                               Optional<PatchAttribs::kWideColorIfEnabled>,
                               Optional<PatchAttribs::kExplicitCurveType>,
                               AddTrianglesWhenChopping,
                               DiscardFlatCurves>;

    static PathCurveTessellator* Make(SkArenaAlloc* arena,
                                      bool infinitySupport,
                                      PatchAttribs attribs = PatchAttribs::kNone) {
        return arena->make<PathCurveTessellator>(infinitySupport, attribs);
    }

    PathCurveTessellator(bool infinitySupport,
                         PatchAttribs attribs = PatchAttribs::kNone)
            : PathTessellator(infinitySupport, attribs) {}

    static int PatchPreallocCount(int totalCombinedPathVerbCnt);

    static int WritePatches(Writer&& patchWriter,
                            const SkMatrix& shaderMatrix,
                            const PathDrawList& pathDrawList);

    // Size of the vertex buffer to use when rendering with a fixed count shader.
    constexpr static int FixedVertexBufferSize(int maxFixedResolveLevel) {
        return ((1 << maxFixedResolveLevel) + 1) * sizeof(SkPoint);
    }

    // Writes the vertex buffer to use when rendering with a fixed count shader.
    static void WriteFixedVertexBuffer(VertexWriter, size_t bufferSize);

    // Size of the index buffer to use when rendering with a fixed count shader.
    constexpr static int FixedIndexBufferSize(int maxFixedResolveLevel) {
        return NumCurveTrianglesAtResolveLevel(maxFixedResolveLevel) * 3 * sizeof(uint16_t);
    }

    // Writes the index buffer to use when rendering with a fixed count shader.
    static void WriteFixedIndexBuffer(VertexWriter vertexWriter, size_t bufferSize) {
        WriteFixedIndexBufferBaseIndex(std::move(vertexWriter), bufferSize, 0);
    }

    static void WriteFixedIndexBufferBaseIndex(VertexWriter, size_t bufferSize, uint16_t baseIndex);

#if SK_GPU_V1
    // PathCurveTessellator exposes the inner pieces used in prepare() so that
    // PathInnerTriangulateOp can inject breadcrumb triangles so that the normal stenciled curves
    // seam correctly with its inner triangulation.
    Writer makeWriter(GrMeshDrawTarget* target, int preallocCount, int maxTessellationSegments) {
        return Writer{target, &fVertexChunkArray, fAttribs, maxTessellationSegments, preallocCount};
    }

    void writeAndPrepare(GrMeshDrawTarget* target,
                         Writer&& patchWriter,
                         const SkMatrix& shaderMatrix,
                         const PathDrawList& pathDrawList,
                         bool willUseTessellationShaders) {
        int resolveLevel = WritePatches(std::move(patchWriter), shaderMatrix, pathDrawList);
        this->updateResolveLevel(resolveLevel);
        if (!willUseTessellationShaders) {
            this->prepareFixedCountBuffers(target);
        }
    }

    void prepare(GrMeshDrawTarget* target,
                 int maxTessellationSegments,
                 const SkMatrix& shaderMatrix,
                 const PathDrawList& pathDrawList,
                 int totalCombinedPathVerbCnt,
                 bool willUseTessellationShaders) final {
        if (int patchPreallocCount = PatchPreallocCount(totalCombinedPathVerbCnt)) {
            Writer writer = this->makeWriter(target, patchPreallocCount, maxTessellationSegments);
            this->writeAndPrepare(target, std::move(writer), shaderMatrix, pathDrawList,
                                  willUseTessellationShaders);
        }
    }

    void prepareFixedCountBuffers(GrMeshDrawTarget*) final;

    void drawTessellated(GrOpFlushState*) const final;
    void drawFixedCount(GrOpFlushState*) const final;

    // Draws a 4-point instance for each patch. This method is used for drawing convex hulls over
    // each cubic with GrFillCubicHullShader. The caller is responsible for binding its desired
    // pipeline ahead of time.
    void drawHullInstances(GrOpFlushState*, sk_sp<const GrGpuBuffer> vertexBufferIfNeeded) const;
#endif
};

}  // namespace skgpu

#endif  // tessellate_PathCurveTessellator_DEFINED
