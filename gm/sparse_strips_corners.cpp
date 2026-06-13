/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/sparse_strips_corners.h"

namespace skgpu::graphite {

namespace {

// Case 1: Corner (Left to Right)
std::vector<VariationDef> makeInteriorLtrVariations() {
    std::vector<VariationDef> vars;
    // V1: Midpoint Corner
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 0.5f);
        pb.lineTo(2.5f, 3.5f);
        pb.lineTo(4.0f, 0.5f);
        vars.push_back({"Variation 1: Midpoint Corner (x=2.5, y=3.5)", pb.detach()});
    }
    // V2: Corner at y = n.5 (X aligned)
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 0.5f);
        pb.lineTo(2.0f, 3.5f);
        pb.lineTo(4.0f, 0.5f);
        vars.push_back({"Variation 2: Corner at y=n.5 (x=2.0, y=3.5)", pb.detach()});
    }
    // V3: Corner at x = n.5 (Y aligned)
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 0.5f);
        pb.lineTo(2.5f, 3.0f);
        pb.lineTo(4.0f, 0.5f);
        vars.push_back({"Variation 3: Corner at x=n.5 (x=2.5, y=3.0)", pb.detach()});
    }
    // V4: Perfect Top-Left
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 0.5f);
        pb.lineTo(2.0f, 3.0f);
        pb.lineTo(4.0f, 0.5f);
        vars.push_back({"Variation 4: Perfect Top-Left Corner (x=2.0, y=3.0)", pb.detach()});
    }
    return vars;
}

// Case 3: Sibling Bottom Caret
std::vector<VariationDef> makeInteriorBotVariations() {
    std::vector<VariationDef> vars;
    // V1: Midpoint Corner
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 0.0f);
        pb.lineTo(2.5f, 3.5f);
        pb.lineTo(4.0f, 0.0f);
        vars.push_back({"Variation 1: Midpoint Corner (x=2.5, y=3.5)", pb.detach()});
    }
    // V2: Corner at y = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 0.0f);
        pb.lineTo(2.0f, 3.5f);
        pb.lineTo(4.0f, 0.0f);
        vars.push_back({"Variation 2: Corner at y=n.5 (x=2.0, y=3.5)", pb.detach()});
    }
    // V3: Corner at x = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 0.0f);
        pb.lineTo(2.5f, 3.0f);
        pb.lineTo(4.0f, 0.0f);
        vars.push_back({"Variation 3: Corner at x=n.5 (x=2.5, y=3.0)", pb.detach()});
    }
    // V4: Perfect Top-Left
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 0.0f);
        pb.lineTo(2.0f, 3.0f);
        pb.lineTo(4.0f, 0.0f);
        vars.push_back({"Variation 4: Perfect Top-Left Corner (x=2.0, y=3.0)", pb.detach()});
    }
    return vars;
}

// Case 4: Sibling Top Caret
std::vector<VariationDef> makeInteriorTopVariations() {
    std::vector<VariationDef> vars;
    // V1: Midpoint Corner
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 4.0f);
        pb.lineTo(2.5f, 0.5f);
        pb.lineTo(4.0f, 4.0f);
        vars.push_back({"Variation 1: Midpoint Corner (x=2.5, y=0.5)", pb.detach()});
    }
    // V2: Corner at y = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 4.0f);
        pb.lineTo(2.0f, 0.5f);
        pb.lineTo(4.0f, 4.0f);
        vars.push_back({"Variation 2: Corner at y=n.5 (x=2.0, y=0.5)", pb.detach()});
    }
    // V3: Corner at x = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 4.0f);
        pb.lineTo(2.5f, 1.0f);
        pb.lineTo(4.0f, 4.0f);
        vars.push_back({"Variation 3: Corner at x=n.5 (x=2.5, y=1.0)", pb.detach()});
    }
    // V4: Perfect Top-Left
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 4.0f);
        pb.lineTo(2.0f, 1.0f);
        pb.lineTo(4.0f, 4.0f);
        vars.push_back({"Variation 4: Perfect Top-Left Corner (x=2.0, y=1.0)", pb.detach()});
    }
    return vars;
}

