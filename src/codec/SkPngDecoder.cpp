
#include "include/codec/SkPngDecoder.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/codec/SkPngCodec.h"

std::unique_ptr<SkCodec> SkPngDecoder::Decode(std::unique_ptr<SkStream> buffer,
                                              SkCodec::Result* outResult,
                                              SkPngChunkReader* reader) {
    SkCodec::Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }

    constexpr size_t bytesToRead = SkCodec::MinBufferedBytesNeeded();
    uint8_t buffer[bytesToRead];
    SkCodec::
    if (SkPngCodec::IsPng(buffer, bytesRead)) {
        return SkPngCodec::MakeFromStream(std::move(stream), nullptr, reader);
    }
}

std::unique_ptr<SkCodec> SkPngDecoder::Decode(sk_sp<SkData> data, SkCodec::Result* outResult,
                                              SkPngChunkReader* reader) {
    if (!data) {
        return nullptr;
    }
    return MakeFromStream(SkMemoryStream::Make(std::move(data)), outResult, reader);
}
