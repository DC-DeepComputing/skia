/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_QuadTreeBoundsManager_DEFINED
#define skgpu_graphite_geom_QuadTreeBoundsManager_DEFINED

#include "include/core/SkRect.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/geom/BoundsManager.h"

namespace skgpu::graphite {

#define FLOAT_QUADTREE 0
#define UNROLL_BIT_QUADTREE 1

/**
 * QuadTreeBoundsManager is implemented using a fully-allocated quad tree that subdivides the
 * 2D bounds of a Device. Fully-allocated means that all data is present in a single allocation that
 * can be trivially reset, and insertions/updates do not trigger heap allocation as child nodes are
 * activated; queries are still able to take advantage of the hierarchy to skip checking deeper
 * nodes.
 *
 * The QuadTreeBoundsManager does not record the bounds of every recorded draw, but tracks the
 * highest CompressedPaintersOrder within each leaf node. As such, it's effective resolution is
 * defined by the fixed depth of the tree. Helper factories exist to create a tree that reaches a
 * desired leaf node resolution given the actual bounds of the owning Device.
 */
class QuadTreeBoundsManager final : public BoundsManager {
public:
    static std::unique_ptr<QuadTreeBoundsManager> MakeDepth(const SkISize& deviceBounds,
                                                            int depth) {
        SkASSERT(deviceBounds.width() > 0 && deviceBounds.height() > 0);
        int depthMinusOne = depth - 1;
        SkASSERT(0 <= depthMinusOne && depthMinusOne <= 16);
        size_t alloc = sizeof(QuadTreeBoundsManager) + sizeof(CompressedPaintersOrder) * num_nodes(depthMinusOne);
        void* quadTree = ::operator new(alloc);
        return std::unique_ptr<QuadTreeBoundsManager>(new (quadTree) QuadTreeBoundsManager(deviceBounds, depthMinusOne));
        // return std::unique_ptr<QuadTreeBoundsManager>(new QuadTreeBoundsManager(deviceBounds, depthMinusOne));
    }

    // static std::unique_ptr<QuadTreeBoundsManager> MakeResolution(const SkISize& deviceBounds,
    //                                                              int leafResolution);

    ~QuadTreeBoundsManager() override {
        // If this isn't true, then relying solely on ::operator delete won't work to clean up
        // the tree's nodes.
        static_assert(std::is_trivially_destructible<CompressedPaintersOrder>::value);
        // delete[] fNodes;
    }
    void operator delete(void* p) { ::operator delete(p); }


    CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const override {
#if FLOAT_QUADTREE
        Rect nodeBounds = Rect::WH(fWidth, fHeight);
        if (!nodeBounds.intersects(bounds)) {
            return CompressedPaintersOrder::First();
        }
        return this->get(fDepthMinusOne, 0, num_nodes(fDepthMinusOne - 1), nodeBounds, bounds);
#else
        Rect clampedBounds = bounds.makeIntersect(Rect::WH(fWidth, fHeight));
        if (clampedBounds.isEmptyNegativeOrNaN()) {
            return CompressedPaintersOrder::First();
        }
        auto [tl, br] = this->getRectCode(clampedBounds);
        return this->get(fDepthMinusOne, 0, num_nodes(fDepthMinusOne - 1), getRootEdges(clampedBounds), tl, br);
#endif
    }

    bool isOccluded(const Rect& bounds, PaintersDepth z) const override {
        // Not implemented, occlusion culling may not remain responsibility of BoundsManager
        return false;
    }

    void recordDraw(const Rect& bounds,
                    CompressedPaintersOrder order,
                    // ignored since they are for occlusion culling
                    PaintersDepth, bool) override {
#if FLOAT_QUADTREE
        Rect nodeBounds = Rect::WH(fWidth, fHeight);
        if (!nodeBounds.intersects(bounds)) {
            return;
        }
        // SkDebugf("record draw, d-1 = %d, num nodes = %d\n", fDepthMinusOne, num_nodes(fDepthMinusOne));
        this->set(fDepthMinusOne, 0, num_nodes(fDepthMinusOne - 1), nodeBounds, bounds, order);
#else
        Rect clampedBounds = bounds.makeIntersect(Rect::WH(fWidth, fHeight));
        if (clampedBounds.isEmptyNegativeOrNaN()) {
            return;
        }
        auto [tl, br] = this->getRectCode(clampedBounds);
        this->set(fDepthMinusOne, 0, num_nodes(fDepthMinusOne - 1), getRootEdges(clampedBounds), tl, br, order);
#endif
    }

