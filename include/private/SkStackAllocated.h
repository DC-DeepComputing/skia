/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStackAllocated_DEFINED
#define SkStackAllocated_DEFINED

#include <stddef.h>

namespace skcpu {

/** \class SkStackAllocated

    Deriving from this class prevents the object from being allocated on the heap.
    It also serves as a marker for static analysis tools (like the MiraclePtr rewriter
    and lifetimer Checkers) to identify stack-allocated types.
*/
class SkStackAllocated {
public:
    // Marker for Clang plugins/rewriter to identify stack-allocated types.
    using IsStackAllocatedTypeMarker [[maybe_unused]] = int;

private:
    // Ban heap allocation
    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;
};

}  // namespace skcpu

#endif
