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

namespace {
template <typename T, size_t Extent = SkDynamicExtent>
struct MockBaseSpan {
    static constexpr size_t extent = Extent;
    constexpr T* data() const { return fPtr; }
    constexpr size_t size() const { return fSize; }
    T* fPtr = nullptr;
    size_t fSize = 0;
};
}  // namespace

DEF_TEST(SkSpanFixedExtent, reporter) {
    // Zero-overhead fixed-extent storage vs dynamic-extent storage.
    static_assert(sizeof(SkSpan<int, 5>) == sizeof(int*));
    static_assert(sizeof(SkSpan<int, 0>) == sizeof(int*));
    static_assert(sizeof(SkSpan<int>) == sizeof(int*) + sizeof(size_t));
    static_assert(SkSpan<int, 5>::extent == 5);
    static_assert(SkSpan<int>::extent == SkDynamicExtent);

    // Default constructibility: allowed for Extent == 0 and SkDynamicExtent, disallowed for N > 0.
    static_assert(std::is_default_constructible_v<SkSpan<int>>);
    static_assert(std::is_default_constructible_v<SkSpan<int, 0>>);
    static_assert(!std::is_default_constructible_v<SkSpan<int, 5>>);

    {
        constexpr SkSpan<int, 0> emptyFixed;
        static_assert(emptyFixed.empty());
        static_assert(emptyFixed.size() == 0);
        static_assert(emptyFixed.size_bytes() == 0);
        REPORTER_ASSERT(reporter, emptyFixed.data() == nullptr);
    }

    // Construction and CTAD from C-array and std::array.
    {
        int arr[4] = {10, 20, 30, 40};
        auto s = SkSpan(arr);
        static_assert(std::is_same_v<decltype(s), SkSpan<int, 4>>);
        static_assert(s.size() == 4);
        static_assert(s.size_bytes() == 4 * sizeof(int));
        REPORTER_ASSERT(reporter, s.front() == 10);
        REPORTER_ASSERT(reporter, s.back() == 40);
        REPORTER_ASSERT(reporter, s[2] == 30);

        // Implicit conversion from fixed-extent to dynamic-extent and const-qualified spans.
        SkSpan<int> dyn = s;
        SkSpan<const int, 4> constFixed = s;
        SkSpan<const int> constDyn = s;
        REPORTER_ASSERT(reporter, dyn.size() == 4 && dyn[1] == 20);
        REPORTER_ASSERT(reporter, constFixed.size() == 4 && constFixed[1] == 20);
        REPORTER_ASSERT(reporter, constDyn.size() == 4 && constDyn[1] == 20);

        // Explicit conversion from dynamic-extent to fixed-extent.
        SkSpan<int, 4> backToFixed(dyn);
        REPORTER_ASSERT(reporter, backToFixed.size() == 4 && backToFixed[3] == 40);
    }

    {
        std::array<int, 3> arr = {{7, 8, 9}};
        auto s = SkSpan(arr);
        static_assert(std::is_same_v<decltype(s), SkSpan<int, 3>>);
        REPORTER_ASSERT(reporter, s[0] == 7 && s[2] == 9);

        const std::array<int, 3>& constArr = arr;
        auto cs = SkSpan(constArr);
        static_assert(std::is_same_v<decltype(cs), SkSpan<const int, 3>>);
        REPORTER_ASSERT(reporter, cs[1] == 8);
    }

    // Compile-time conversion rules (matching base::span / std::span).
    static_assert(std::is_convertible_v<int(&)[4], SkSpan<int, 4>>);
    static_assert(std::is_convertible_v<int(&)[4], SkSpan<const int, 4>>);
    static_assert(!std::is_convertible_v<int(&)[3], SkSpan<int, 4>>);
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, int(&)[3]>);

    static_assert(std::is_convertible_v<std::array<int, 4>&, SkSpan<int, 4>>);
    static_assert(std::is_convertible_v<std::array<int, 4>&, SkSpan<const int, 4>>);
    static_assert(std::is_convertible_v<const std::array<int, 4>&, SkSpan<const int, 4>>);
    static_assert(!std::is_convertible_v<const std::array<int, 4>&, SkSpan<int, 4>>);
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, std::array<int, 3>&>);

    // Interop with external span types exposing `::extent` (like base::span and std::span).
    static_assert(std::is_convertible_v<MockBaseSpan<int, 4>, SkSpan<int, 4>>);
    static_assert(std::is_convertible_v<MockBaseSpan<int, 4>, SkSpan<const int, 4>>);
    static_assert(std::is_convertible_v<MockBaseSpan<int, 4>, SkSpan<int>>);
    static_assert(!std::is_constructible_v<SkSpan<int, 4>, MockBaseSpan<int, 3>>);
    static_assert(!std::is_convertible_v<MockBaseSpan<int>, SkSpan<int, 4>>);
    static_assert(std::is_constructible_v<SkSpan<int, 4>, MockBaseSpan<int>>);

    // Dynamic container to fixed-extent span is explicit-only.
    static_assert(std::is_convertible_v<std::vector<int>&, SkSpan<int>>);
    static_assert(!std::is_convertible_v<std::vector<int>&, SkSpan<int, 4>>);
    static_assert(std::is_constructible_v<SkSpan<int, 4>, std::vector<int>&>);

    // Compile-time subviews: first<Count>(), last<Count>(), subspan<Offset, Count>().
    {
        int data[5] = {1, 2, 3, 4, 5};
        SkSpan<int, 5> s(data);

        auto f2 = s.first<2>();
        static_assert(std::is_same_v<decltype(f2), SkSpan<int, 2>>);
        REPORTER_ASSERT(reporter, f2[0] == 1 && f2[1] == 2);

        auto l3 = s.last<3>();
        static_assert(std::is_same_v<decltype(l3), SkSpan<int, 3>>);
        REPORTER_ASSERT(reporter, l3[0] == 3 && l3[2] == 5);

        auto subFixed = s.subspan<1, 3>();
        static_assert(std::is_same_v<decltype(subFixed), SkSpan<int, 3>>);
        REPORTER_ASSERT(reporter, subFixed[0] == 2 && subFixed[2] == 4);

        auto subTail = s.subspan<2>();
        static_assert(std::is_same_v<decltype(subTail), SkSpan<int, 3>>);
        REPORTER_ASSERT(reporter, subTail[0] == 3 && subTail[2] == 5);

        // Compile-time subviews on a dynamic SkSpan<T>.
        SkSpan<int> dyn = s;
        auto dynFirst = dyn.first<2>();
        static_assert(std::is_same_v<decltype(dynFirst), SkSpan<int, 2>>);
        REPORTER_ASSERT(reporter, dynFirst[0] == 1 && dynFirst[1] == 2);

        auto dynLast = dyn.last<2>();
        static_assert(std::is_same_v<decltype(dynLast), SkSpan<int, 2>>);
        REPORTER_ASSERT(reporter, dynLast[0] == 4 && dynLast[1] == 5);

        auto dynSubFixed = dyn.subspan<1, 2>();
        static_assert(std::is_same_v<decltype(dynSubFixed), SkSpan<int, 2>>);
        REPORTER_ASSERT(reporter, dynSubFixed[0] == 2 && dynSubFixed[1] == 3);

        auto dynSubTail = dyn.subspan<2>();
        static_assert(std::is_same_v<decltype(dynSubTail), SkSpan<int>>);
        REPORTER_ASSERT(reporter, dynSubTail.size() == 3 && dynSubTail[0] == 3);
    }
}
