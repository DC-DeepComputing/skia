/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/DecodeUtils.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "src/image/SkImageGeneratorPriv.h"

#include <memory>
#include <utility>

namespace ToolUtils {

bool DecodeDataToBitmap(sk_sp<SkData> data, SkBitmap* dst) {
    std::unique_ptr<SkImageGenerator> gen(SkImageGenerators::MakeFromEncoded(std::move(data)));
    return gen && dst->tryAllocPixels(gen->getInfo()) &&
           gen->getPixels(
                   gen->getInfo().makeColorSpace(nullptr), dst->getPixels(), dst->rowBytes());
}

bool DecodeFile(sk_sp<SkData> data, SkBitmap* bitmap, SkColorType colorType, bool requireUnpremul) {
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(std::move(data));
    if (!codec) {
        return false;
    }

    SkImageInfo info = codec->getInfo().makeColorType(colorType);
    if (requireUnpremul && kPremul_SkAlphaType == info.alphaType()) {
        info = info.makeAlphaType(kUnpremul_SkAlphaType);
    }

    if (!bitmap->tryAllocPixels(info)) {
        return false;
    }

    return SkCodec::kSuccess == codec->getPixels(info, bitmap->getPixels(), bitmap->rowBytes());
}

bool DecodeFile(const char* filename,
                SkBitmap* bitmap,
                SkColorType colorType,
                bool requireUnpremul) {
    return DecodeFile(SkData::MakeFromFileName(filename), bitmap, colorType, requireUnpremul);
}

sk_sp<SkImage> DecodeFile(const char filename[]) {
    sk_sp<SkData> data(SkData::MakeFromFileName(filename));
    return data ? SkImages::DeferredFromEncodedData(data) : nullptr;
}

}  // namespace ToolUtils
