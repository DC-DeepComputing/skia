/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPathBuilder.h"

DEF_SIMPLE_GM(crbug_1257515, canvas, 1139, 400) {
    // <svg width="1139" height="400" viewBox="0 0 1139 400">
    //    <g transform="translate(46,60) scale(1 1)">
    //       <path fill="none" d="M 45.125 102.53701800000002
    //                            L 135.375 162.666156 L 225.625 116.622276
    //                            L 315.875 121.52087700000001 L 406.125 134.632899
    //                            L 496.375 192.317736 L 586.625 138.82944899999998
    //                            L 676.875 234.212031 L 767.125 207.082926 L 857.375 128.083857
    //                            L 947.625 127.95689999999999 L 1037.875 113.956785"
    //             stroke="red" stroke-width="2" stroke-linejoin="round" stroke-linecap="round">
    //       </path>
    //    </g>
    // </svg>
    SkPathBuilder b;
    b.moveTo(45.125, 102.53701800000002)
     .lineTo(135.375, 162.666156)
     .lineTo(225.625, 116.622276)
     .lineTo(315.875, 121.52087700000001)
     .lineTo(406.125, 134.632899)
     .lineTo(496.375, 192.317736)
     .lineTo(586.625, 138.82944899999998)
     .lineTo(676.875, 234.212031)
     .lineTo(767.125, 207.082926)
     .lineTo(857.375, 128.083857)
     .lineTo(947.625, 127.95689999999999)
     .lineTo(1037.875, 113.956785);

    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setStrokeWidth(2.f);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeCap(SkPaint::kRound_Cap);
    p.setStrokeJoin(SkPaint::kRound_Join);
    p.setAntiAlias(true);

    canvas->save();
    canvas->translate(-50.f, -200.f);
    canvas->scale(2.f, 2.f);
    canvas->drawPath(b.detach(), p);
    canvas->restore();

    // <svg width="1148" height="700" viewBox="0 0 1148 700">
    //    <path fill="none" d="M 129.5307 587.5728 L 232.4748 617.037 L 335.4189 624.8472
    //                         L 438.3631 630.5933 L 541.3073 625.1138 L 644.2513 626.8717
    //                         L 747.1955 629.9542 L 850.1396 629.6956 L 953.0838 616.4909
    //                         L 1056.028 613.8181"
    //          stroke="rgba(47,136,255,1)" stroke-width="3"
    //          stroke-linecap="butt" stroke-linejoin="bevel" stroke-miterlimit="10">
    //    </path>
    // </svg>
    b.moveTo(128.5307, 587.5728)
     .lineTo(232.4748, 617.037)
     .lineTo(335.4189, 624.8472)
     .lineTo(438.3631, 630.5933)
     .lineTo(541.3073, 625.1138)
     .lineTo(644.2513, 626.8717)
     .lineTo(747.1955, 629.9542)
     .lineTo(850.1396, 629.6956)
     .lineTo(953.0838, 616.4909)
     .lineTo(1056.028, 613.8181);
    p.setColor(SkColorSetARGB(255, 47, 136, 255));
    p.setStrokeWidth(3.f);
    p.setStrokeCap(SkPaint::kButt_Cap);
    p.setStrokeJoin(SkPaint::kBevel_Join);
    p.setStrokeMiter(10.f);

    canvas->save();
    canvas->translate(-300.f, -900.f);
    canvas->scale(2.f, 2.f);
    canvas->drawPath(b.detach(), p);
    canvas->restore();
}