    void reset() override {
        // It is sufficient to mark the root node as min = max = no-intersection; as new draws
        // are recorded and the root node or children are split, the (min == max) handling will
        // write the value to child nodes as needed.
        fNodes[0] = DrawOrder::kNoIntersection; // max for root
        fNodes[1] = DrawOrder::kNoIntersection; // min for root
    }

private:
    QuadTreeBoundsManager(const SkISize deviceBounds, int depthMinusOne)
            : fWidth(deviceBounds.width())
            , fHeight(deviceBounds.height())
            , fScaleX((1 << depthMinusOne) / fWidth)
            , fScaleY((1 << depthMinusOne) / fHeight)
            , fDepthMinusOne(depthMinusOne)
            , fNodes(reinterpret_cast<CompressedPaintersOrder*>(reinterpret_cast<uintptr_t>(this) + sizeof(QuadTreeBoundsManager))) {
        // fNodes = new CompressedPaintersOrder[num_nodes(depthMinusOne)];
        this->reset();
    }

    CompressedPaintersOrder getNodeMax(int nodeIndex) const {
        return fNodes[nodeIndex];
    }

    CompressedPaintersOrder getNodeMin(int height, int nodeIndex) const {
        return height ? fNodes[nodeIndex + 1] : fNodes[nodeIndex];
    }

#if FLOAT_QUADTREE
    CompressedPaintersOrder set(int height, int nodeIndex, int stride, const Rect& nodeBounds, const Rect& drawBounds, CompressedPaintersOrder drawOrder) {
        if (fNodes[nodeIndex] < drawOrder && (height == 0 || drawBounds.contains(nodeBounds))) {
            // The new order is greater than what's stored at or below this node and the draw covers
            // the entire node (hand-waving when at the leaf level).
            fNodes[nodeIndex] = drawOrder;
            if (height > 0) {
                // Not a leaf, so update the minimum to reflect that all child values should be
                // 'drawOrder' as well.
                fNodes[nodeIndex + 1] = drawOrder;
            }
            return drawOrder;
        } else if (height == 0 || fNodes[nodeIndex + 1] >= drawOrder) {
            // The new order is less than what's stored at or below this node, so nothing should be
            // updated and the current min remains unchanged.
            return this->getNodeMin(height, nodeIndex);
        }

        // Otherwise the draw order is above the current minimum and some of the child nodes may
        // be updated to raise their recorded value.

        const int childHeight = height - 1;
        const int childIndex  = nodeIndex + 2; // + i * stride, the +2 skips this node's min and max
        const int childStride = stride >> 2;

        // SkDebugf("h = %d, i = %d, s = %d\n", height, nodeIndex, stride);
        if (fNodes[nodeIndex] == fNodes[nodeIndex + 1]) {
            CompressedPaintersOrder childOrder = fNodes[nodeIndex];
            if (childHeight == 0) {
                SkASSERT(stride == 1);
                fNodes[nodeIndex + 1] = childOrder;
                fNodes[nodeIndex + 2] = childOrder;
                fNodes[nodeIndex + 3] = childOrder;
                fNodes[nodeIndex + 4] = childOrder;
            } else {
                for (int i = 0; i < 4; ++i) {
                    int j = childIndex + i * stride;
                    fNodes[j] = childOrder;
                    fNodes[j + 1] = childOrder;
                }
            }
        }

        skvx::float4 nodeCenter = nodeBounds.center().xyxy() * skvx::float4{1.f, 1.f, -1.f, -1.f};

        auto childrenIntersections = drawBounds.vals() <= nodeCenter; // r,b are negated already
        childrenIntersections = skvx::shuffle<0,2,0,2>(childrenIntersections) &
                                skvx::shuffle<1,1,3,3>(childrenIntersections);
        SkASSERT(any(childrenIntersections));



        const int kTrue = 0xffffffff;

        auto min = CompressedPaintersOrder::Last();
        if (childrenIntersections[0]) { // top-left
            min = std::min(min, this->set(childHeight, childIndex + 0*stride, childStride, Rect::FromVals(if_then_else({kTrue, kTrue, 0, 0}, nodeBounds.vals(), nodeCenter)), drawBounds, drawOrder));
        } else {
            min = std::min(min, this->getNodeMin(childHeight, childIndex + 0*stride));
        }
        if (childrenIntersections[1]) { // top-right
            min = std::min(min, this->set(childHeight, childIndex + 1*stride, childStride, Rect::FromVals(if_then_else({0, kTrue, kTrue, 0}, nodeBounds.vals(), nodeCenter)), drawBounds, drawOrder));
        } else {
            min = std::min(min, this->getNodeMin(childHeight, childIndex + 1*stride));
        }
        if (childrenIntersections[2]) { // bot-left
            min = std::min(min, this->set(childHeight, childIndex + 2*stride, childStride, Rect::FromVals(if_then_else({kTrue, 0, 0, kTrue}, nodeBounds.vals(), nodeCenter)), drawBounds, drawOrder));
        } else {
            min = std::min(min, this->getNodeMin(childHeight, childIndex + 2*stride));
        }
        if (childrenIntersections[3]) { // bot-right
            min = std::min(min, this->set(childHeight, childIndex + 3*stride, childStride, Rect::FromVals(if_then_else({0, 0, kTrue, kTrue}, nodeBounds.vals(), nodeCenter)), drawBounds, drawOrder));
        } else {
            min = std::min(min, this->getNodeMin(childHeight, childIndex + 3*stride));
        }

        // Even if some child node's maximums were raised to 'drawOrder', this node's max could
        // still be higher due to other unmodified children.
        fNodes[nodeIndex] = std::max(drawOrder, fNodes[nodeIndex]);
        fNodes[nodeIndex + 1] = min;
        return min;
    }

