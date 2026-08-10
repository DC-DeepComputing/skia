/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContextOptions_DEFINED
#define SkContextOptions_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkAPI.h"

#include <memory>
#include <optional>

class SkData;
class SkImageGenerator;
class SkOpenTypeSVGDecoder;
class SkTraceMemoryDump;

struct SK_API SkContextOptions {
    /**
     *  Specify the max number of bytes that should be used by the font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     *  A value of zero means no limit.
     */
    size_t fFontCacheLimit = 0;
};

#endif  // SkContextOptions_DEFINED
