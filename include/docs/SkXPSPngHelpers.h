/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkXPSPNGHelpers_DEFINED
#define SkXPSPNGHelpers_DEFINED

#include "include/encode/SkPngEncoder.h"

class SkPixmap;
class SkWStream;

namespace SkXPS::PNG {

inline bool Encode(SkWStream* dst, const SkPixmap& src) {
    return SkPngEncoder::Encode(dst, src, {});
}

}  // namespace SkXPS::PNG

#endif
