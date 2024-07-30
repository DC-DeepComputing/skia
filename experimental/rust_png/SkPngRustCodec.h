/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustCodec_DEFINED
#define SkPngRustCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "third_party/rust/cxx/v1/cxx.h"

#include <memory>

struct SkEncodedInfo;
class SkStream;

class SkPngRustCodec : public SkCodec {
public:
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    ~SkPngRustCodec() override;

private:
    using INHERITED = SkCodec;

    SkEncodedImageFormat onGetEncodedFormat() const override { return SkEncodedImageFormat::kPNG; }

    Result onGetPixels(const SkImageInfo& info,
                       void* pixels,
                       size_t rowBytes,
                       const Options&,
                       int* rowsDecoded) override;

    SkPngRustCodec(SkEncodedInfo&&,
                   XformFormat,
                   std::unique_ptr<SkStream>,
                   rust::Vec<uint8_t> decodedData);

    rust::Vec<uint8_t> fDecodedData;
};

#endif  // SkPngCodec_DEFINED
