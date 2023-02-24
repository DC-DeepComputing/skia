/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCompressedDataUtils_DEFINED
#define SkCompressedDataUtils_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkTArray.h"

class SkBitmap;
class SkData;

static constexpr bool SkCompressionTypeIsOpaque(sk_image_factory::CompressionType compression) {
    switch (compression) {
        case sk_image_factory::CompressionType::kNone:            return true;
        case sk_image_factory::CompressionType::kETC2_RGB8_UNORM: return true;
        case sk_image_factory::CompressionType::kBC1_RGB8_UNORM:  return true;
        case sk_image_factory::CompressionType::kBC1_RGBA8_UNORM: return false;
    }

    SkUNREACHABLE;
}

size_t SkCompressedDataSize(sk_image_factory::CompressionType, SkISize baseDimensions,
                            SkTArray<size_t>* individualMipOffsets, bool mipmapped);
size_t SkCompressedBlockSize(sk_image_factory::CompressionType type);

/**
 * Returns the data size for the given sk_image_factory::CompressionType
 */
size_t SkCompressedFormatDataSize(sk_image_factory::CompressionType compressionType,
                                  SkISize dimensions, bool mipmapped);

 /*
  * This method will decompress the bottommost level in 'data' into 'dst'.
  */
bool SkDecompress(sk_sp<SkData> data,
                  SkISize dimensions,
                  sk_image_factory::CompressionType compressionType,
                  SkBitmap* dst);

#endif
