/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDebugUtils_DEFINED
#define SkDebugUtils_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkTileMode.h"

#if defined(SK_DEBUG) || GR_TEST_UTILS
static constexpr const char* SkTileModeToStr(SkTileMode);
static constexpr const char* SkBlendModeToStr(SkBlendMode);
#endif

#endif // SkDebugUtils_DEFINED
