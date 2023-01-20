/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegGainmapEncoder_DEFINED
#define SkJpegGainmapEncoder_DEFINED

#include "include/encode/SkJpegEncoder.h"

class SkPixmap;
class SkWStream;
struct SkGainmapInfo;

class SK_API SkJpegGainmapEncoder {
public:
    static bool EncodeJpegR(SkWStream* dst,
                            const SkPixmap& base,
                            const SkJpegEncoder::Options& baseOptions,
                            const SkPixmap& gainmap,
                            const SkJpegEncoder::Options& gainmapOptions,
                            const SkGainmapInfo& gainmapInfo);
};

#endif
