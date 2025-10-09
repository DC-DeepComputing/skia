/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/capture/SkCaptureManager.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/private/base/SkDebug.h"
#include "src/capture/SkCapture.h"
#include "src/capture/SkCaptureCanvas.h"

#include <memory>
#include <optional>

SkCaptureManager::SkCaptureManager() {}

SkCanvas* SkCaptureManager::makeCaptureCanvas(SkCanvas* canvas) {
    auto newCanvas = std::make_unique<SkCaptureCanvas>(canvas, this);
    auto rawCanvasPtr = newCanvas.get();
    fTrackedCanvases.emplace_back(std::move(newCanvas));
    return rawCanvasPtr;
}

void SkCaptureManager::snapPictures() {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            auto picture = canvas->snapPicture();
            if (picture) {
                fPictures.emplace_back(picture);
            }
        }
    }
}

void SkCaptureManager::snapPicture(SkSurface* surface) {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            if (canvas->getSurface() == surface) {
                auto picture = canvas->snapPicture();
                if (picture) {
                    // TODO(412351769): for every storing of a picture, we should track a content id
                    // and the surface it was drawn to.
                    fPictures.emplace_back(picture);
                }
                return;
            }
        }
    }
}

// TODO(b/412351769): When serializing our SkPictures, the images that are drawn through drawImage
// and similar functions will also need to be serialized. Instead of naively encoding all images as
// PNGs, we want the images that refer to content created from an SkSurface to point to its
// corresponding SkPicture. This means that we need to create a context for the proc to track these
// contentIDs.
sk_sp<SkData> SkCaptureManager::serializeImageProc(SkImage* img, void* ctx) {
    const int contentID = -1; // TODO: replace with real content ID.
    sk_sp<SkData> data = SkData::MakeWithCopy(&contentID, sizeof(int));
    return data;
}

sk_sp<SkImage> SkCaptureManager::deserializeImageProc(sk_sp<SkData> data, std::optional<SkAlphaType> a, void* ctx) {
    // TODO: set up the SkCapture context and inspect it to grab SkPictures and pass them as images.
    SkBitmap b;
    b.allocN32Pixels(5, 5);
    SkCanvas canvas(b);
    canvas.drawColor(SK_ColorMAGENTA);
    return b.asImage();
}


sk_sp<SkData> SkCaptureManager::serializeCapture() {
    SkDynamicMemoryWStream stream;

    stream.write(SkCapture::kMagic, sizeof(SkCapture::kMagicSize));
    stream.write32(SkCapture::kVersion);

    // Number of pictures
    stream.write32(fPictures.size());

    // TODO (b/412351769): Write metadata on each picture and assosiated canvas. This will be needed
    // when we have multiple SkPictures per canvas and we want to track which ones are drawn into
    // each other.

    for (const auto& picture : fPictures) {
        SkDynamicMemoryWStream pictureStream;
        SkSerialProcs procs;
        procs.fImageProc = SkCaptureManager::serializeImageProc;
        picture->serialize(&pictureStream, &procs);
        sk_sp<SkData> pictureData = pictureStream.detachAsData();

        // Write size and then data
        stream.write64(pictureData->size());
        stream.write(pictureData->data(), pictureData->size());
    }

    auto data = stream.detachAsData();
    SkDebugf("Wrote %d pictures from %d canvases to sk_capture.skcp\n", fPictures.size(),
        fTrackedCanvases.size());
    return data;
}
