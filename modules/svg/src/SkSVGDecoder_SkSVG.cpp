/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkSVGDecoder.h"
#include "include/core/SkTypes.h"
#include "include/utils/SkBase64.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/svg/include/SkSVGDecoder_SkSVG.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGSVG.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGUse.h"

#include <memory>

namespace {
class DataResourceProvider final : public skresources::ResourceProvider {
public:
    static sk_sp<skresources::ResourceProvider> Make() {
        return sk_sp<skresources::ResourceProvider>(new DataResourceProvider());
    }

    sk_sp<skresources::ImageAsset> loadImageAsset(const char rpath[],
                                                  const char rname[],
                                                  const char rid[]) const override {
        if (auto data = decode_datauri("data:image/", rname)) {
            return skresources::MultiFrameImageAsset::Make(std::move(data));
        }
        return nullptr;
    }

private:
    DataResourceProvider() = default;

    static sk_sp<SkData> decode_datauri(const char prefix[], const char uri[]) {
        // We only handle B64 encoded image dataURIs: data:image/<type>;base64,<data>
        // (https://en.wikipedia.org/wiki/Data_URI_scheme)
        static constexpr char kDataURIEncodingStr[] = ";base64,";

        const size_t prefixLen = strlen(prefix);
        if (strncmp(uri, prefix, prefixLen) != 0) {
            return nullptr;
        }

        const char* encoding = strstr(uri + prefixLen, kDataURIEncodingStr);
        if (!encoding) {
            return nullptr;
        }

        const char* b64Data = encoding + SK_ARRAY_COUNT(kDataURIEncodingStr) - 1;
        size_t b64DataLen = strlen(b64Data);
        size_t dataLen;
        if (SkBase64::Decode(b64Data, b64DataLen, nullptr, &dataLen) != SkBase64::kNoError) {
            return nullptr;
        }

        sk_sp<SkData> data = SkData::MakeUninitialized(dataLen);
        void* rawData = data->writable_data();
        if (SkBase64::Decode(b64Data, b64DataLen, rawData, &dataLen) != SkBase64::kNoError) {
            return nullptr;
        }

        return data;
    }

    using INHERITED = ResourceProvider;
};
}  // namespace

bool SkSVGDecoder_SkSVG::render(SkCanvas& canvas,
                                const uint8_t* svg, size_t svgLength, int upem, SkGlyphID glyphId,
                                SkColor foregroundColor, SkSpan<SkColor> palette) {
    std::unique_ptr<SkStreamAsset> stream = SkMemoryStream::MakeDirect(svg, svgLength);
    if (!stream) {
        return false;
    }
    SkSVGDOM::Builder builder;
    builder.setResourceProvider(DataResourceProvider::Make());
    sk_sp<SkSVGDOM> skSvg = builder.make(*stream);
    if (!skSvg) {
        return false;
    }

    SkSize emSize = SkSize::Make(SkScalar(upem), SkScalar(upem));
    skSvg->setContainerSize(emSize);

    SkSVGPresentationContext pctx;
    pctx.fInherited.fColor = SkSVGProperty<SkSVGColorType , true>(foregroundColor);

    constexpr const size_t glyphStringLen = sizeof("glyph") - 1;
    char glyphIdString[glyphStringLen + kSkStrAppendU32_MaxSize + 1] = "glyph";
    *SkStrAppendU32(glyphIdString + glyphStringLen, glyphId) = 0;

    skSvg->render(&canvas, pctx, glyphIdString);
    return true;
}
