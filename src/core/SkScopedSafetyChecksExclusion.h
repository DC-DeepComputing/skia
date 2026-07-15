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

#define SK_SCOPED_DISABLE_PARTITION_ALLOC_SAFETY_CHECKS \
    base::ScopedSafetyChecksExclusion _miracleObjExclusion

#else

// Macro is empty in the no-op case for standalone Skia builds
#define SK_SCOPED_DISABLE_PARTITION_ALLOC_SAFETY_CHECKS

#endif  // SK_USE_CHROMIUM_SAFETY_CHECKS

#endif  // SkScopedSafetyChecksExclusion_DEFINED