// Case 5: Reverse Winding (Up then Down)
std::vector<VariationDef> makeInteriorRevVariations() {
    std::vector<VariationDef> vars;
    // V1: Midpoint Corner
    {
        SkPathBuilder pb;
        pb.moveTo(4.0f, 0.5f);
        pb.lineTo(2.5f, 3.5f);
        pb.lineTo(0.0f, 0.5f);
        vars.push_back({"Variation 1: Midpoint Corner (x=2.5, y=3.5)", pb.detach()});
    }
    // V2: Corner at y = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(4.0f, 0.5f);
        pb.lineTo(2.0f, 3.5f);
        pb.lineTo(0.0f, 0.5f);
        vars.push_back({"Variation 2: Corner at y=n.5 (x=2.0, y=3.5)", pb.detach()});
    }
    // V3: Corner at x = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(4.0f, 0.5f);
        pb.lineTo(2.5f, 3.0f);
        pb.lineTo(0.0f, 0.5f);
        vars.push_back({"Variation 3: Corner at x=n.5 (x=2.5, y=3.0)", pb.detach()});
    }
    // V4: Perfect Top-Left
    {
        SkPathBuilder pb;
        pb.moveTo(4.0f, 0.5f);
        pb.lineTo(2.0f, 3.0f);
        pb.lineTo(0.0f, 0.5f);
        vars.push_back({"Variation 4: Perfect Top-Left Corner (x=2.0, y=3.0)", pb.detach()});
    }
    return vars;
}

// L-shaped Joint (Left-Bottom)
std::vector<VariationDef> makeInteriorLLeftBottomVariations() {
    std::vector<VariationDef> vars;
    // V1: Midpoint Corner
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 2.5f);
        pb.lineTo(2.5f, 2.5f);
        pb.lineTo(2.5f, 4.0f);
        vars.push_back({"Variation 1: Midpoint Corner (x=2.5, y=2.5)", pb.detach()});
    }
    // V2: Corner at y = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 2.5f);
        pb.lineTo(2.0f, 2.5f);
        pb.lineTo(2.0f, 4.0f);
        vars.push_back({"Variation 2: Corner at y=n.5 (x=2.0, y=2.5)", pb.detach()});
    }
    // V3: Corner at x = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 2.0f);
        pb.lineTo(2.5f, 2.0f);
        pb.lineTo(2.5f, 4.0f);
        vars.push_back({"Variation 3: Corner at x=n.5 (x=2.5, y=2.0)", pb.detach()});
    }
    // V4: Perfect Top-Left
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 2.0f);
        pb.lineTo(2.0f, 2.0f);
        pb.lineTo(2.0f, 4.0f);
        vars.push_back({"Variation 4: Perfect Top-Left Corner (x=2.0, y=2.0)", pb.detach()});
    }
    return vars;
}

// L-shaped Joint (Left-Top)
std::vector<VariationDef> makeInteriorLLeftTopVariations() {
    std::vector<VariationDef> vars;
    // V1: Midpoint Corner
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 2.5f);
        pb.lineTo(2.5f, 2.5f);
        pb.lineTo(2.5f, 0.0f);
        vars.push_back({"Variation 1: Midpoint Corner (x=2.5, y=2.5)", pb.detach()});
    }
    // V2: Corner at y = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 2.5f);
        pb.lineTo(2.0f, 2.5f);
        pb.lineTo(2.0f, 0.0f);
        vars.push_back({"Variation 2: Corner at y=n.5 (x=2.0, y=2.5)", pb.detach()});
    }
    // V3: Corner at x = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 2.0f);
        pb.lineTo(2.5f, 2.0f);
        pb.lineTo(2.5f, 0.0f);
        vars.push_back({"Variation 3: Corner at x=n.5 (x=2.5, y=2.0)", pb.detach()});
    }
    // V4: Perfect Top-Left
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 2.0f);
        pb.lineTo(2.0f, 2.0f);
        pb.lineTo(2.0f, 0.0f);
        vars.push_back({"Variation 4: Perfect Top-Left Corner (x=2.0, y=2.0)", pb.detach()});
    }
    return vars;
}

// Left Edge Corner (Down-Up)
std::vector<VariationDef> makeLeftEdgeDuVariations() {
    std::vector<VariationDef> vars;
    // V1: Y aligned
    {
        SkPathBuilder pb;
        pb.moveTo(2.0f, 0.0f);
        pb.lineTo(0.0f, 2.0f);
        pb.lineTo(2.0f, 4.0f);
        vars.push_back({"Variation 1: Y aligned (x=0.0, y=2.0)", pb.detach()});
    }
    // V2: Y = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(2.0f, 0.0f);
        pb.lineTo(0.0f, 2.5f);
        pb.lineTo(2.0f, 4.0f);
        vars.push_back({"Variation 2: Y = n.5 (x=0.0, y=2.5)", pb.detach()});
    }
    return vars;
}

