// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Picture_MakeFromStream, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* pictureCanvas = recorder.beginRecording({0, 0, 256, 256});
    SkPaint paint;
    pictureCanvas->drawRect(SkRect::MakeWH(200, 200), paint);
    paint.setColor(SK_ColorWHITE);
    pictureCanvas->drawRect(SkRect::MakeLTRB(20, 20, 180, 180), paint);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    SkDynamicMemoryWStream writableStream;

    // The above drawing doesn't have any images to serialize, but if such
    // serialization were necessary, one would need to register an encoder.
    SkSerialProcs sProcs;
    sProcs.fImageProc = [](SkImage* img, void*) -> SkSerialReturnType {
        return SkPngEncoder::Encode(nullptr, img, SkPngEncoder::Options{});
    };
    picture->serialize(&writableStream, nullptr);
    std::unique_ptr<SkStreamAsset> readableStream = writableStream.detachAsStream();

    SkDeserialProcs procs;
    // The above SKP doesn't have any image data in it, but if it did, a decoder
    // needs to be registered like this.
    procs.fImageDataProc = [](sk_sp<SkData> data, std::optional<SkAlphaType>, void*) -> sk_sp<SkImage> {
        if (!SkPngDecoder::IsPng(data->data(), data->size())) {
            SkDebugf("non-png image serialized in skp\n");
            return nullptr;
        }
        auto codec = SkPngDecoder::Decode(data, nullptr);
        if (!codec) {
            SkDebugf("Invalid png data detected\n");
            return nullptr;
        }
        return std::get<0>(codec->getImage());
    };
    sk_sp<SkPicture> copy = SkPicture::MakeFromStream(readableStream.get(), &procs);
    copy->playback(canvas);
}
}  // END FIDDLE
