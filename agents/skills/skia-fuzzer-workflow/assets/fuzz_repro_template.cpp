/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "src/core/SkFloatBits.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

// ==============================================================================
// Pattern 1: Geometry / Path Precision Bug (using exact float bits)
// ==============================================================================
// Use this pattern when reproducing crashes or assertion failures in geometry,
// clipping, stroking, or triangulation where denormalized/extreme floats trigger
// numerical instability.
//
// Tip: Use SkBits2Float(0x...) directly instead of decimal literals to prevent
// compiler rounding from altering the exact bit pattern that caused the crash.
DEF_TEST(Fuzz_Geometry_BugNumber, reporter) {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1.0f);

    SkPathBuilder builder;
    builder.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));
    builder.cubicTo(SkBits2Float(0x43434343), SkBits2Float(0x43434341),
                    SkBits2Float(0xb74343bd), SkBits2Float(0x01434343),
                    SkBits2Float(0x10434343), SkBits2Float(0x00144332));
    builder.close();

    SkPath path = builder.detach();

    // Contextual assertion reporting: provides diagnostic context if assertion fails
    {
        skiatest::ReporterContext ctx(reporter, SkString("Drawing fuzzed path to canvas"));
        canvas->clipRect(SkRect::MakeWH(100, 100));
        canvas->drawPath(path, paint);
    }
}

// ==============================================================================
// Pattern 2: Deserialization / Memory Buffer Bug
// ==============================================================================
// Use this pattern when the bug occurs while reading serialized buffers or memory
// packets (such as SkRRect, SkPath, SkColorSpace, or SkReadBuffer).
DEF_TEST(Fuzz_Deserialize_BugNumber, reporter) {
    const unsigned char raw_fuzz_bytes[] = {
        0x0a, 0x00, 0x00, 0xff, 0x00, 0x30, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
    };

    skiatest::ReporterContext ctx(reporter, SkString("Testing deserialization robustness"));
    SkRRect rrect;
    // Verify that invalid/corrupt buffer does not crash or assert
    size_t readBytes = rrect.readFromMemory(raw_fuzz_bytes, sizeof(raw_fuzz_bytes));
    (void)readBytes;
}

// ==============================================================================
// Pattern 3: GPU Backend Specific Bug (Ganesh / Graphite)
// ==============================================================================
#if defined(SK_GANESH)
DEF_GANESH_TEST(Fuzz_Ganesh_BugNumber, reporter, options, CtsEnforcement::kNever) {
    // Ganesh GPU reproduction logic here
    REPORTER_ASSERT(reporter, true);
}
#endif

#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(
        Fuzz_Graphite_BugNumber, reporter, context, CtsEnforcement::kNever) {
    // Graphite GPU reproduction logic here
    REPORTER_ASSERT(reporter, context);
}
#endif
