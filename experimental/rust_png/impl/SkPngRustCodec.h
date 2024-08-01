/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustCodec_DEFINED
#define SkPngRustCodec_DEFINED

#include <memory>

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "third_party/rust/cxx/v1/cxx.h"

struct SkEncodedInfo;
class SkStream;

// This class provides the Skia image decoding API on top of the Rust `png`
// crate.  Note that post-processing of the decoded pixels is done using C++
// (e.g. using `SkSwizzle` and/or `skcms_Transform`) and therefore this class
// should not be considered compliant with Chromium's Rule-or-2.
class SkPngRustCodec : public SkCodec {
public:
    static std::unique_ptr<SkPngRustCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

private:
    using INHERITED = SkCodec;

    // SkCodec overrides:
    ~SkPngRustCodec() override;
    SkEncodedImageFormat onGetEncodedFormat() const override;
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
