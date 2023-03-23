/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngDecoder_DEFINED
#define SkPngDecoder_DEFINED

#include "include/codec/SkCodec.h"
#include "include/codec/SkPngChunkReader.h"
#include "include/core/SkRefCnt.h"

class SkCodec;
class SkData;
class SkStream;

#include <memory>

class SK_API SkPngDecoder {
public:

    /**
     *  Attempts to decode the given bytes as a PNG.
     *
     *  If the bytes are not a PNG, returns nullptr.
     */
    static std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                           SkCodec::Result* = nullptr,
                                           SkPngChunkReader* = nullptr);
    static std::unique_ptr<SkCodec> Decode(sk_sp<SkData>, SkCodec::Result* = nullptr,
                                           SkPngChunkReader* = nullptr);

private:
    SkPngDecoder(); // not meant to be instantiated
};

#endif  // SkPngDecoder_DEFINED
