/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "tests/Test.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

DEF_TEST(SkSpanBasicTemplateGuide, reporter) {
    // Test constness preservation for SkSpan.
    {
        std::vector<int> v = {{1, 2, 3, 4, 5}};
        auto s = SkSpan(v);
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::vector<int> t = {{1, 2, 3, 4, 5}};
        const std::vector<int>& v = t;
        auto s = SkSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::array<int, 5> v = {{1, 2, 3, 4, 5}};
        auto s = SkSpan(v); // {1, 2, 3, 4, 5}
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100; // {1, 2, 3, 100, 5}
        REPORTER_ASSERT(reporter, s[3] == 100);
        auto s1 = s.subspan(1,3); // {2, 3, 100}
        REPORTER_ASSERT(reporter, s1.size() == 3);
        REPORTER_ASSERT(reporter, s1.front() == 2);
        REPORTER_ASSERT(reporter, s1.back() == 100);
        auto s2 = s.subspan(2); // {3, 100, 5}
        REPORTER_ASSERT(reporter, s2.size() == 3);
        REPORTER_ASSERT(reporter, s2.front() == 3);
        REPORTER_ASSERT(reporter, s2.back() == 5);
    }

    {
        std::array<int, 5> t = {{1, 2, 3, 4, 5}};
        const std::array<int, 5>& v = t;
        auto s = SkSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::vector<int> v;
        auto s = SkSpan(v);
        REPORTER_ASSERT(reporter, s.empty());
    }

    auto routine = [&](SkSpan<const int> a) {
        REPORTER_ASSERT(reporter, a.size() == 4);
    };
    routine({{1,2,3,4}});
}

static bool test_span_parameter(SkSpan<const int> s) {
    return s[0] == 1 && s[1] == 2 && s[2] == 3;
}

DEF_TEST(SkSpanDeduceParam, reporter) {
    {
        std::vector<int> v = {{1, 2, 3}};
        REPORTER_ASSERT(reporter, test_span_parameter(v));
        REPORTER_ASSERT(reporter, test_span_parameter(std::move(v)));
    }

    {
        const std::vector<int> v = {{1, 2, 3}};
        REPORTER_ASSERT(reporter, test_span_parameter(v));
        REPORTER_ASSERT(reporter, test_span_parameter(std::move(v)));
    }

    REPORTER_ASSERT(reporter, test_span_parameter(std::vector<int>{1,2,3}));

    {
        int v[]{1, 2, 3};
        REPORTER_ASSERT(reporter, test_span_parameter(v));
    }

    {
        test_span_parameter({{1, 2, 3}});
        REPORTER_ASSERT(reporter, test_span_parameter({{1, 2, 3}}));
    }

    {
        int v[]{1, 2, 3};
        auto s = SkSpan(v);
        REPORTER_ASSERT(reporter, test_span_parameter(s));
    }
}

DEF_TEST(SkSpanDeduceSize, reporter) {
    int d[] = {1, 2, 3, 4, 5};
    {
        size_t s = std::size(d);
        SkSpan span = SkSpan{d, s};
        REPORTER_ASSERT(reporter, span.size() == std::size(d));
    }
    {
        uint32_t s = std::size(d);
        SkSpan span = SkSpan{d, s};
        REPORTER_ASSERT(reporter, span.size() == std::size(d));
    }
    {
        size_t s = std::size(d);
        SkSpan span = SkSpan{d, s};
        REPORTER_ASSERT(reporter, span.size() == std::size(d));
    }
}

DEF_TEST(SkSpanFromRef, reporter) {
    int val = 42;
    SkSpan<int> s = SkSpanFromRef(val);
    REPORTER_ASSERT(reporter, s.size() == 1);
    REPORTER_ASSERT(reporter, s.data() == &val);
    REPORTER_ASSERT(reporter, s[0] == 42);
    s[0] = 99;
    REPORTER_ASSERT(reporter, val == 99);

    const int constVal = 123;
    SkSpan<const int> cs = SkSpanFromRef(constVal);
    REPORTER_ASSERT(reporter, cs.size() == 1);
    REPORTER_ASSERT(reporter, cs.data() == &constVal);
    REPORTER_ASSERT(reporter, cs[0] == 123);
}

DEF_TEST(SkAsByteSpan, reporter) {
    std::array<uint32_t, 2> arr = {0x01020304u, 0x05060708u};
    SkSpan<const uint8_t> bytes = SkAsByteSpan(arr);
    REPORTER_ASSERT(reporter, bytes.size() == sizeof(arr));
    REPORTER_ASSERT(reporter,
                    bytes.data() == reinterpret_cast<const uint8_t*>(arr.data()));

    SkSpan<uint8_t> writableBytes = SkAsWritableByteSpan(arr);
    REPORTER_ASSERT(reporter, writableBytes.size() == sizeof(arr));
    REPORTER_ASSERT(reporter,
                    writableBytes.data() == reinterpret_cast<uint8_t*>(arr.data()));
    writableBytes[0] = 0xFF;
    REPORTER_ASSERT(reporter, bytes[0] == 0xFF);

    uint32_t single = 0;
    SkSpan<uint8_t> singleBytes = SkAsWritableByteSpan(SkSpanFromRef(single));
    REPORTER_ASSERT(reporter, singleBytes.size() == sizeof(uint32_t));
    for (size_t i = 0; i < singleBytes.size(); ++i) {
        singleBytes[i] = 0x11;
    }
    REPORTER_ASSERT(reporter, single == 0x11111111u);
}

DEF_TEST(SkSpanificationHelpers, reporter) {
    std::array<uint32_t, 4> arr = {10, 20, 30, 40};
    static_assert(SkSpanificationSizeofForStdArray(arr) == 4 * sizeof(uint32_t));
    REPORTER_ASSERT(reporter, SkSpanificationSizeofForStdArray(arr) == sizeof(arr));

    SkSpan<uint32_t> s(arr);
    SkSpan<uint32_t> post = SkPostIncrementSpan(s);
    REPORTER_ASSERT(reporter, post.size() == 4);
    REPORTER_ASSERT(reporter, post[0] == 10);
    REPORTER_ASSERT(reporter, s.size() == 3);
    REPORTER_ASSERT(reporter, s[0] == 20);

    SkSpan<uint32_t> pre = SkPreIncrementSpan(s);
    REPORTER_ASSERT(reporter, pre.size() == 2);
    REPORTER_ASSERT(reporter, pre[0] == 30);
    REPORTER_ASSERT(reporter, s.size() == 2);
    REPORTER_ASSERT(reporter, s[0] == 30);
}

