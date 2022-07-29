/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/CtsEnforcement.h"

CtsEnforcement::RunMode CtsEnforcement::eval(int apiLevel) const {
    // always return false if both the strict and relaxed versions are undefined
    if (fStrictVersion == kNever && fStrictVersion == fWorkaroundsVersion) {
        return RunMode::kSkip;
    }

    // do a check for strict mode
    if (fStrictVersion != kNever && apiLevel >= fStrictVersion){
        return RunMode::kRunStrict;
    }

    // all strict mode checks failed so do a final check for relaxed mode
    return (apiLevel >= fWorkaroundsVersion) ? RunMode::kRunWithWorkarounds : RunMode::kSkip;
}
