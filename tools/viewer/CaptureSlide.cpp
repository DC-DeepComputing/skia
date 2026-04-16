/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/CaptureSlide.h"

#include <string>

#include "imgui.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkDebug.h"
#include "src/capture/SkCapture.h"
#include "src/core/SkBigPicture.h"
#include "src/core/SkRecord.h"

const char* RecordTypeNames[] = {
#define CASE(T) #T,
        SK_RECORD_TYPES(CASE)
#undef CASE
};

CaptureSlide::CaptureSlide(const SkString& name, const SkString& path) {
    auto data = SkData::MakeFromFileName(path.c_str());
    fCapture = SkCapture::MakeFromData(data);
    if (fCapture) {
        fMetadata = fCapture->getMetadata();
    } else {
        SkDebugf("Couldn't load capture %s", path.c_str());
    }
}

CaptureSlide::~CaptureSlide() {}

void drawCommandHistory(const SkBigPicture* bigPicture) {
    const SkRecord* commands = bigPicture->record();
    ImGui::Begin("Command History");
    ImGui::Text("Total Commands: %d", commands->count());
    ImGui::Separator();
    auto printer = [](const auto& record) {
        int typeID = (int)record.kType;
        return SkStringPrintf("%s\n", RecordTypeNames[record.kType]);
    };

    ImGui::BeginChild("ScrollingRegion");

    for (int i = 0; i < commands->count(); ++i) {
        SkString cmdName = commands->visit(i, printer);
        ImGui::Text(cmdName.c_str());
    }

    ImGui::EndChild();
    ImGui::End();
}

void drawMetadata(int currentPictureIndex) {
    ImGui::Begin("Capture Info");
    ImGui::Text("Current Index: %d", currentPictureIndex);
    ImGui::End();
}

void CaptureSlide::draw(SkCanvas* canvas) {
    if (!fCapture) {
        return;
    }

    auto focusPicture = fCapture->getPicture(fCurrentPictureIndex);
    auto bounds = focusPicture->cullRect().roundOut();
    canvas->clipIRect(bounds, SkClipOp::kIntersect);
    canvas->drawPicture(fCapture->getPicture(fCurrentPictureIndex));
    drawMetadata(fCurrentPictureIndex);

    // TODO: this cast is improper and we should consider merging SkBigPicture
    // and SkPicture
    const SkBigPicture* bigPicture =
            static_cast<const SkBigPicture*>(focusPicture.get());

    drawCommandHistory(bigPicture);
}

bool CaptureSlide::animate(double) {
    if (fInvalidate) {
        fInvalidate = false;
        return true;
    }
    return fInvalidate;
}

void CaptureSlide::load(SkScalar, SkScalar) {}

void CaptureSlide::unload() { fCapture.reset(nullptr); }

SkISize CaptureSlide::getDimensions() const { return {0, 0}; }

bool CaptureSlide::onChar(SkUnichar c) {
    switch (c) {
        case 'N':
            fCurrentPictureIndex =
                    (fCurrentPictureIndex + 1) % fMetadata.numPictures;
            fInvalidate = true;
            return true;
        case 'P':
            fCurrentPictureIndex =
                    (fCurrentPictureIndex + fMetadata.numPictures - 1) %
                    fMetadata.numPictures;
            fInvalidate = true;
            return true;
    }

    return Slide::onChar(c);
}
