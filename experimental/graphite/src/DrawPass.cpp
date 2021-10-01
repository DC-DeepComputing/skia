/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/DrawPass.h"

#include "experimental/graphite/include/GraphiteTypes.h"
#include "experimental/graphite/src/DrawList.h"
#include "experimental/graphite/src/SurfaceDrawContext.h"
#include "src/core/SkUtils.h"

namespace skgpu {

namespace {

enum class DrawType : unsigned {
    kStencilCurves = 0b00, // Primary stencil pass for large paths, or only stencil pass
    kStencilTris   = 0b01, // Optional secondary pass for large paths, just inner triangles
    kDepthOnly     = 0b10, // Same as kDirect but used when color buffer isn't modified
    kDirect        = 0b11, // Color primitives, convex or stroked paths, or post-stencil fill
};

// Each command in a DrawList can produce up to several actual "draw" operations that are
// dependent on the original command but can also be sorted independently. The goal of sorting
// the operations for the DrawPass is to minimize pipeline transitions and dynamic binds within
// a pipeline, while still respecting the overall painter's order. This reduces to a vertex
// coloring problem on the intersection graph formed by the commands and how their bounds
// overlap, followed by ordering by pipeline description and uniform data. General vertex
// coloring is NP-complete so DrawPass uses a greedy algorithm where the order it "colors" the
// vertices is based on the ordering constraints for the color+depth buffer and optionally the
// stencil buffer (stored in fColorDepthIndex and fStencilIndex respectively). skgpu::Device
// determines the ordering on-the-fly by using BoundsManager to approximate intersections as
// draw commands are recorded. It is possible to issue draws to Skia that produce pathologic
// orderings using this method, but it strikes a reasonable balance between finding a near
// optimal ordering that respects painter's order and is very efficient to compute.
//
// The color-depth index and stencil index represent the most significant bits of the key, and
// are shared by all SortKeys produced by the same command. Next, the pipeline description is
// encoded in two steps:
//  1. The logical type of draw (i.e. stencil, stencil inner triangles, depth-only, regular) is
//     packed in the high bits to ensure dependent draws are ordered correctly.
//  2. An index into a cache of pipeline descriptions is used to encode the identity of the
//     pipeline (sort keys that differ in the high bits from #1 necessarily would have different
//     description indices, but then ordering isn't enforced).
// Last, the command-specific uniform/sampling data is hashed to increase the probability that
// draws with the same pipeline and same data are adjacent. This data hash is split into data
// for the geometry (e.g. transform matrix and scissor) and for shading (e.g. color) so that
// a hierarchical uniform binding approach can be more easily implemented.
//
// The SortKey also stores an index pointing back to the command in the DrawList. To minimize
// the size of the struct, this index (and the pipeline description index) are not pointers,
// which means using SortKeys is only possible when the originating DrawList and DrawPass are
// available.
struct SortKey {
    SortKey(int commandIndex,
            BufferIndex colorDepthIndex,
            BufferIndex stencilIndex,
            DrawType drawType,
            int pipelineIndex,
            uint16_t geomDataHash,
            uint32_t shadingDataHash)
        : fPipelineKey{colorDepthIndex,
                        stencilIndex,
                        static_cast<uint64_t>(drawType),
                        static_cast<uint64_t>(pipelineIndex)}
        , fDataHash{geomDataHash,
                    static_cast<uint16_t>(commandIndex),
                    shadingDataHash} {}

    bool operator<(const SortKey& k) const {
        uint64_t k1 = this->pipelineKey();
        uint64_t k2 = k.pipelineKey();
        return k1 < k2 || (k1 == k2 && this->dataHash() < k.dataHash());
    }

    int commandIndex() const { return static_cast<int>(fDataHash.fCommandIndex); }
    int pipelineIndex() const { return static_cast<int>(fPipelineKey.fPipelineIndex); }

    DrawType drawType() const { return static_cast<DrawType>(fPipelineKey.fDrawType); }

    // Exposed for inspection, but generally the buffer indices aren't needed after sorting
    // since draws can be merged with different index values so long as they have the
    // same pipeline and their sorted ordering is preserved.
    BufferIndex colorDepthIndex() const {
        return static_cast<BufferIndex>(fPipelineKey.fColorDepthIndex);
    }
    BufferIndex stencilIndex() const {
        return static_cast<BufferIndex>(fPipelineKey.fStencilIndex);
    }

    // These are exposed to help optimize detecting when new uniforms need to be bound.
    // Differing hashes definitely represent different uniform bindings, but identical hashes
    // require a complete comparison.
    uint16_t geomDataHash() const { return static_cast<uint16_t>(fDataHash.fGeomDataHash); }
    uint32_t shadingDataHash() const {
        return static_cast<uint32_t>(fDataHash.fShadingDataHash);
    }

private:
    // Fields are ordered from most-significant to lowest when sorting by 128-bit value.
    struct {
        // The buffer indices are limited by the number of unique values we can store in the
        // depth buffer, and minimum supported depth buffer is 16-bits, so this packing is
        // sufficient.
        uint64_t fColorDepthIndex : 16;
        uint64_t fStencilIndex    : 16;
        uint64_t fDrawType        : 2;
        // The 16-bit limitation on command index (and buffer indices), combined with the max of
        // 4 dependent draws per command means that 30 bits for the pipeline index is more than
        // sufficient to represent the unique pipeline descriptions referenced in a DrawPass.
        uint64_t fPipelineIndex   : 30;
    } fPipelineKey; // NOTE: named for bit-punning, can't take address of a bit-field

    uint64_t pipelineKey() const { return sk_bit_cast<uint64_t>(fPipelineKey); }

    struct {
        // Presumably there is less variance in geometric uniform data, so a 16-bit hash is
        // hopefully sufficient (also why it has higher sort precedence than shading).
        uint64_t fGeomDataHash    : 16;
        // The command index does not impact comparison of SortKeys but is stored in the data
        // hash for better packing (must be masked off to compare).
        uint64_t fCommandIndex    : 16;
        // 32-bit hash could have collisions, but hopefully it's low enough given that
        // collisions only produce sub-optimal ordering when they have the same pipeline desc.
        uint64_t fShadingDataHash : 32;
    } fDataHash;

    uint64_t dataHash() const {
        static constexpr uint64_t kCommandIndexMask = 0xffff0000ffffffff;
        return sk_bit_cast<uint64_t>(fDataHash) & kCommandIndexMask;
    }
};
static_assert(sizeof(SortKey) == 16);

} // namespace

std::unique_ptr<DrawPass> DrawPass::Make(std::unique_ptr<DrawList> cmds, SurfaceDrawContext* sdc) {
    // TODO: DrawList processing will likely go here and then move the results into the DrawPass
    return std::unique_ptr<DrawPass>(new DrawPass());
}

} // namespace skgpu
