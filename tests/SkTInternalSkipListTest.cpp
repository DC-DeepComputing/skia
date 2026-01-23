/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "src/base/SkRandom.h"
#include "src/base/SkTInternalSkipList.h"
#include "tests/Test.h"

#include <vector>
#include <algorithm>
#include <vector>

// -----------------------------------------------------------------------------
// Test Object Definition
// -----------------------------------------------------------------------------

// We define a predefined MaxLevel for testing
static const int kMaxHeight = 8;

struct TestNode {
    int fValue;
    int fID; // Unique ID to track specific instances even if values are equal

    // The required interface macro
    SK_DECLARE_INTERNAL_SKIPLIST_INTERFACE(TestNode, kMaxHeight);

    // Constructor for easier testing
    TestNode(int v, int id) : fValue(v), fID(id) {}

    // Comparator required by the SkipList
    // Note: The list sorts based on this.
    bool operator<(const TestNode& other) const {
        if (fValue != other.fValue) {
            return fValue < other.fValue;
        }
        // If values are equal, sort by ID to ensure deterministic ordering
        // for the test validator (optional, but helps strict testing)
        return fID < other.fID;
    }
};

using TestList = SkTInternalSkipList<TestNode, kMaxHeight>;

// -----------------------------------------------------------------------------
// Helper: Validator
// -----------------------------------------------------------------------------

