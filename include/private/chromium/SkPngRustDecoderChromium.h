/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPngRustDecoderChromium_DEFINED
#define SkPngRustDecoderChromium_DEFINED

#include <memory>

#include "include/codec/SkCodec.h"
#include "include/private/base/SkAPI.h"

class SkStream;

namespace SkPngRustDecoder {

// Decode a PNG, optionally forcing the use of skcms for ICC profile parsing
// instead of the build-default parser. This is intended for Chromium
// experiments comparing the skcms and Rust (moxcms) ICC implementations.
SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext,
                                       bool forceSkCMSICCParsing);

}  // namespace SkPngRustDecoder

#endif  // SkPngRustDecoderChromium_DEFINED
