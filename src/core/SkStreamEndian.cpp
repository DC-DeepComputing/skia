/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStreamEndian.h"

bool SkWriteU16(SkWStream* s, uint16_t value, SkEndian endian) {
  if (endian != SkEndian::kNative) {
    value = SkEndianSwap16(value);
  }
  return s->write16(value);
}

bool SkWriteU32(SkWStream* s, uint32_t value, SkEndian endian);
bool SkWriteU64(SkWStream* s, uint64_t value, SkEndian endian);

bool SkWriteS16(SkWStream* s, int16_t value, SkEndian endian);
bool SkWriteS32(SkWStream* s, int32_t value, SkEndian endian);
bool SkWriteS64(SkWStream* s, int64_t value, SkEndian endian);

