/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkParseEncodedOrigin.h"

#include "include/core/SkData.h"
#include "include/private/base/SkAssert.h"
#include "src/codec/SkExif.h"

bool SkParseEncodedOrigin(const uint8_t* data, size_t data_length, SkEncodedOrigin* orientation) {
    SkASSERT(orientation);
    SkExifMetadata exif(SkData::MakeWithoutCopy(data, data_length));
    return exif.getOrigin(orientation);
}
