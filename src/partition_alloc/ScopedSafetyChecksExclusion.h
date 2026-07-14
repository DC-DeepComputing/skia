/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SRC_PARTITION_ALLOC_SCOPED_SAFETY_CHECKS_EXCLUSION_H_
#define SRC_PARTITION_ALLOC_SCOPED_SAFETY_CHECKS_EXCLUSION_H_

#if defined(__cplusplus)

#if defined(__has_include)
#    include "src/partition_alloc/safety_checks.h"
#  if __has_include("src/partition_alloc/safety_checks.h")
#    define SK_HAS_SCOPED_SAFETY_CHECKS_EXCLUSION 1
#  endif
#endif

#if !defined(SK_HAS_SCOPED_SAFETY_CHECKS_EXCLUSION)
namespace base {

// A "no-op" implementation of Chromium's base::ScopedSafetyChecksExclusion
// for use in standalone Skia builds where PartitionAlloc/Chromium base is missing.
class ScopedSafetyChecksExclusion {
public:
    ScopedSafetyChecksExclusion() = default;
    ~ScopedSafetyChecksExclusion() {} // Non-trivial to suppress unused variable warning

    // Disallow copy and move to match the STACK_ALLOCATED semantics in Chromium
    ScopedSafetyChecksExclusion(const ScopedSafetyChecksExclusion&) = delete;
    ScopedSafetyChecksExclusion& operator=(const ScopedSafetyChecksExclusion&) = delete;
};

}  // namespace base
#endif  // !defined(SK_HAS_SCOPED_SAFETY_CHECKS_EXCLUSION)

#endif  // defined(__cplusplus)

#endif  // SRC_PARTITION_ALLOC_SCOPED_SAFETY_CHECKS_EXCLUSION_H_