    CompressedPaintersOrder get(int height, int nodeIndex, int stride, const Rect& nodeBounds, const Rect& drawBounds) const {
        if (height == 0 || fNodes[nodeIndex] == fNodes[nodeIndex + 1] || drawBounds.contains(nodeBounds)) {
            return fNodes[nodeIndex];
        }

        skvx::float4 nodeCenter = nodeBounds.center().xyxy() * skvx::float4{1.f, 1.f, -1.f, -1.f};

        auto childrenIntersections = drawBounds.vals() <= nodeCenter; // r,b are negated already
        childrenIntersections = skvx::shuffle<0,2,0,2>(childrenIntersections) &
                                skvx::shuffle<1,1,3,3>(childrenIntersections);
        SkASSERT(any(childrenIntersections));

        const int childHeight = height - 1;
        const int childIndex  = nodeIndex + 2; // + i * stride, the +2 skips this node's min and max
        const int childStride = stride >> 2;

        const int kTrue = 0xffffffff;

        auto max = CompressedPaintersOrder::First();
        if (childrenIntersections[0]) { // top-left
            max = std::max(max, this->get(childHeight, childIndex + 0*stride, childStride, Rect::FromVals(if_then_else({kTrue, kTrue, 0, 0}, nodeBounds.vals(), nodeCenter)), drawBounds));
        } else {
            max = std::max(max, this->getNodeMax(childIndex + 0*stride));
        }
        if (childrenIntersections[1]) { // top-right
            max = std::max(max, this->get(childHeight, childIndex + 1*stride, childStride, Rect::FromVals(if_then_else({0, kTrue, kTrue, 0}, nodeBounds.vals(), nodeCenter)), drawBounds));
        } else {
            max = std::max(max, this->getNodeMax(childIndex + 1*stride));
        }
        if (childrenIntersections[2]) { // bot-left
            max = std::max(max, this->get(childHeight, childIndex + 2*stride, childStride, Rect::FromVals(if_then_else({kTrue, 0, 0, kTrue}, nodeBounds.vals(), nodeCenter)), drawBounds));
        } else {
            max = std::max(max, this->getNodeMax(childIndex + 2*stride));
        }
        if (childrenIntersections[3]) { // bot-right
            max = std::max(max, this->get(childHeight, childIndex + 3*stride, childStride, Rect::FromVals(if_then_else({0, 0, kTrue, kTrue}, nodeBounds.vals(), nodeCenter)), drawBounds));
        } else {
            max = std::max(max, this->getNodeMax(childIndex + 3*stride));
        }

        return max;
    }
#else
    static constexpr uint32_t kXBits = 0x55555555;  // odd bits are 0s, even are 1s
    static constexpr uint32_t kYBits = kXBits << 1; // odd bits are 1s, even are 0s

    static constexpr uint32_t kT = 0b1000;
    static constexpr uint32_t kL = 0b0100;
    static constexpr uint32_t kB = 0b0010;
    static constexpr uint32_t kR = 0b0001;

