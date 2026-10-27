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
            "M65.73110499999999,361.3333333333333"
            "C65.73110499999999,438.16666666666663 47.2603755,438.16666666666663 47.2603755,515"
            "h9.942864"
            "C57.2032395,438.16666666666663 75.67396899999999,438.16666666666663 75.67396899999999,361.3333333333333"
            "Z",
            true
        },
        {
            "M235.754637,204.66666666666666"
            "C235.754637,281.5 47.773745500000004,281.5 47.773745500000004,358.3333333333333"
            "h9.2910995"
            "C57.064845000000005,281.5 245.0457365,281.5 245.0457365,204.66666666666666"
            "Z",
            true
        },
        {
            "M162.03592483333333,204.66666666666666"
            "C162.03592483333333,281.5 282.2070646666666,281.5 282.2070646666666,358.3333333333333"
            "h8.021137"
            "C290.2282016666666,281.5 170.05706183333334,281.5 170.05706183333334,204.66666666666666"
            "Z",
            false
        },
        {
            "M254.11764816666664,361.3333333333333"
            "C254.11764816666664,438.16666666666663 217.04605166666667,438.16666666666663 217.04605166666667,515"
            "h7.8179430000000005"
            "C224.86399466666668,438.16666666666663 261.93559116666665,438.16666666666663 261.93559116666665,361.3333333333333"
            "Z",
            true
        },
        {
            "M220.74949566666666,204.66666666666666"
            "C220.74949566666666,281.5 156.61913500000003,281.5 156.61913500000003,358.3333333333333"
            "h7.647777"
            "C164.26691200000002,281.5 228.39727266666665,281.5 228.39727266666665,204.66666666666666"
            "Z",
            false
        },
        {
            "M232.78407316666664,361.3333333333333"
            "C232.78407316666664,438.16666666666663 257.2506595,438.16666666666663 257.2506595,515"
            "h7.529306999999999"
            "C264.7799665,438.16666666666663 240.31338016666663,438.16666666666663 240.31338016666663,361.3333333333333"
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
