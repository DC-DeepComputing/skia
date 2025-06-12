// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SKIA_LOGO, 816, 464, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(4.0f, 4.0f);
    const SkColor background = SK_ColorWHITE;  // SK_ColorTRANSPARENT;
    const SkColor rgb[] = {0xFFE94037, 0xFF70BF4F, 0xFF465BA6};
    const SkColor lettering = 0xFF292929;
    const SkColor lineColors[2] = {0x30565656, 0xFF565656};
    SkPaint p;
    p.setAntiAlias(true);

    canvas->clear(background);
    canvas->scale(0.363f, 0.363f);

    p.setColor(rgb[1]);
    canvas->drawRect({326.0, 82.25, 343.9, 249.2}, p);
    p.setColor(rgb[0]);
    canvas->drawRect({310.2, 82.25, 327.0, 249.2}, p);
    p.setColor(rgb[2]);
    canvas->drawRect({342.9, 82.25, 358.87, 249.2}, p);

    p.setColor(lettering);
    canvas->drawCircle(335.355, 45.965, 29.25, p);

    SkPath s = SkPathBuilder()
        .moveTo(34.63, 100.63)
        .cubicTo(44.38, 88.57, 59.87, 82.86, 74.88, 81.2)
        .cubicTo(97.4, 78.5, 120.27, 83.25, 140.87, 92.37)
        .lineTo(127.12, 127.14)
        .cubicTo(113.55, 121.16, 99.04, 115.9, 83.98, 116.56)
        .cubicTo(78.86, 116.75, 72.88, 118.54, 70.71, 123.69)
        .cubicTo(68.62, 128.43, 71.52, 133.68, 75.58, 136.27)
        .cubicTo(91.49, 146.66, 110.67, 151.38, 125.46, 163.6)
        .cubicTo(132.35, 169.11, 137.33, 176.9, 139.36, 185.49)
        .cubicTo(142.55, 199.14, 140.94, 214.31, 133.13, 226.17)
        .cubicTo(126.23, 236.96, 114.82, 244.16, 102.75, 247.89)
        .cubicTo(87.95, 252.51, 72.16, 252.21, 56.88, 250.78)
        .cubicTo(45.54, 249.72, 34.64, 246.05, 24.32, 241.36)
        .lineTo(24.25, 201.1)
        .cubicTo(38.23, 208.15, 53.37, 213.15, 68.98, 214.75)
        .cubicTo(75.42, 215.25, 82.17, 215.63, 88.31, 213.27)
        .cubicTo(92.84, 211.53, 96.4, 206.93, 95.86, 201.93)
        .cubicTo(95.64, 196.77, 91.1, 193.38, 87.03, 190.99)
        .cubicTo(71.96, 182.67, 54.94, 177.66, 41.5, 166.57)
        .cubicTo(33.19, 159.73, 27.51, 149.8, 26.1, 139.11)
        .cubicTo(24.09, 125.88, 25.91, 111.25, 34.63, 100.63)
        .detach();
    canvas->drawPath(s, p);

    SkPath k = SkPathBuilder()
        .moveTo(160.82, 82.85)
        .lineTo(206.05, 82.85)
        .lineTo(206.05, 155.15)
        .lineTo(254.83, 82.84)
        .lineTo(304.01, 82.85)
        .lineTo(251.52, 157.27)
        .lineTo(303.09, 249.42)
        .lineTo(252.28, 249.4)
        .lineTo(219.18, 185.75)
        .lineTo(206.23, 193.45)
        .lineTo(206.05, 249.42)
        .lineTo(160.82, 249.42)
        .lineTo(160.82, 82.85)
        .detach();
    canvas->drawPath(k, p);

    SkPath a = SkPathBuilder()
        .moveTo(426.45, 218.16)
        .lineTo(480.705, 218.16)
        .lineTo(489.31, 249.4)
        .lineTo(538.54, 249.42)
        .lineTo(483.56, 82.18)
        .lineTo(423.43, 82.17)
        .lineTo(369.13, 249.42)
        .lineTo(418.5, 249.47)
        .lineTo(453.75, 109.83)
        .lineTo(471.77, 181.28)
        .lineTo(430.5, 181.28)
        .detach();
    canvas->drawPath(a, p);

    SkPath triangle = SkPathBuilder()
        .moveTo(362.64, 257.32)
        .lineTo(335.292, 293.392)
        .lineTo(307.8, 257.48)
        .lineTo(362.64, 257.32)
        .detach();
    p.setColor(lettering);
    canvas->drawPath(triangle, p);

    // line
    const SkPoint pts[2] = {{160, 290}, {341, 290}};
    p.setShader(SkGradientShader::MakeLinear(pts, lineColors, nullptr, 2, SkTileMode::kClamp));
    if (true) {
        SkRRect rrect;
        rrect.setRectXY({138, 291, 341, 300}, 25.0, 5.0);
        canvas->drawRRect(rrect, p);
    } else {
        SkPath path = SkPath::RRect({138, 291, 341, 299.95}, 25.0, 5.0);
        canvas->drawPath(path, p);
    }
}
}  // END FIDDLE
