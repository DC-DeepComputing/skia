/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStackAllocated_DEFINED
#define SkStackAllocated_DEFINED

#include <stddef.h>

/** \class SkStackAllocated

    Deriving from this class prevents the object from being allocated on the heap.
    It also serves as a marker for static analysis tools (like the MiraclePtr rewriter)
    to identify stack-allocated types.
*/
class SkStackAllocated {
public:
    // Marker for Clang plugins/rewriter to identify stack-allocated types.
    // In Chromium, Clang plugins enforce that:
    // 1. StackAllocated structs are only embedded inside other StackAllocated structs.
    // 2. Pointers to StackAllocated structs are not rewritten to raw_ptr.
    using IsStackAllocatedTypeMarker [[maybe_unused]] = int;

private:
    // Ban heap allocation
    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;
};

#endif
