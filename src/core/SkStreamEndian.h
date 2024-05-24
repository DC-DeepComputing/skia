/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStreamEndian_DEFINED
#define SkStreamEndian_DEFINED

#include "include/core/SkStream.h"
#include "src/base/SkEndian.h"

enum class SkEndian {
  kBig,
  kLittle,
#ifdef SK_CPU_LENDIAN
  kNative = kLittle,
#else   // SK_CPU_BENDIAN
  kNative = kBig,
#endif
};

bool SkWriteU16(SkWStream* s, uint16_t value, SkEndian endian);
bool SkWriteU32(SkWStream* s, uint32_t value, SkEndian endian);
bool SkWriteS16(SkWStream* s, int16_t value, SkEndian endian);
bool SkWriteS32(SkWStream* s, int32_t value, SkEndian endian);
bool SkWriteU16_BE(SkWStream* s, uint16_t value);
bool SkWriteU32_BE(SkWStream* s, uint32_t value);
bool SkWriteS16_BE(SkWStream* s, int16_t value);
bool SkWriteS32_BE(SkWStream* s, int32_t value);
bool SkReadU16(SkStream* s, uint16_t* value, SkEndian endian);
bool SkReadU32(SkStream* s, uint32_t* value, SkEndian endian);
bool SkReadS16(SkStream* s, int16_t* value, SkEndian endian);
bool SkReadS32(SkStream* s, int32_t* value, SkEndian endian);
bool SkReadSignedRational(SkStream* s, float* value, SkEndian endian);
bool SkReadUnsignedRational(SkStream* s, float* value, SkEndian endian);

#endif  // SkStreamEndian_DEFINED