    std::pair<uint32_t, uint32_t> getRectCode(const Rect& bounds) const {
        // Normalize bounds by 1/wh of device bounds, then scale up to 2^(D-1)-1 and round out.
        // fScaleXY includes both 1/wh and the 2^(D-1)-1 scale factor. bounds should already have
        // been intersected with device bounds (and have a non-empty intersection).
        SkASSERT(!bounds.isEmptyNegativeOrNaN() && Rect::WH(fWidth, fHeight).contains(bounds));
        // These should fit into uint16_t, but we are immediately interleaving bits into two uint32s
        // so keep in the wider type with high bits being 0.
        SkASSERT(fDepthMinusOne <= 16 && (bounds.right() * fScaleX) <= (1 << fDepthMinusOne)
                                      && (bounds.bot() * fScaleY) <= (1 << fDepthMinusOne));
        skvx::uint4 leafCoords = min(
                skvx::cast<uint32_t>(bounds.ltrb() * skvx::float2(fScaleX, fScaleY).xyxy()),
                skvx::uint4((1 << fDepthMinusOne) - 1));

        // Move each (D-1)-bit coordinate to have its MSB at bit 16 in the 32-bit storage. After
        // interleaving the x and y coordinates, this ensures that the root node's Morton codes for
        // are in the top 2 bits, and shifting left by 2 bits places the next level's codes in the
        // exact same bit locations.
        leafCoords = leafCoords << (16 - fDepthMinusOne);
        // Interleave 8-bit blocks
        leafCoords = (leafCoords | (leafCoords << 8)) & skvx::uint4(0x00ff00ff);
        // Interleave 4-bit blocks
        leafCoords = (leafCoords | (leafCoords << 4)) & skvx::uint4(0x0f0f0f0f);
        // Interleave 2-bit blocks
        leafCoords = (leafCoords | (leafCoords << 2)) & skvx::uint4(0x33333333);
        // Interleave final bits
        leafCoords = (leafCoords | (leafCoords << 1)) & skvx::uint4(0x55555555);

        // Move Y bits (T and B) to odd positions and combine with X bits (L and R).
        skvx::uint2 tlbr = skvx::shuffle<0,2>(leafCoords) |
                          (skvx::shuffle<1,3>(leafCoords) << 1);
        return {~tlbr[0], tlbr[1]};
    }

    uint32_t getRootEdges(const Rect& bounds) const {
        SkASSERT(!bounds.isEmptyNegativeOrNaN() && Rect::WH(fWidth, fHeight).contains(bounds));
        auto nodeEdges = bounds.vals() == skvx::float4(0.f, 0.f, -fWidth, -fHeight);
        return (nodeEdges[0] ? kL : 0) |
               (nodeEdges[1] ? kT : 0) |
               (nodeEdges[2] ? kR : 0) |
               (nodeEdges[3] ? kB : 0);
    }

