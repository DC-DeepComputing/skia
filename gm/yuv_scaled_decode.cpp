/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/graphite/Image.h"
#include "tools/Resources.h"

DEF_SIMPLE_GM_CAN_FAIL(yuv_scaled_decode, canvas, error, 400, 267) {
    auto context = GrAsDirectContext(canvas->recordingContext());
    skgpu::graphite::Recorder* recorder = nullptr;
#if defined(SK_GRAPHITE)
    recorder = canvas->recorder();
#endif
    if (!recorder && !context) {
        *error = "Must draw directly to GrContext or graphite::Recorder.";
        return skiagm::DrawResult::kSkip;
    }

    auto make_image = [&](const SkYUVAPixmaps& pixmaps) {
#if defined(SK_GRAPHITE)
        if (recorder) {
            return SkImages::TextureFromYUVAPixmaps(recorder, pixmaps);
        }
#endif
        return SkImages::TextureFromYUVAPixmaps(context, pixmaps);
    };

    static constexpr const char* imageFiles[] = {
            "images/mandrill_h2v1.jpg",
            "images/mandrill_h4v2.jpg",
    };

    for (auto scale : {1.f / 4.f, 1 / 8.f}) {
        canvas->save();
        int w = 0;
        for (const auto& file : imageFiles) {
            auto codec = SkCodec::MakeFromStream(GetResourceAsStream(file));
            if (!codec) {
                *error = SkStringPrintf("Could not make codec from %s.", file);
                return skiagm::DrawResult::kFail;
            }
            SkYUVAPixmapInfo info;
            if (!codec->queryYUVAInfo(SkYUVAPixmapInfo::SupportedDataTypes::All(), scale, &info)) {
                *error = SkStringPrintf("Could decode %s at scale %f.", file, scale);
                return skiagm::DrawResult::kFail;
            }

            auto pixmaps = SkYUVAPixmaps::Allocate(info);
            if (!pixmaps.isValid()) {
                *error = SkStringPrintf(
                        "Could not allocate pixmaps for %s at scale %f.", file, scale);
                return skiagm::DrawResult::kFail;
            }
            if (codec->getYUVAPlanes(pixmaps) != SkCodec::kSuccess) {
                *error = SkStringPrintf("Could not get planes for %s at scale %f.", file, scale);
                return skiagm::DrawResult::kFail;
            }

            SkBitmap referenceBmp;
            referenceBmp.allocPixels(
                    codec->getInfo().makeDimensions(pixmaps.yuvaInfo().dimensions()));
            if (codec->getPixels(referenceBmp.pixmap()) != SkCodec::kSuccess) {
                *error = SkStringPrintf(
                        "Could not get reference bmp for %s at scale %f.", file, scale);
                return skiagm::DrawResult::kFail;
            }
            auto reference = SkImages::RasterFromBitmap(std::move(referenceBmp));

            auto yuvImage = make_image(pixmaps);
            if (!yuvImage) {
                *error = SkStringPrintf("Could not make SkImage for %s at scale %f.", file, scale);
                return skiagm::DrawResult::kFail;
            }

            canvas->drawImage(yuvImage, 0, 0);
            canvas->drawImage(reference, yuvImage->width() + 5, 0);
            canvas->translate(0, yuvImage->height() + 10);
            w = std::max(w, yuvImage->width());
        }
        canvas->restore();
        canvas->translate(2 * (w + 5), 0);
    }
    return skiagm::DrawResult::kOk;
}
