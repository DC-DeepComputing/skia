/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScopedSafetyChecksExclusion_DEFINED
#define SkScopedSafetyChecksExclusion_DEFINED

#if defined(SK_USE_CHROMIUM_SAFETY_CHECKS)

#include "base/memory/safety_checks.h"

using SkScopedSafetyChecksExclusion = base::ScopedSafetyChecksExclusion;

#else

// A "no-op" implementation of Chromium's base::ScopedSafetyChecksExclusion
// for use in standalone Skia builds where PartitionAlloc/Chromium base is missing.
class SkScopedSafetyChecksExclusion {
public:
    SkScopedSafetyChecksExclusion() = default;
    ~SkScopedSafetyChecksExclusion() {} // Non-trivial to suppress unused variable warning

    // Disallow copy and move to match the STACK_ALLOCATED semantics in Chromium
    SkScopedSafetyChecksExclusion(const SkScopedSafetyChecksExclusion&) = delete;
    SkScopedSafetyChecksExclusion& operator=(const SkScopedSafetyChecksExclusion&) = delete;
    SkScopedSafetyChecksExclusion(SkScopedSafetyChecksExclusion&&) = delete;
    SkScopedSafetyChecksExclusion& operator=(SkScopedSafetyChecksExclusion&&) = delete;
};

#endif  // SK_USE_CHROMIUM_SAFETY_CHECKS

#endif  // SkScopedSafetyChecksExclusion_DEFINED
