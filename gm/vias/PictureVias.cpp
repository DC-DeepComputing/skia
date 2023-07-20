/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "gm/vias/Draw.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/utils/SkBase64.h"

#include <sstream>
#include <string>

// Encodes a bitmap as a base-64 image/png URI. In the presence of errors, the returned string will
// contain a human-friendly error message. Otherwise the string will start with "data:image/png".
//
// Based on
// https://skia.googlesource.com/skia/+/650c980daa72d887602e701db8f84072e26d4d48/tests/TestUtils.cpp#127.
static std::string bitmap_to_base64_data_uri(const SkBitmap& bitmap) {
    SkPixmap pm;
    if (!bitmap.peekPixels(&pm)) {
        return "peekPixels failed";
    }

    // We're going to embed this PNG in a data URI, so make it as small as possible
    SkPngEncoder::Options options;
    options.fFilterFlags = SkPngEncoder::FilterFlag::kAll;
    options.fZLibLevel = 9;

    SkDynamicMemoryWStream wStream;
    if (!SkPngEncoder::Encode(&wStream, pm, options)) {
        return "SkPngEncoder::Encode failed";
    }

    sk_sp<SkData> pngData = wStream.detachAsData();
    size_t len = SkBase64::Encode(pngData->data(), pngData->size(), nullptr);

    // The PNG can be almost arbitrarily large. We don't want to fill our logs with enormous URLs.
    // Infra says these can be pretty big, as long as we're only outputting them on failure.
    static const size_t kMaxBase64Length = 1024 * 1024;
    if (len > kMaxBase64Length) {
        return (std::ostringstream() << "Encoded image too large (" << len << " bytes)").str();
    }

    std::string out;
    out.resize(len);
    out.data();
    SkBase64::Encode(pngData->data(), pngData->size(), out.data());
    return "data:image/png;base64," + out;
}

// This draw() implementation supports the "picture" and "picture_serialization" vias.
//
// The "picture" via tests that if we record a GM using an SkPictureRecorder, the bitmap produced
// by drawing the recorded picture on the surface under test is the same as the bitmap obtained by
// drawing the GM directly on the surface under test.
//
// The "picture_serialization" via is identical to the "picture" via, except that the recorded
// picture is serialized and then deserialized before being drawn on the surface under test.
//
// Based on DM's ViaSerialization and ViaPicture classes here:
//   https://skia.googlesource.com/skia/+/dcc56df202cca129edda3f6f8bae04ec306b264e/dm/DMSrcSink.cpp#2281
//   https://skia.googlesource.com/skia/+/dcc56df202cca129edda3f6f8bae04ec306b264e/dm/DMSrcSink.cpp#2310
GMOutput draw(skiagm::GM* gm, SkSurface* surface, std::string via) {
    bool serialize;

    if (via == "picture") {
        serialize = false;
    } else if (via == "picture_serialization") {
        serialize = true;
    } else {
        SK_ABORT("unknown --via flag value: %s", via.c_str());
    }

    // Draw GM on a recording canvas.
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas =
            recorder.beginRecording(gm->getISize().width(), gm->getISize().height());
    SkString msg;
    skiagm::DrawResult result = gm->draw(recordingCanvas, &msg);
    if (result != skiagm::DrawResult::kOk) {
        return {result, msg.c_str()};
    }

    // Finish recording, and optionally serialize and then deserialize the resulting picture. Note
    // that the pic->serialize() call uses the default behavior from SkSerialProcs, which implies a
    // dependency on libpng.
    sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
    if (serialize) {
        pic = SkPicture::MakeFromData(pic->serialize().get());
    }

    // Draw the recorded picture on the surface under test and extract it as a bitmap.
    surface->getCanvas()->drawPicture(pic);
    SkBitmap recordedBitmap;
    recordedBitmap.allocPixelsFlags(surface->getCanvas()->imageInfo(),
                                    SkBitmap::kZeroPixels_AllocFlag);
    if (!surface->readPixels(recordedBitmap, 0, 0)) {
        return {skiagm::DrawResult::kFail, "Could not read recorded picture pixels from surface"};
    }

    // Draw GM on the surface under test and extract the reference bitmap.
    result = gm->draw(surface->getCanvas(), &msg);
    if (result != skiagm::DrawResult::kOk) {
        return {result, msg.c_str()};
    }
    SkBitmap referenceBitmap;
    referenceBitmap.allocPixelsFlags(surface->getCanvas()->imageInfo(),
                                     SkBitmap::kZeroPixels_AllocFlag);
    if (!surface->readPixels(referenceBitmap, 0, 0)) {
        return {skiagm::DrawResult::kFail, "Could not read reference picture pixels from surface"};
    }
    // The recorded and reference bitmaps should be identical.
    if (recordedBitmap.computeByteSize() != referenceBitmap.computeByteSize()) {
        return {skiagm::DrawResult::kFail,
                (std::ostringstream()
                 << "Recorded and reference bitmap dimensions do not match: "
                 << "expected byte size " << referenceBitmap.computeByteSize() << ", width "
                 << referenceBitmap.bounds().width() << " and height "
                 << referenceBitmap.bounds().height() << "; got "
                 << recordedBitmap.computeByteSize() << ", " << recordedBitmap.bounds().width()
                 << " and " << recordedBitmap.bounds().height())
                        .str()};
    }
    if (0 != memcmp(recordedBitmap.getPixels(),
                    referenceBitmap.getPixels(),
                    referenceBitmap.computeByteSize())) {
        return {skiagm::DrawResult::kFail,
                (std::ostringstream()
                 << "Recorded and reference bitmap pixels do not match."
                 << "\nRecorded image:\n"
                 << bitmap_to_base64_data_uri(recordedBitmap) << "\nReference image:\n"
                 << bitmap_to_base64_data_uri(referenceBitmap))
                        .str()};
    }

    return {result, msg.c_str(), referenceBitmap};
}
