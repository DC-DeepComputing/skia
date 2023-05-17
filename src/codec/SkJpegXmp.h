/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegXmp_codec_DEFINED
#define SkJpegXmp_codec_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/codec/SkXmp.h"
#include "src/xml/SkDOM.h"

class SkData;
struct SkGainmapInfo;

#include <memory>
#include <vector>

// Find and parse all XMP metadata, given a list of all APP1 segment parameters.
std::unique_ptr<SkXmp> skjpeg_make_xmp(const std::vector<sk_sp<SkData>>& decoderApp1Params);

#endif
