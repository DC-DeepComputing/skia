/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCompressedDataUtils_DEFINED
#define SkCompressedDataUtils_DEFINED

#include "include/core/SkCompressionType.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"

#include <cstddef>

class SkBitmap;
class SkData;
struct SkISize;

static constexpr bool SkCompressionTypeIsOpaque(SkCompressionType compression) {
    switch (compression) {
        case SkCompressionType::kNone:            return true;
        case SkCompressionType::kETC2_RGB8_UNORM: return true;
        case SkCompressionType::kBC1_RGB8_UNORM:  return true;
        case SkCompressionType::kBC1_RGBA8_UNORM: return false;
    }

    SkUNREACHABLE;
}

size_t SkCompressedDataSize(SkCompressionType, SkISize baseDimensions,
                            skia_private::TArray<size_t>* individualMipOffsets, bool mipmapped);
size_t SkCompressedBlockSize(SkCompressionType type);

/**
 * Returns the data size for the given SkCompressionType
 */
size_t SkCompressedFormatDataSize(SkCompressionType compressionType,
                                  SkISize dimensions, bool mipmapped);

 /*
  * This method will decompress the bottommost level in 'data' into 'dst'.
  */
bool SkDecompress(sk_sp<SkData> data,
                  SkISize dimensions,
                  SkCompressionType compressionType,
                  SkBitmap* dst);

#endif
