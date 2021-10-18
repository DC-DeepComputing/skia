/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_geom_IntersectionTree_DEFINED
#define skgpu_geom_IntersectionTree_DEFINED

#include "experimental/graphite/src/geom/Rect.h"

namespace skgpu {

// Maintains a collection of non-overlapping rectangles.
//
// add() either adds the given rect to the collection, or returns false if it intersected with a
// rect already in the collection.
class IntersectionTree {
public:
    IntersectionTree();

    ~IntersectionTree() { delete fRoot; }

    bool add(Rect rect) {
        if (!fRoot->intersects(rect)) {
            fRoot = fRoot->addNonIntersecting(rect);
            return true;
        }
        return false;
    }

private:
    class Node {
    public:
        virtual ~Node() = default;

        virtual bool intersects(Rect) = 0;
        virtual Node* addNonIntersecting(Rect) = 0;
    };

    enum class SplitType : bool {
        kX,
        kY
    };

    template<SplitType kSplitType> class TreeNode;
    class LeafNode;

    Node* fRoot;
};


} // namespace skgpu

#endif // skgpu_geom_IntersectionTree_DEFINED