    CompressedPaintersOrder set(int height, int nodeIndex, int stride, uint32_t nodeEdges, uint32_t lt, uint32_t rb, CompressedPaintersOrder drawOrder) {
        if (fNodes[nodeIndex] < drawOrder && (height == 0 || nodeEdges == 0b1111)) {
            // The new order is greater than what's stored at or below this node and the draw covers
            // the entire node (hand-waving when at the leaf level).
            fNodes[nodeIndex] = drawOrder;
            if (height > 0) {
                // Not a leaf, so update the minimum to reflect that all child values should be
                // 'drawOrder' as well.
                fNodes[nodeIndex + 1] = drawOrder;
            }
            return drawOrder;
        } else if (height == 0 || fNodes[nodeIndex + 1] >= drawOrder) {
            // The new order is less than what's stored at or below this node, so nothing should be
            // updated and the current min remains unchanged.
            return this->getNodeMin(height, nodeIndex);
        }

        // Otherwise the draw order is above the current minimum and some of the child nodes may
        // be updated to raise their recorded value.

        // Assuming that 'lt' holds a bit-negated Morton code for the top left corner, and 'rb'
        // holds a regular Morton code for the bottom right corner, both shifted so that bit 31
        // holds the code's highest bit for the current depth in the quad tree, we can access the
        // top 2 bits of each corner's code with a fixed mask and shift. Combined, the 4 bits
        // identify which of the 4 half planes (top, left, bottom, or right) that the draw bounds
        // intersect with.
        const uint32_t intersectionCode = ((lt & 0xc0000000) >> 28) |
                                          ((rb & 0xc0000000) >> 30);

        const int childHeight = height - 1;
        const int childIndex  = nodeIndex + 2; // + i * stride, the +2 skips this node's min and max
        const int childStride = stride >> 2;

        if (fNodes[nodeIndex] == fNodes[nodeIndex + 1]) {
            CompressedPaintersOrder childOrder = fNodes[nodeIndex];
            if (childHeight == 0) {
                SkASSERT(stride == 1);
                for (int i = 0; i < 4; ++i) {
                    fNodes[childIndex + i] = childOrder;
                }
            } else {
                for (int i = 0; i < 4; ++i) {
                    int j = childIndex + i * stride;
                    fNodes[j] = childOrder;
                    fNodes[j + 1] = childOrder;
                }
            }
        }

#if UNROLL_BIT_QUADTREE
        // possibly new minimums for each child
        CompressedPaintersOrder tl, tr, bl, br;

        switch(intersectionCode) {
            // Single child intersections
            case kT | kL: {
                tl = this->set(childHeight, childIndex + 0*stride, childStride, nodeEdges, lt << 2, rb << 2, drawOrder);
                tr = this->getNodeMin(childHeight, childIndex + 1*stride);
                bl = this->getNodeMin(childHeight, childIndex + 2*stride);
                br = this->getNodeMin(childHeight, childIndex + 3*stride);
                break;
            }
            case kT | kR: {
                tl = this->getNodeMin(childHeight, childIndex + 0*stride);
                tr = this->set(childHeight, childIndex + 1*stride, childStride, nodeEdges, lt << 2, rb << 2, drawOrder);
                bl = this->getNodeMin(childHeight, childIndex + 2*stride);
                br = this->getNodeMin(childHeight, childIndex + 3*stride);
                break;
            }
            case kB | kL: {
                tl = this->getNodeMin(childHeight, childIndex + 0*stride);
                tr = this->getNodeMin(childHeight, childIndex + 1*stride);
                bl = this->set(childHeight, childIndex + 2*stride, childStride, nodeEdges, lt << 2, rb << 2, drawOrder);
                br = this->getNodeMin(childHeight, childIndex + 3*stride);
                break;
            }
            case kB | kR: {
                tl = this->getNodeMin(childHeight, childIndex + 0*stride);
                tr = this->getNodeMin(childHeight, childIndex + 1*stride);
                bl = this->getNodeMin(childHeight, childIndex + 2*stride);
                br = this->set(childHeight, childIndex + 3*stride, childStride, nodeEdges, lt << 2, rb << 2, drawOrder);
                break;
            }

            // Two child intersections (vertical or horizontal)
            case kT | kL | kR: { // top row
                tl = this->set(childHeight, childIndex + 0*stride, childStride, nodeEdges | kR, lt << 2, (rb | kXBits) << 2, drawOrder);
                tr = this->set(childHeight, childIndex + 1*stride, childStride, nodeEdges | kL, (lt | kXBits) << 2, rb << 2, drawOrder);
                bl = this->getNodeMin(childHeight, childIndex + 2*stride);
                br = this->getNodeMin(childHeight, childIndex + 3*stride);
                break;
            }
            case kB | kL | kR: { // bot col
                tl = this->getNodeMin(childHeight, childIndex + 0*stride);
                tr = this->getNodeMin(childHeight, childIndex + 1*stride);
                bl = this->set(childHeight, childIndex + 2*stride, childStride, nodeEdges | kR, lt << 2, (rb | kXBits) << 2, drawOrder);
                br = this->set(childHeight, childIndex + 3*stride, childStride, nodeEdges | kL, (lt | kXBits) << 2, rb << 2, drawOrder);
                break;
            }
            case kL | kT | kB: { // left col
                tl = this->set(childHeight, childIndex + 0*stride, childStride, nodeEdges | kB, lt << 2, (rb | kYBits) << 2, drawOrder);
                tr = this->getNodeMin(childHeight, childIndex + 1*stride);
                bl = this->set(childHeight, childIndex + 2*stride, childStride, nodeEdges | kT, (lt | kYBits) << 2, rb << 2, drawOrder);
                br = this->getNodeMin(childHeight, childIndex + 3*stride);
                break;
            }
            case kR | kT | kB: { // right col
                tl = this->getNodeMin(childHeight, childIndex + 0*stride);
                tr = this->set(childHeight, childIndex + 1*stride, childStride, nodeEdges | kB, lt << 2, (rb | kYBits) << 2, drawOrder);
                bl = this->getNodeMin(childHeight, childIndex + 2*stride);
                br = this->set(childHeight, childIndex + 3*stride, childStride, nodeEdges | kT, (lt | kYBits) << 2, rb << 2, drawOrder);
                break;
            }

            // Four children intersection
            case kT | kL | kB | kR: {
                tl = this->set(childHeight, childIndex + 0*stride, childStride, nodeEdges | kB | kR, lt << 2, (rb | kXBits | kYBits) << 2, drawOrder);
                tr = this->set(childHeight, childIndex + 1*stride, childStride, nodeEdges | kB | kL, (lt | kXBits) << 2, (rb | kYBits) << 2, drawOrder);
                bl = this->set(childHeight, childIndex + 2*stride, childStride, nodeEdges | kT | kR, (lt | kYBits) << 2, (rb | kXBits) << 2, drawOrder);
                br = this->set(childHeight, childIndex + 3*stride, childStride, nodeEdges | kT | kL, (lt | kXBits | kYBits) << 2, rb << 2, drawOrder);
                break;
            }

            default:
                // Shouldn't be possible if 'lt' and 'rb' were valid, sorted bounds coords
                SkUNREACHABLE;
        }
        auto min = std::min(std::min(tl, tr), std::min(bl, br));
#else
        auto min = CompressedPaintersOrder::Last();

        static constexpr uint32_t kQuadrantCodes[] = { kT | kL, kT | kR, kB | kL, kB | kR };
        for (int i = 0; i < (int) SK_ARRAY_COUNT(kQuadrantCodes); ++i) {
            if ((intersectionCode & kQuadrantCodes[i]) == kQuadrantCodes[i]) {
                // Bounds intersects the ith quadrant, which means that if the bounds intersect
                // either of the opposite quadrants, we can mark those edges as covered when we
                // recurse into this quadrant.
                const uint32_t oppositeQuadrants = intersectionCode & ~kQuadrantCodes[i];
                const uint32_t childEdges = nodeEdges | oppositeQuadrants;
                // Additionally, have to clamp the Morton code coordinates of the bounds corners to
                // the center axis of the parent node. Since the LT codes have been twiddled already
                // this is simply OR'ing in all 1s for x or y if the opposite quad is intersected.
                // NOTE: bool(oppositeQuadrants & axisMask) * kAxisBits is equivalent to
                //       (oppositeQuadrants & axisMask) ? kAxisBits : 0, but avoids a branch.
                const uint32_t childLT = (lt | (bool(oppositeQuadrants & kL) * kXBits)
                                             | (bool(oppositeQuadrants & kT) * kYBits)) << 2;
                const uint32_t childRB = (rb | (bool(oppositeQuadrants & kR) * kXBits)
                                             | (bool(oppositeQuadrants & kB) * kYBits)) << 2;
                min = std::min(min, this->set(childHeight, childIndex + i*stride, childStride, childEdges, childLT, childRB, drawOrder));
            } else {
                // Bounds does not touch the ith quadrant, so include its old minimum in this node's
                min = std::min(min, this->getNodeMin(childHeight, childIndex + i*stride));
            }
        }
#endif

        // Even if some child node's maximums were raised to 'drawOrder', this node's max could
        // still be higher due to other unmodified children.
        fNodes[nodeIndex] = std::max(drawOrder, fNodes[nodeIndex]);
        fNodes[nodeIndex + 1] = min;
        return min;
    }

CompressedPaintersOrder get(int height, int nodeIndex, int stride, uint32_t nodeEdges, uint32_t lt, uint32_t rb) const {
        if (height == 0 || nodeEdges == 0b1111 || fNodes[nodeIndex] == fNodes[nodeIndex + 1]) {
            return fNodes[nodeIndex];
        }

        // Assuming that 'lt' holds a bit-negated Morton code for the top left corner, and 'rb'
        // holds a regular Morton code for the bottom right corner, both shifted so that bit 31
        // holds the code's highest bit for the current depth in the quad tree, we can access the
        // top 2 bits of each corner's code with a fixed mask and shift. Combined, the 4 bits
        // identify which of the 4 half planes (top, left, bottom, or right) that the draw bounds
        // intersect with.
        const uint32_t intersectionCode = ((lt & 0xc0000000) >> 28) |
                                          ((rb & 0xc0000000) >> 30);

        const int childHeight = height - 1;
        const int childIndex  = nodeIndex + 2; // + i * stride, the +2 skips this node's min and max
        const int childStride = stride >> 2;

#if UNROLL_BIT_QUADTREE
        switch(intersectionCode) {
            // Single child intersections
            case kT | kL: {
                return this->get(childHeight, childIndex + 0*stride, childStride, nodeEdges, lt << 2, rb << 2);
            }
            case kT | kR: {
                return this->get(childHeight, childIndex + 1*stride, childStride, nodeEdges, lt << 2, rb << 2);
            }
            case kB | kL: {
                return this->get(childHeight, childIndex + 2*stride, childStride, nodeEdges, lt << 2, rb << 2);
            }
            case kB | kR: {
                return this->get(childHeight, childIndex + 3*stride, childStride, nodeEdges, lt << 2, rb << 2);
            }

            // Two child intersections (vertical or horizontal)
            case kT | kL | kR: { // top row
                return std::max(this->get(childHeight, childIndex + 0*stride, childStride, nodeEdges | kR, lt << 2, (rb | kXBits) << 2),
                                this->get(childHeight, childIndex + 1*stride, childStride, nodeEdges | kL, (lt | kXBits) << 2, rb << 2));
            }
            case kB | kL | kR: { // bot col
                return std::max(this->get(childHeight, childIndex + 2*stride, childStride, nodeEdges | kR, lt << 2, (rb | kXBits) << 2),
                                this->get(childHeight, childIndex + 3*stride, childStride, nodeEdges | kL, (lt | kXBits) << 2, rb << 2));
            }
            case kL | kT | kB: { // left col
                return std::max(this->get(childHeight, childIndex + 0*stride, childStride, nodeEdges | kB, lt << 2, (rb | kYBits) << 2),
                                this->get(childHeight, childIndex + 2*stride, childStride, nodeEdges | kT, (lt | kYBits) << 2, rb << 2));
            }
            case kR | kT | kB: { // right col
                return std::max(this->get(childHeight, childIndex + 1*stride, childStride, nodeEdges | kB, lt << 2, (rb | kYBits) << 2),
                                this->get(childHeight, childIndex + 3*stride, childStride, nodeEdges | kT, (lt | kYBits) << 2, rb << 2));
            }

            // Four children intersection
            case kT | kL | kB | kR: {
                return std::max(
                        std::max(this->get(childHeight, childIndex + 0*stride, childStride, nodeEdges | kB | kR, lt << 2, (rb | kXBits | kYBits) << 2),
                                 this->get(childHeight, childIndex + 1*stride, childStride, nodeEdges | kB | kL, (lt | kXBits) << 2, (rb | kYBits) << 2)),
                        std::max(this->get(childHeight, childIndex + 2*stride, childStride, nodeEdges | kT | kR, (lt | kYBits) << 2, (rb | kXBits) << 2),
                                 this->get(childHeight, childIndex + 3*stride, childStride, nodeEdges | kT | kL, (lt | kXBits | kYBits) << 2, rb << 2)));
            }

            default:
                // Shouldn't be possible if 'lt' and 'rb' were valid, sorted bounds coords
                SkUNREACHABLE;
        }
#else
        auto max = CompressedPaintersOrder::First();

        static constexpr uint32_t kQuadrantCodes[] = { kT | kL, kT | kR, kB | kL, kB | kR };
        for (int i = 0; i <  (int) SK_ARRAY_COUNT(kQuadrantCodes); ++i) {
            if ((intersectionCode & kQuadrantCodes[i]) == kQuadrantCodes[i]) {
                // Bounds intersects the ith quadrant, which means that if the bounds intersect
                // either of the opposite quadrants, we can mark those edges as covered when we
                // recurse into this quadrant.
                const uint32_t oppositeQuadrants = intersectionCode & ~kQuadrantCodes[i];
                const uint32_t childEdges = nodeEdges | oppositeQuadrants;
                // Additionally, have to clamp the Morton code coordinates of the bounds corners to
                // the center axis of the parent node. Since the LT codes have been twiddled already
                // this is simply OR'ing in all 1s for x or y if the opposite quad is intersected.
                // NOTE: bool(oppositeQuadrants & axisMask) * kAxisBits is equivalent to
                //       (oppositeQuadrants & axisMask) ? kAxisBits : 0, but avoids a branch.
                const uint32_t childLT = (lt | (bool(oppositeQuadrants & kL) * kXBits)
                                             | (bool(oppositeQuadrants & kT) * kYBits)) << 2;
                const uint32_t childRB = (rb | (bool(oppositeQuadrants & kR) * kXBits)
                                             | (bool(oppositeQuadrants & kB) * kYBits)) << 2;
                max = std::max(max, this->get(childHeight, childIndex + i*stride, childStride, childEdges, childLT, childRB));
            }
        }

        return max;
#endif
    }
#endif

