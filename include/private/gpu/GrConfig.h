
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrConfig_DEFINED
#define GrConfig_DEFINED

#include "include/private/base/SkDebug.h"

/**
 *  Gr defines are set to 0 or 1, rather than being undefined or defined
 */

#if !defined(GR_CACHE_STATS)
  #if defined(SK_DEBUG) || defined(SK_DUMP_STATS)
      #define GR_CACHE_STATS  1
  #else
      #define GR_CACHE_STATS  0
  #endif
#endif

#if !defined(GR_GPU_STATS)
  #if defined(SK_DEBUG) || defined(SK_DUMP_STATS) || GR_TEST_UTILS
      #define GR_GPU_STATS    1
  #else
      #define GR_GPU_STATS    0
  #endif
#endif

#endif