static void validate_list(skiatest::Reporter* r, const TestList& list, const std::vector<TestNode*>& expected) {
    // 1. Validate Forward Iteration
    int count = 0;
    TestList::Iter iter;
    TestNode* curr = iter.init(list, TestList::Iter::kHead_IterStart);

    for (TestNode* exp : expected) {
        REPORTER_ASSERT(r, curr != nullptr);
        if (curr) {
            REPORTER_ASSERT(r, curr == exp);
            REPORTER_ASSERT(r, curr->fValue == exp->fValue);
            REPORTER_ASSERT(r, list.contains(curr));
        }
        curr = iter.next();
        count++;
    }
    REPORTER_ASSERT(r, curr == nullptr); // Should be at end
    REPORTER_ASSERT(r, list.isEmpty() == expected.empty());

    // 2. Validate Backward Iteration
    curr = iter.init(list, TestList::Iter::kTail_IterStart);
    for (auto it = expected.rbegin(); it != expected.rend(); ++it) {
        TestNode* exp = *it;
        REPORTER_ASSERT(r, curr != nullptr);
        if (curr) {
            REPORTER_ASSERT(r, curr == exp);
        }
        curr = iter.prev();
    }
    REPORTER_ASSERT(r, curr == nullptr); // Should be at start

    // 3. Validate Internal Integrity (Debug mode usually does this, but we do it explicitly)
    SkDEBUGCODE(list.validate();)
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

DEF_TEST(SkInternalSkipList_BasicLifecycle, r) {
    TestList list;

    // Simulate an "Arena" allocation on the stack
    TestNode n1(10, 1);
    TestNode n2(5, 2);
    TestNode n3(20, 3);

    REPORTER_ASSERT(r, list.isEmpty());
    REPORTER_ASSERT(r, list.head() == nullptr);
    REPORTER_ASSERT(r, list.tail() == nullptr);

    // Insert Middle
    list.insert(&n1);
    REPORTER_ASSERT(r, !list.isEmpty());
    REPORTER_ASSERT(r, list.head() == &n1);
    REPORTER_ASSERT(r, list.tail() == &n1);

    // Insert Head (Smaller)
    list.insert(&n2);
    REPORTER_ASSERT(r, list.head() == &n2);
    REPORTER_ASSERT(r, list.tail() == &n1);

    // Insert Tail (Larger)
    list.insert(&n3);
    REPORTER_ASSERT(r, list.head() == &n2);
    REPORTER_ASSERT(r, list.tail() == &n3);

    // Order should be 5 -> 10 -> 20
    TestList::Iter iter;
    TestNode* curr = iter.init(list, TestList::Iter::kHead_IterStart);
    REPORTER_ASSERT(r, curr->fValue == 5);
    curr = iter.next();
    REPORTER_ASSERT(r, curr->fValue == 10);
    curr = iter.next();
    REPORTER_ASSERT(r, curr->fValue == 20);
    REPORTER_ASSERT(r, iter.next() == nullptr);

    // Remove Middle
    list.remove(&n1);
    REPORTER_ASSERT(r, !list.contains(&n1));
    REPORTER_ASSERT(r, list.contains(&n2));
    REPORTER_ASSERT(r, list.contains(&n3));

    // Check links
    REPORTER_ASSERT(r, list.head() == &n2);
    REPORTER_ASSERT(r, list.tail() == &n3);

    // Remove Head
    list.remove(&n2);
    REPORTER_ASSERT(r, list.head() == &n3);
    REPORTER_ASSERT(r, list.tail() == &n3);

    // Remove Tail
    list.remove(&n3);
    REPORTER_ASSERT(r, list.isEmpty());
    REPORTER_ASSERT(r, list.head() == nullptr);
}

DEF_TEST(SkInternalSkipList_Duplicates, r) {
    // Tests that the list handles equivalent values correctly
    // (stable insertion or just valid linkage)
    TestList list;

    std::vector<TestNode> arena;
    arena.reserve(10);
    // Create 3 nodes with value 10
    arena.emplace_back(10, 1);
    arena.emplace_back(10, 2);
    arena.emplace_back(10, 3);

    list.insert(&arena[0]);
    list.insert(&arena[1]);
    list.insert(&arena[2]);

    // Check iteration count
    int count = 0;
    TestList::Iter iter;
    TestNode* curr = iter.init(list, TestList::Iter::kHead_IterStart);
    while(curr) {
        REPORTER_ASSERT(r, curr->fValue == 10);
        curr = iter.next();
        count++;
    }
    REPORTER_ASSERT(r, count == 3);

    // Remove the middle one (by ID)
    // The implementation relies on operator<, but also pointer checks during remove
    list.remove(&arena[1]);

    count = 0;
    curr = iter.init(list, TestList::Iter::kHead_IterStart);
    while(curr) {
        // We expect ID 1 and 3 to remain
        REPORTER_ASSERT(r, curr->fID == 1 || curr->fID == 3);
        curr = iter.next();
        count++;
    }
    REPORTER_ASSERT(r, count == 2);

    list.remove(&arena[0]);
    list.remove(&arena[2]);
    REPORTER_ASSERT(r, list.isEmpty());
}

DEF_TEST(SkInternalSkipList_RandomFuzz, r) {
    // A robust test that randomizes operations and verifies against a truth source
    SkRandom rand;
    TestList list;

    // The "Arena" holds the actual memory
    // Using pointers to avoid vector resizing invalidating references
    std::vector<TestNode*> arena;

    // The "Truth" - what we expect the list to contain in sorted order
    std::vector<TestNode*> expected;

    const int kNumOperations = 2000;
    const int kValueRange = 1000;

    for (int i = 0; i < kNumOperations; ++i) {
        bool add = arena.empty() || rand.nextBool();

        if (add) {
            // ALLOCATE
            int val = rand.nextRangeU(0, kValueRange);
            TestNode* node = new TestNode(val, i);
            arena.push_back(node);

            // ACTION
            list.insert(node);

            // TRUTH UPDATE
            expected.push_back(node);
            // Keep expected sorted to match SkipList behavior
            std::sort(expected.begin(), expected.end(), [](TestNode* a, TestNode* b) {
                return *a < *b;
            });

        } else {
            // REMOVE
            if (expected.empty()) continue;

            // Pick a random index to remove
            int idx = rand.nextULessThan(expected.size());
            TestNode* nodeToRemove = expected[idx];

            // ACTION
            list.remove(nodeToRemove);

            // TRUTH UPDATE
            expected.erase(expected.begin() + idx);

            // Note: We don't delete the node from 'arena' because we might not
            // easily find it there, and we want to mimic an arena where memory
            // isn't immediately freed. We just ensure it's out of the list.
        }

        // VALIDATE every 50 ops (to save time) or if list is small
        if (i % 50 == 0 || expected.size() < 10) {
            validate_list(r, list, expected);
        }
    }

    // Final cleanup check
    for (TestNode* node : expected) {
        list.remove(node);
    }
    REPORTER_ASSERT(r, list.isEmpty());

    // Cleanup the heap allocated test nodes
    for (TestNode* node : arena) {
        delete node;
    }
}

DEF_TEST(SkInternalSkipList_Reset, r) {
    TestList list;
    TestNode n1(1, 1);
    TestNode n2(2, 2);

    list.insert(&n1);
    list.insert(&n2);

    REPORTER_ASSERT(r, !list.isEmpty());

    // Resetting the list pointers only
    // Note: In an intrusive list, reset() clears the container's view.
    // The nodes themselves might still hold garbage pointers if not handled carefully,
    // but the list itself should report empty.
    list.reset();

    REPORTER_ASSERT(r, list.isEmpty());
    REPORTER_ASSERT(r, list.head() == nullptr);
    REPORTER_ASSERT(r, list.tail() == nullptr);
}

DEF_TEST(SkInternalSkipList_PointerIntegrity, r) {
    // This test explicitly validates that the list is using the exact pointers
    // we give it (Arena style) and not copying data.
    TestList list;
    TestNode n1(100, 1);

    uintptr_t originalAddr = reinterpret_cast<uintptr_t>(&n1);

    list.insert(&n1);

    TestNode* retrieved = list.head();
    uintptr_t retrievedAddr = reinterpret_cast<uintptr_t>(retrieved);

    REPORTER_ASSERT(r, originalAddr == retrievedAddr);

    // Modify original, check list reflects it (proving by-reference)
    n1.fValue = 200;
    // WARNING: Changing value in-place breaks sorting invariant usually,
    // but here we just check if the list sees the change.
    REPORTER_ASSERT(r, list.head()->fValue == 200);

    // Clean up
    list.remove(&n1);
}