    // If we start with int WH, do we need float bounds? Should we just store the precomputed
    // numbers that's needed to convert to quad tree coords? Is there a more efficient quad tree
    // coord we can use?
    // const SkRect fBounds; // Not skgpu::graphite::Rect to avoid over-aligning this class
    const float fWidth;
    const float fHeight;

    const float fScaleX;
    const float fScaleY;

    const int fDepthMinusOne;

    // Allocated immediately after this class, automatically deleted when the quad tree is deleted.
    //
    // The quad tree is stored in a flat array of CompressedPaintersOrders, arranged to match the
    // depth-first traversal of the quad tree. The tree is fully allocated to a fixed depth,
    // calculated from the Device bounds and target leaf-node resolution. Leaf nodes are represented
    // at a single index in `fNodes` and store the maximum recorded painter's order that intersects
    // their 2D bounds. Non-leaf nodes are represented by two consecutive indices, storing the
    // minimum and the maximum. When a non-leaf node's min and max are equal, it allows the depth
    // first traversal to terminate early.  Leaves are stored differently since its min order is
    // its max order, and avoiding that redundancy halves the memory size of the largest level.
    //                                                         D-2
    // For a tree of depth D, there are 4^(D-1) leaf nodes and  ∑ 4^i = (4^(D-1) - 1)/3 non-leaves.
    //                                                         i=0
    //
    // A leaf node uses one 16-bit CompressedPaintersOrder and the other nodes use two. The required
    // size of 'fNodes' is thus sizeof(uint16_t)*(4^(D-1) + 2*(4^(D-1) - 1)/3). Even when fully
    // allocated, this is not unreasonable for useful tree depths:
    //    Depth    Leaf Res (@ 4k)   Memory
    //      5        256 px/side        0.8 kb
    //      6        128   ""           3.3 kb
    //      7         64   ""          13.3 kb
    //      8         32   ""          54.0 kb
    //      9         16   ""         212.3 kb
    //      10         8   ""         853.3 kb
    //

