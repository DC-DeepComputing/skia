/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkAssert.h"
#include "src/partition_alloc/raw_ptr.h"
#include "tests/Test.h"

#include <memory>
#include <tuple>

#if defined(SK_USE_PARTITION_ALLOC)
#include <partition_alloc/buildflags.h>
#if PA_BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS)
#include <partition_alloc/dangling_raw_ptr_checks.h>

namespace {
class ScopedDanglingPointerTracker {
public:
    ScopedDanglingPointerTracker()
            : fOldDetectedFn(partition_alloc::GetDanglingRawPtrDetectedFn())
            , fOldReleasedFn(partition_alloc::GetDanglingRawPtrReleasedFn()) {
        gDetected = false;
        gReleased = false;
        partition_alloc::SetDanglingRawPtrDetectedFn([](uintptr_t) { gDetected = true; });
        partition_alloc::SetDanglingRawPtrReleasedFn([](uintptr_t) { gReleased = true; });
    }

    ~ScopedDanglingPointerTracker() {
        partition_alloc::SetDanglingRawPtrDetectedFn(fOldDetectedFn);
        partition_alloc::SetDanglingRawPtrReleasedFn(fOldReleasedFn);
    }

    bool detected() const { return gDetected; }
    bool released() const { return gReleased; }

private:
    static inline bool gDetected = false;
    static inline bool gReleased = false;

    partition_alloc::DanglingRawPtrDetectedFn* fOldDetectedFn;
    partition_alloc::DanglingRawPtrReleasedFn* fOldReleasedFn;
};
}  // namespace

// This test temporarily affects global PartitionAlloc state by replacing
// detection and release handlers, so it cannot run in parallel.
DEF_SERIAL_TEST(RawPtr_DanglingPointerDetected, reporter) {
    ScopedDanglingPointerTracker tracker;

    std::unique_ptr<int> owner = std::make_unique<int>(42);
    raw_ptr<int> ptr = owner.get();
    REPORTER_ASSERT(reporter, !tracker.detected());
    REPORTER_ASSERT(reporter, !tracker.released());

    owner.reset();
    REPORTER_ASSERT(reporter, tracker.detected());
    REPORTER_ASSERT(reporter, !tracker.released());

    ptr = nullptr;
    REPORTER_ASSERT(reporter, tracker.detected());
    REPORTER_ASSERT(reporter, tracker.released());
}
#endif
#endif

DEF_TEST(RawPtr_DisableDanglingPtrDetection, reporter) {
    std::unique_ptr<int> owner = std::make_unique<int>(42);
    raw_ptr<int, DisableDanglingPtrDetection> ptr = owner.get();
    std::ignore = ptr;
    owner.reset();
}

DEF_TEST(RawPtr_PointerBehavior, reporter) {
    int arr[] = {10, 20, 30};
    int* expected = arr;
    raw_ptr<int, AllowPtrArithmetic> p = arr;

    // Initial state
    REPORTER_ASSERT(reporter, p == expected);
    REPORTER_ASSERT(reporter, *p == *expected);

    // Pre-increment
    ++expected;
    raw_ptr<int, AllowPtrArithmetic>& p_ref = ++p;
    REPORTER_ASSERT(reporter, &p_ref == &p);
    REPORTER_ASSERT(reporter, p == expected);

    // Post-increment
    int* expected_old = expected++;
    raw_ptr<int, AllowPtrArithmetic> p_old = p++;
    REPORTER_ASSERT(reporter, p_old == expected_old);
    REPORTER_ASSERT(reporter, p == expected);

    // Pre-decrement
    --expected;
    raw_ptr<int, AllowPtrArithmetic>& p_ref2 = --p;
    REPORTER_ASSERT(reporter, &p_ref2 == &p);
    REPORTER_ASSERT(reporter, p == expected);

    // Post-decrement
    expected_old = expected--;
    p_old = p--;
    REPORTER_ASSERT(reporter, p_old == expected_old);
    REPORTER_ASSERT(reporter, p == expected);

    // Addition
    REPORTER_ASSERT(reporter, (p + 1) == (expected + 1));
    REPORTER_ASSERT(reporter, (p + 2) == (expected + 2));

    // Subtraction
    raw_ptr<int, AllowPtrArithmetic> p2 = p + 2;
    int* expected2 = expected + 2;
    REPORTER_ASSERT(reporter, (p2 - 1) == (expected2 - 1));
    REPORTER_ASSERT(reporter, (p2 - 2) == (expected2 - 2));

    // Difference
    REPORTER_ASSERT(reporter, (p2 - p) == (expected2 - expected));

    // Member access
    struct Foo { int x; } foo{42};
    Foo* expectedFoo = &foo;
    raw_ptr<Foo> pFoo = &foo;
    REPORTER_ASSERT(reporter, pFoo->x == expectedFoo->x);
    REPORTER_ASSERT(reporter, pFoo.get() == expectedFoo);
}
