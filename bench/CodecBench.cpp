/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/CodecBench.h"
#include "bench/CodecBenchPriv.h"
#include "include/codec/SkCodec.h"
#if defined(SK_CODEC_DECODES_JPEG_WITH_RUST)
#include "experimental/rust_jpeg/decoder/SkJpegRustDecoder.h"
#endif
#include "include/core/SkBitmap.h"
#include "src/core/SkOSFile.h"
#include "tools/flags/CommandLineFlags.h"

// Actually zeroing the memory would throw off timing, so we just lie.
static DEFINE_bool(zero_init, false,
                   "Pretend our destination is zero-intialized, simulating Android?");

static std::unique_ptr<SkCodec> make_codec(SkData* data) {
#if defined(SK_CODEC_DECODES_JPEG_WITH_RUST)
    if (SkJpegRustDecoder::IsJpeg(data->data(), data->size())) {
        return SkJpegRustDecoder::Decode(sk_ref_sp<const SkData>(data), nullptr);
    }
#endif
    return SkCodec::MakeFromData(sk_ref_sp<const SkData>(data));
}

CodecBench::CodecBench(SkString baseName, SkData* encoded, SkColorType colorType,
        SkAlphaType alphaType)
    : fColorType(colorType)
    , fAlphaType(alphaType)
    , fData(SkRef(encoded))
{
    // Parse filename and the color type to give the benchmark a useful name
    fName.printf("Codec_%s_%s%s", baseName.c_str(), color_type_to_str(colorType),
            alpha_type_to_str(alphaType));
    // Ensure that we can create an SkCodec from this data.
    SkASSERT(make_codec(fData.get()));
}

const char* CodecBench::onGetName() {
    return fName.c_str();
}

bool CodecBench::isSuitableFor(Backend backend) {
    return Backend::kNonRendering == backend;
}

void CodecBench::onDelayedSetup() {
    std::unique_ptr<SkCodec> codec = make_codec(fData.get());

    fInfo = codec->getInfo().makeColorType(fColorType)
                            .makeAlphaType(fAlphaType)
                            .makeColorSpace(nullptr);

    fPixelStorage.reset(fInfo.computeMinByteSize());
}

void CodecBench::onDraw(int n, SkCanvas* canvas) {
    std::unique_ptr<SkCodec> codec;
    SkCodec::Options options;
    if (FLAGS_zero_init) {
        options.fZeroInitialized = SkCodec::kYes_ZeroInitialized;
    }
    for (int i = 0; i < n; i++) {
        codec = make_codec(fData.get());
#ifdef SK_DEBUG
        const SkCodec::Result result =
#endif
        codec->getPixels(fInfo, fPixelStorage.get(), fInfo.minRowBytes(),
                         &options);
        SkASSERT(result == SkCodec::kSuccess
                 || result == SkCodec::kIncompleteInput);
    }
}
