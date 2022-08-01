/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef CtsEnforcement_DEFINED
#define CtsEnforcement_DEFINED

#include <cstdint>
#include "include/core/SkTypes.h"

class CtsEnforcement {
public:
    enum ApiLevel : int32_t { kNever = INT32_MAX, kApiLevel_T = 33, kNextRelease = 1000 };

    constexpr CtsEnforcement(ApiLevel strictVersion)
            : fStrictVersion(strictVersion), fWorkaroundsVersion(kNever) {}

    constexpr CtsEnforcement& withWorkarounds(ApiLevel workaroundVersion) {
        SkASSERT(workaroundVersion <= fStrictVersion);
        fWorkaroundsVersion = workaroundVersion;
        return *this;
    }

    enum class RunMode { kSkip = 0, kRunWithWorkarounds = 1, kRunStrict = 2 };
    RunMode eval(int apiLevel) const;

private:
    ApiLevel fStrictVersion;
    ApiLevel fWorkaroundsVersion;
};

#endif
