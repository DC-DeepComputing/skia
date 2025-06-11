/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathEffect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkPathEffectBase.h"
#include "tests/Test.h"

using namespace skia_private;

DEF_TEST(AsADashTest_noneDash, reporter) {
    sk_sp<SkPathEffect> pe(SkCornerPathEffect::Make(1.0));
    SkPathEffectBase::DashInfo info;

    auto dashInfo = as_PEB(pe)->asADash();
    REPORTER_ASSERT(reporter, !dashInfo.has_value());
}

DEF_TEST(AsADashTest_nullInfo, reporter) {
    const SkScalar inIntervals[] = { 4.0, 2.0, 1.0, 3.0 };
    const SkScalar phase = 2.0;
    sk_sp<SkPathEffect> pe(SkDashPathEffect::Make(inIntervals, phase));

    auto dashInfo = as_PEB(pe)->asADash();
    REPORTER_ASSERT(reporter, dashInfo.has_value());
}

DEF_TEST(AsADashTest_usingDash, reporter) {
    const SkScalar inIntervals[] = { 4.0, 2.0, 1.0, 3.0 };
    SkScalar totalIntSum = 10.0;
    const SkScalar phase = 2.0;

    sk_sp<SkPathEffect> pe(SkDashPathEffect::Make(inIntervals, phase));

    auto info = as_PEB(pe)->asADash();
    REPORTER_ASSERT(reporter, info.has_value());
    REPORTER_ASSERT(reporter, 4 == info->fVals.size());
    REPORTER_ASSERT(reporter, SkScalarMod(phase, totalIntSum) == info->fPhase);

    REPORTER_ASSERT(reporter, inIntervals[0] == info->fVals[0]);
    REPORTER_ASSERT(reporter, inIntervals[1] == info->fVals[1]);
    REPORTER_ASSERT(reporter, inIntervals[2] == info->fVals[2]);
    REPORTER_ASSERT(reporter, inIntervals[3] == info->fVals[3]);
}
