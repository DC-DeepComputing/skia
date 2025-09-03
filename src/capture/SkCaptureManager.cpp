/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/capture/SkCaptureManager.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkDebug.h"
#include "src/capture/SkCaptureCanvas.h"

#include <memory>

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

sk_sp<SkData> SkCaptureManager::serializeCapture() {
    SkDynamicMemoryWStream stream;

    // Magic number and version
    static const char kMagic[] = { 's', 'k', 'i', 'a', 'c', 'a', 'p', 't' };
    const uint32_t kVersion = 1;
    stream.writeText(kMagic);
    stream.write32(kVersion);

    // Number of pictures
    stream.write32(fPictures.size());

    // TODO (b/412351769): Write metadata on each picture and assosiated canvas. This will be needed
    // when we have multiple SkPictures per canvas and we want to track which ones are drawn into
    // each other.

    for (const auto& picture : fPictures) {
        SkDynamicMemoryWStream pictureStream;
        picture->serialize(&pictureStream);
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
