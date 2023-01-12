/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "tests/Test.h"

DEF_TEST(ColorToHSVRoundTrip, reporter) {
    SkScalar hsv[3];
    for (int r = 0; r <= 255; r++) {
        for (int g = 0; g <= 255; g++) {
            for (int b = 0; b <= 255; b++) {
                SkColor color = SkColorSetRGB(r, g, b);
                SkColorToHSV(color, hsv);
                SkColor result = SkHSVToColor(0xFF, hsv);
                if (result != color) {
                    ERRORF(reporter, "HSV roundtrip mismatch!\n"
                                     "\toriginal: %X\n"
                                     "\tHSV: %f, %f, %f\n"
                                     "\tresult: %X\n",
                           color, hsv[0], hsv[1], hsv[2], result);
                }
            }
        }
    }
}
