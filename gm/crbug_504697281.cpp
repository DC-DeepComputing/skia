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
#include "include/core/SkPathBuilder.h"
#include "src/base/SkFloatBits.h"

DEF_SIMPLE_GM(crbug_504697281, canvas, 700, 260) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(22);
    paint.setStrokeJoin(SkPaint::kMiter_Join);
    paint.setStrokeMiter(4.0f);
    paint.setColor(SK_ColorGREEN);

    SkPathBuilder builder;

    {
        builder.reset();
        builder.setFillType(SkPathFillType::kWinding);
        builder.moveTo(SkBits2Float(0x41980000), SkBits2Float(0x42300000));  // 19, 44
        builder.lineTo(SkBits2Float(0x41980000), SkBits2Float(0x42300000));  // 19, 44
        builder.conicTo(SkBits2Float(0x41980000), SkBits2Float(0x421c0000), SkBits2Float(0x41c00000), SkBits2Float(0x421c0000), SkBits2Float(0x3f3504f3));  // 19, 39, 24, 39, 0.707106769f
        builder.lineTo(SkBits2Float(0x43423a90), SkBits2Float(0x421c0000));  // 194.22876f, 39
        builder.lineTo(SkBits2Float(0x43423a90), SkBits2Float(0x421c0000));  // 194.22876f, 39
        builder.conicTo(SkBits2Float(0x43450000), SkBits2Float(0x421c0000), SkBits2Float(0x43467800), SkBits2Float(0x42256669), SkBits2Float(0x3f5fe88d));  // 197, 39, 198.46875f, 41.3500099f, 0.874642193f
        builder.lineTo(SkBits2Float(0x434ba53b), SkBits2Float(0x424687e4));  // 203.645432f, 49.6327057f
        builder.lineTo(SkBits2Float(0x434ba53b), SkBits2Float(0x424687e3));  // 203.645432f, 49.6327019f
        builder.conicTo(SkBits2Float(0x434de666), SkBits2Float(0x4254f5c2), SkBits2Float(0x434ab354), SkBits2Float(0x42602c68), SkBits2Float(0x3f42fb3f));  // 205.899994f, 53.2399979f, 202.7005f, 56.0433655f, 0.761646211f
        builder.lineTo(SkBits2Float(0x41da5c47), SkBits2Float(0x4351bb61));  // 27.2950573f, 209.731949f
        builder.lineTo(SkBits2Float(0x41da5c48), SkBits2Float(0x4351bb61));  // 27.2950592f, 209.731949f
        builder.conicTo(SkBits2Float(0x41980000), SkBits2Float(0x43590000), SkBits2Float(0x41980000), SkBits2Float(0x434df8a6), SkBits2Float(0x3ed368dd));  // 19, 217, 19, 205.971283f, 0.412909418f
        builder.close();

        canvas->drawPath(builder.detach(), paint);
    }

    {
        builder.reset();
        builder.setFillType(SkPathFillType::kWinding);
        builder.moveTo(SkBits2Float(0x439219bb), SkBits2Float(0x429b9915));  // 292.201019f, 77.7989883f
        builder.lineTo(SkBits2Float(0x439219bb), SkBits2Float(0x429b9917));  // 292.201019f, 77.7990036f
        builder.conicTo(SkBits2Float(0x439c0000), SkBits2Float(0x42680000), SkBits2Float(0x43a5e645), SkBits2Float(0x429b9917), SkBits2Float(0x3f3504f1));  // 312, 58, 331.798981f, 77.7990036f, 0.70710665f
        builder.lineTo(SkBits2Float(0x43b519bb), SkBits2Float(0x42d866eb));  // 362.201019f, 108.201012f
        builder.lineTo(SkBits2Float(0x43b519bb), SkBits2Float(0x42d866ec));  // 362.201019f, 108.201019f
        builder.conicTo(SkBits2Float(0x43bf0000), SkBits2Float(0x43000000), SkBits2Float(0x43b519bb), SkBits2Float(0x4313cc8a), SkBits2Float(0x3f3504f7));  // 382, 128, 362.201019f, 147.798981f, 0.707107008f
        builder.lineTo(SkBits2Float(0x43a5e645), SkBits2Float(0x43323375));  // 331.798981f, 178.201004f
        builder.lineTo(SkBits2Float(0x43a5e646), SkBits2Float(0x43323374));  // 331.799011f, 178.200989f
        builder.conicTo(SkBits2Float(0x439c0000), SkBits2Float(0x43460000), SkBits2Float(0x439219ba), SkBits2Float(0x43323374), SkBits2Float(0x3f3504ef));  // 312, 198, 292.200989f, 178.200989f, 0.707106531f
        builder.lineTo(SkBits2Float(0x4382e645), SkBits2Float(0x4313cc8b));  // 261.798981f, 147.798996f
        builder.lineTo(SkBits2Float(0x4382e645), SkBits2Float(0x4313cc8a));  // 261.798981f, 147.798981f
        builder.conicTo(SkBits2Float(0x43720000), SkBits2Float(0x43000000), SkBits2Float(0x4382e645), SkBits2Float(0x42d866ec), SkBits2Float(0x3f3504f7));  // 242, 128, 261.798981f, 108.201019f, 0.707107008f
        builder.close();

        canvas->drawPath(builder.detach(), paint);
    }

    {
        builder.reset();
        builder.setFillType(SkPathFillType::kWinding);
        builder.moveTo(SkBits2Float(0x43f764b6), SkBits2Float(0x42946d29));  // 494.786804f, 74.2132034f
        builder.lineTo(SkBits2Float(0x43f764b6), SkBits2Float(0x42946d2b));  // 494.786804f, 74.2132187f
        builder.conicTo(SkBits2Float(0x44010000), SkBits2Float(0x42540000), SkBits2Float(0x44064da5), SkBits2Float(0x42946d2a), SkBits2Float(0x3f3504f1));  // 516, 53, 537.213196f, 74.2132111f, 0.70710665f
        builder.lineTo(SkBits2Float(0x440e725b), SkBits2Float(0x42d592d7));  // 569.786804f, 106.786797f
        builder.lineTo(SkBits2Float(0x440e725b), SkBits2Float(0x42d592d7));  // 569.786804f, 106.786797f
        builder.conicTo(SkBits2Float(0x4413c000), SkBits2Float(0x43000000), SkBits2Float(0x440e725b), SkBits2Float(0x43153694), SkBits2Float(0x3f3504f5));  // 591, 128, 569.786804f, 149.213196f, 0.707106888f
        builder.lineTo(SkBits2Float(0x44064da5), SkBits2Float(0x4335c96c));  // 537.213196f, 181.786804f
        builder.lineTo(SkBits2Float(0x44064da5), SkBits2Float(0x4335c96b));  // 537.213196f, 181.786789f
        builder.conicTo(SkBits2Float(0x44010000), SkBits2Float(0x434b0000), SkBits2Float(0x43f764b6), SkBits2Float(0x4335c96c), SkBits2Float(0x3f3504f3));  // 516, 203, 494.786804f, 181.786804f, 0.707106769f
        builder.lineTo(SkBits2Float(0x43e71b4a), SkBits2Float(0x43153694));  // 462.213196f, 149.213196f
        builder.lineTo(SkBits2Float(0x43e71b4a), SkBits2Float(0x43153694));  // 462.213196f, 149.213196f
        builder.conicTo(SkBits2Float(0x43dc8000), SkBits2Float(0x43000000), SkBits2Float(0x43e71b4a), SkBits2Float(0x42d592d7), SkBits2Float(0x3f3504f5));  // 441, 128, 462.213196f, 106.786797f, 0.707106888f
        builder.close();

        canvas->drawPath(builder.detach(), paint);
    }
}
