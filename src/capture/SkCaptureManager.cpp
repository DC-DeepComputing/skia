/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/capture/SkCaptureManager.h"
#include <memory>
#include "include/core/SkCanvas.h"
#include "src/capture/SkCaptureCanvas.h"

SkCaptureManager::SkCaptureManager() {}

SkCaptureCanvas* SkCaptureManager::makeCaptureCanvas(SkCanvas* canvas) {
    auto newCanvas = std::make_unique<SkCaptureCanvas>(canvas);
    auto rawCanvasPtr = newCanvas.get();
    fTrackedCanvases.emplace_back(std::move(newCanvas));
    return rawCanvasPtr;
}