    // TODO: Is this okay? For save layers having to allocate 3kb for a bounds manager seems like
    // it could add up. For small layers where we still target the same leaf resolution, we'd hit
    // lower memory counts. Maybe it is worth allocating in blocks so that we can jump to a new
    // allocation for a sub-tree, e.g. we fully allocate D=5, and then if we hit D=6, those are
    // lazily allocated?

    // Nodes are arranged to coincide with a depth-first traversal. This means that the stride
    // between siblings at a level height, h, has to account for all of the deeper children.
    // The stride between leaf node siblings (h=0) is S_0=1.                      h-1
    // The stride between non-leaf node siblings, is S_h = (2 + 4*S_h-1) = 4^h + 2 ∑ 4^i
    //                                                                            i=0
    //                                                                       D-2
    // If we start at the root node, fNodes[0], h=D-1 and S_D-1 = 4^(D-1) + 2 ∑ 4^i,
    //                                                                       i=0
    // which is exactly equal to the total size required for fNodes.
    //
    // Additionally, the stride for the next level can be derived by simply dividing by 4 and
    // rounding down, S_h-1 = floor(S_h / 4). Here is an example layout of fNodes for D=3:
    //
    //  fNodes[i] 0                   <-- i -->                     25
    //       h=2  [][]
    //       h=1      [][]        [][]        [][]        [][]
    //       h=0          [][][][]    [][][][]    [][][][]    [][][][]
    //            root TL TLTRBLBR TR TLTRBLBR BL TLTRBLBR BR TLTRBLBR
    //
    // Lastly, it's not necessary to keep track of the quadtree's node's bounds or use floating
    // point coordinates to test for intersection and containment. The depth-first arrangement of
    // nodes is equivalent to the Morton ordering of the leaf nodes, described by their X and Y
    // axis indices (from 0 to (2^(D-1)-1)). Based on the owning Device's dimensions and the tree
    // depth, the query bounds can be converted to this same Morton code once at the start of the
    // traversal.
    CompressedPaintersOrder* fNodes;

    static  int pow4(int i) {
        // SkDebugf("pow4 of %d, exponent = %d, 2^e = %d\n", i, i<<1, (1 << (i << 1)));
        return (1 << (i << 1)); // 4^i = (2^2)^i = 2^(2i)
    }

    static  int num_nodes(int depthMinusOne) {
        // SkDebugf("num nodes, d-1 = %d, leaves = %d, non-leaves = %d\n",
        //     depthMinusOne, pow4(depthMinusOne), 2*(pow4(depthMinusOne) - 1)/3);
        return /*leaves*/ pow4(depthMinusOne) + /*non-leaves*/ 2*(pow4(depthMinusOne) - 1) / 3;
    }
};


}  // namespace skgpu::graphite


#endif // skgpu_graphite_geom_QuadTreeBoundsManager_DEFINED