// Left Edge Corner (Up-Down, Reverse)
std::vector<VariationDef> makeLeftEdgeUdVariations() {
    std::vector<VariationDef> vars;
    // V1: Y aligned
    {
        SkPathBuilder pb;
        pb.moveTo(2.0f, 4.0f);
        pb.lineTo(0.0f, 2.0f);
        pb.lineTo(2.0f, 0.0f);
        vars.push_back({"Variation 1: Y aligned (x=0.0, y=2.0)", pb.detach()});
    }
    // V2: Y = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(2.0f, 4.0f);
        pb.lineTo(0.0f, 2.5f);
        pb.lineTo(2.0f, 0.0f);
        vars.push_back({"Variation 2: Y = n.5 (x=0.0, y=2.5)", pb.detach()});
    }
    return vars;
}

// Left Edge Crossing
std::vector<VariationDef> makeLeftEdgeCrossVariations() {
    std::vector<VariationDef> vars;
    // V1: Y aligned
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 0.0f);
        pb.lineTo(2.0f, 2.0f);
        pb.lineTo(0.0f, 4.0f);
        vars.push_back({"Variation 1: Y aligned", pb.detach()});
    }
    // V2: Y = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(0.0f, 0.5f);
        pb.lineTo(2.5f, 2.0f);
        pb.lineTo(0.0f, 3.5f);
        vars.push_back({"Variation 2: Y = n.5", pb.detach()});
    }
    return vars;
}

// Left Edge Sibling Touch
std::vector<VariationDef> makeLeftEdgeSiblingTouchVariations() {
    std::vector<VariationDef> vars;
    // V1: Y aligned
    {
        SkPathBuilder pb;
        pb.moveTo(-2.0f, 4.0f);
        pb.lineTo(0.0f, 2.0f);
        pb.lineTo(2.0f, 4.0f);
        vars.push_back({"Variation 1: Y aligned (x=0.0, y=2.0)", pb.detach()});
    }
    // V2: Y = n.5
    {
        SkPathBuilder pb;
        pb.moveTo(-2.0f, 4.0f);
        pb.lineTo(0.0f, 2.5f);
        pb.lineTo(2.0f, 4.0f);
        vars.push_back({"Variation 2: Y = n.5 (x=0.0, y=2.5)", pb.detach()});
    }
    // V3: Top-Left Touch
    {
        SkPathBuilder pb;
        pb.moveTo(-2.0f, 2.0f);
        pb.lineTo(0.0f, 0.0f);
        pb.lineTo(2.0f, 4.0f);
        vars.push_back({"Variation 3: Top-Left Touch (x=0.0, y=0.0)", pb.detach()});
    }
    // V4: Grazing Touch from Left
    {
        SkPathBuilder pb;
        pb.moveTo(-2.0f, 4.0f);
        pb.lineTo(0.0f, 2.5f);
        pb.lineTo(-2.0f, 1.0f);
        vars.push_back({"Variation 4: Grazing Touch from Left (x=0.0, y=2.5)", pb.detach()});
    }
    return vars;
}

} // namespace

DEF_GM(return new SparseStripsCornerGMBase("sparse_strips_corner_interior_ltr", "Interior Corner: Left to Right", makeInteriorLtrVariations());)
DEF_GM(return new SparseStripsCornerGMBase("sparse_strips_corner_interior_bot", "Interior Corner: Bottom Caret (\\ /)", makeInteriorBotVariations());)
DEF_GM(return new SparseStripsCornerGMBase("sparse_strips_corner_interior_top", "Interior Corner: Top Caret (/ \\)", makeInteriorTopVariations());)
DEF_GM(return new SparseStripsCornerGMBase("sparse_strips_corner_interior_rev", "Interior Corner: Reverse Winding (Up-Down)", makeInteriorRevVariations());)
DEF_GM(return new SparseStripsCornerGMBase("sparse_strips_corner_interior_l_left_bottom", "Interior L-Joint: Left-Bottom", makeInteriorLLeftBottomVariations());)
DEF_GM(return new SparseStripsCornerGMBase("sparse_strips_corner_interior_l_left_top", "Interior L-Joint: Left-Top", makeInteriorLLeftTopVariations());)
DEF_GM(return new SparseStripsCornerGMBase("sparse_strips_corner_left_edge_du", "Left Edge Corner: Down-Up", makeLeftEdgeDuVariations());)
DEF_GM(return new SparseStripsCornerGMBase("sparse_strips_corner_left_edge_ud", "Left Edge Corner: Up-Down (Reverse)", makeLeftEdgeUdVariations());)
DEF_GM(return new SparseStripsCornerGMBase("sparse_strips_corner_left_edge_cross", "Left Edge: Crossing", makeLeftEdgeCrossVariations());)
DEF_GM(return new SparseStripsCornerGMBase("sparse_strips_corner_left_edge_sibling_touch", "Left Edge: Sibling Touch", makeLeftEdgeSiblingTouchVariations(), false);)

} // namespace skgpu::graphite
