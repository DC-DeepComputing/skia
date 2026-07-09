/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/capture/SkCaptureManager.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "src/capture/SkCapture.h"
#include "src/image/SkSurface_Base.h"

#include <memory>

SkCaptureManager::SkCaptureManager() {}

void SkCaptureManager::registerCanvas(SkCanvas* canvas) {
    TrackedCanvas tc;
    tc.fCanvas = canvas;
    if (fIsCurrentlyCapturing) {
        this->startRecording(&tc);
    }
    fTrackedCanvases.push_back(std::move(tc));
}

void SkCaptureManager::unregisterCanvas(SkCanvas* canvas) {
    for (int i = 0; i < fTrackedCanvases.size(); ++i) {
        if (fTrackedCanvases[i].fCanvas == canvas) {
            if (fIsCurrentlyCapturing) {
                this->stopRecording(&fTrackedCanvases[i]);
            }
            fTrackedCanvases.removeShuffle(i);
            break;
        }
    }
}

void SkCaptureManager::startRecording(TrackedCanvas* tc) {
    tc->fRecorder = std::make_unique<SkPictureRecorder>();
    SkCanvas* recordingCanvas = tc->fRecorder->beginRecording(
            tc->fCanvas->imageInfo().width(),
            tc->fCanvas->imageInfo().height());
    tc->fCanvas->setCaptureCanvas(recordingCanvas);
}

void SkCaptureManager::stopRecording(TrackedCanvas* tc) {
    if (!tc->fRecorder) {
        return;
    }
    tc->fCanvas->setCaptureCanvas(nullptr);
    sk_sp<SkPicture> picture = tc->fRecorder->finishRecordingAsPicture();
    if (picture) {
        if (auto surfaceBase = tc->fCanvas->getSurfaceBase()) {
            if (auto surfacePixelStorage = surfaceBase->getPixelStorage()) {
                surfacePixelStorage->incrementContentId();
            }
        }
        fPictures.emplace_back(std::move(picture));
    }
    tc->fRecorder.reset();
}

void SkCaptureManager::snapPictures() {
    for (auto& tc : fTrackedCanvases) {
        if (fIsCurrentlyCapturing && tc.fRecorder) {
            // Stop current recording, saving picture
            tc.fCanvas->setCaptureCanvas(nullptr);
            sk_sp<SkPicture> picture = tc.fRecorder->finishRecordingAsPicture();
            if (picture) {
                if (auto surfaceBase = tc.fCanvas->getSurfaceBase()) {
                    if (auto surfacePixelStorage = surfaceBase->getPixelStorage()) {
                        surfacePixelStorage->incrementContentId();
                    }
                }
                fPictures.emplace_back(std::move(picture));
            }
            // Start a new recording
            tc.fRecorder = std::make_unique<SkPictureRecorder>();
            SkCanvas* recordingCanvas = tc.fRecorder->beginRecording(
                    tc.fCanvas->imageInfo().width(),
                    tc.fCanvas->imageInfo().height());
            tc.fCanvas->setCaptureCanvas(recordingCanvas);
        }
    }
}

// TODO: make thread safe by using exchange() and a mutex.
void SkCaptureManager::toggleCapture(bool capturing) {
    if (capturing == fIsCurrentlyCapturing) {
        return;
    }
    if (capturing) {
        fIsCurrentlyCapturing = true;
        for (auto& tc : fTrackedCanvases) {
            this->startRecording(&tc);
        }
    } else {
        // on capture stop, save the capture and reset
        this->snapPictures();
        for (auto& tc : fTrackedCanvases) {
            this->stopRecording(&tc);
        }
        fLastCapture = SkCapture::MakeFromPictures(fPictures);
        fPictures.clear();
        fIsCurrentlyCapturing = false;
    }
}

void SkCaptureManager::snapPicture(SkSurface* surface) {
    for (auto& tc : fTrackedCanvases) {
        if (tc.fCanvas->getSurfaceBase() == surface) {
            if (fIsCurrentlyCapturing && tc.fRecorder) {
                tc.fCanvas->setCaptureCanvas(nullptr);
                sk_sp<SkPicture> picture = tc.fRecorder->finishRecordingAsPicture();
                if (picture) {
                    if (auto surfaceBase = tc.fCanvas->getSurfaceBase()) {
                        if (auto surfacePixelStorage = surfaceBase->getPixelStorage()) {
                            surfacePixelStorage->incrementContentId();
                        }
                    }
                    fPictures.emplace_back(std::move(picture));
                }
                tc.fRecorder = std::make_unique<SkPictureRecorder>();
                SkCanvas* recordingCanvas = tc.fRecorder->beginRecording(
                        tc.fCanvas->imageInfo().width(),
                        tc.fCanvas->imageInfo().height());
                tc.fCanvas->setCaptureCanvas(recordingCanvas);
            }
        }
    }
}

sk_sp<SkCapture> SkCaptureManager::getLastCapture() const {
   return fLastCapture;
}
