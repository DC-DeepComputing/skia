// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Int96.h"
#include "tests/Test.h"

#include <cstdint>

using namespace bentleyottmann;

DEF_TEST(BO_Int96Basic, reporter) {
    {
        int32_t t = 0;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == 0 && z.lo == 0);
    }
    {
        int64_t t = 0;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == 0 && z.lo == 0);
    }
    {
        int32_t t = -1;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == -1 && z.lo == 0xFFFFFFFF);
    }
    {
        int64_t t = -1;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == -1 && z.lo == 0xFFFFFFFF);
    }
    {
        int32_t t = 3;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == 0 && z.lo == 3);
    }
    {
        int64_t t = 3;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == 0 && z.lo == 3);
    }
    {
        int32_t t = -3;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == -1 && z.lo == (uint32_t)-3);
    }
    {
        int64_t t = -3;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == -1 && z.lo == (uint32_t)-3);
    }

    {
        int64_t t = 1ll << 32;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == 1 && z.lo == 0);
    }
    {
        // -2 << 32 -- without the warnings.
        int64_t t = -(2ll << 32);
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == -2 && z.lo == 0);
    }
}

DEF_TEST(BO_Int96Less, reporter) {
    {
        REPORTER_ASSERT(reporter, Int96::Make(0) < Int96::Make(1));
        REPORTER_ASSERT(reporter, !(Int96::Make(1) < Int96::Make(0)));
        REPORTER_ASSERT(reporter, Int96::Make(-1) < Int96::Make(0));
        REPORTER_ASSERT(reporter, !(Int96::Make(0) < Int96::Make(-1)));
    }
    {
        REPORTER_ASSERT(reporter, Int96::Make(1ll << 31) < Int96::Make(1ll << 32));
        REPORTER_ASSERT(reporter, !(Int96::Make(1ll << 32) < Int96::Make(1ll << 31)));
        REPORTER_ASSERT(reporter, Int96::Make(-(1ll << 32)) < Int96::Make(-(1LL << 31)));
        REPORTER_ASSERT(reporter, !(Int96::Make(-(1LL << 31)) < Int96::Make(-(1ll << 32))));
    }
}

DEF_TEST(BO_Int96Add, reporter) {
    {
        REPORTER_ASSERT(reporter, Int96::Make(0) + Int96::Make(0) == Int96::Make(0));
        REPORTER_ASSERT(reporter, Int96::Make(1) + Int96::Make(0) == Int96::Make(1));
        REPORTER_ASSERT(reporter, Int96::Make(-1) + Int96::Make(0) == Int96::Make(-1));
        REPORTER_ASSERT(reporter, Int96::Make(-1) + Int96::Make(-1) == Int96::Make(-2));
        REPORTER_ASSERT(reporter, Int96::Make(-2) + Int96::Make(1) == Int96::Make(-1));
        REPORTER_ASSERT(reporter, Int96::Make(2) + Int96::Make(-1) == Int96::Make(1));
    }
}

DEF_TEST(BO_Int96Mult, reporter) {
    REPORTER_ASSERT(reporter, multiply(0ll, 0) == Int96::Make(0));
    REPORTER_ASSERT(reporter, multiply(1ll, 1) == Int96::Make(1));
    REPORTER_ASSERT(reporter, multiply(-1ll, 1) == Int96::Make(-1));
    REPORTER_ASSERT(reporter, multiply(-1ll, -1) == Int96::Make(1));
    REPORTER_ASSERT(reporter, multiply(-2ll, -2) == Int96::Make(4));
    REPORTER_ASSERT(reporter, multiply(2ll, -2) == Int96::Make(-4));
    REPORTER_ASSERT(reporter, multiply(-2ll, 2) == Int96::Make(-4));
    REPORTER_ASSERT(reporter, multiply(2ll, 2) == Int96::Make(4));
}
