/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/utils/SkParsePath.h"

// This GM replicates the SVG path elements in the HTML file provided in b/576311140.
// Six semi-transparent filled paths, each bounded by two cubic Beziers and two horizontal lines.
DEF_SIMPLE_GM(cubicfilled, canvas, 379, 520) {
    SkPaint blue;
    blue.setColor(SkColorSetRGB(74, 95, 193));
    blue.setAlphaf(0.6);
    blue.setAntiAlias(true);

    SkPaint orange;
    orange.setColor(SkColorSetRGB(224, 154, 47));
    orange.setAlphaf(0.6);
    orange.setAntiAlias(true);

    const struct {
        const char* svgPath;
        bool isBlue;
    } svgPaths[] = {
        {
            "M65.731105,361.333333"
            "C65.731105,438.166667 47.2603755,438.166667 47.2603755,515"
            "h9.942864"
            "C57.2032395,438.166667 75.673969,438.166667 75.673969,361.333333"
            "Z",
            true
        },
        {
            "M235.754637,204.666667"
            "C235.754637,281.5 47.7737455,281.5 47.7737455,358.333333"
            "h9.2910995"
            "C57.064845,281.5 245.0457365,281.5 245.0457365,204.666667"
            "Z",
            true
        },
        {
            "M162.035925,204.666667"
            "C162.035925,281.5 282.207065,281.5 282.207065,358.333333"
            "h8.021137"
            "C290.228202,281.5 170.057062,281.5 170.057062,204.666667"
            "Z",
            false
        },
        {
            "M254.117648,361.333333"
            "C254.117648,438.166667 217.046052,438.166667 217.046052,515"
            "h7.817943"
            "C224.863995,438.166667 261.935591,438.166667 261.935591,361.333333"
            "Z",
            true
        },
        {
            "M220.749496,204.666667"
            "C220.749496,281.5 156.619135,281.5 156.619135,358.333333"
            "h7.647777"
            "C164.266912,281.5 228.397273,281.5 228.397273,204.666667"
            "Z",
            false
        },
        {
            "M232.784073,361.333333"
            "C232.784073,438.166667 257.2506595,438.166667 257.2506595,515"
            "h7.529307"
            "C264.7799665,438.166667 240.313380,438.166667 240.313380,361.333333"
            "Z",
            true
        },
    };

    canvas->clear(SK_ColorWHITE);
    for (const auto& item : svgPaths) {
        if (auto path = SkParsePath::FromSVGString(item.svgPath)) {
            canvas->drawPath(path.value(), item.isBlue ? blue : orange);
        }
    }
}
