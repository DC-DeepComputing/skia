/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/CaptureSlide.h"

#include "imgui.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/SkDebug.h"
#include "src/capture/SkCapture.h"
#include "src/core/SkBigPicture.h"
#include "src/core/SkRecord.h"
#include "src/utils/SkJSONWriter.h"
#include "tools/ProcsUtils.h"
#include "tools/UrlDataManager.h"
#include "tools/debugger/DebugCanvas.h"
#include "tools/debugger/DrawCommand.h"

#include <string>

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

void drawCommandHistory(const DebugCanvas* debugCanvas) {
    static int currentCommandIdx = 0;

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Appearing);
    if (!ImGui::Begin("Command History")) {
        ImGui::End();
        return;
    }

    ImGui::Text("Total Commands: %d", debugCanvas->getSize());
    ImGui::Separator();

    const char* previewValue = "Select a command...";
    if (currentCommandIdx >= 0 && currentCommandIdx < debugCanvas->getSize()) {
        DrawCommand* cmd = debugCanvas->getDrawCommandAt(currentCommandIdx);
        previewValue = DrawCommand::GetCommandString(cmd->getOpType());
    }

    if (ImGui::BeginCombo("Select Command", previewValue)) {
        for (int i = 0; i < debugCanvas->getSize(); ++i) {
            const bool isSelected = (currentCommandIdx == i);

            DrawCommand* cmd = debugCanvas->getDrawCommandAt(i);
            const char* commandName = DrawCommand::GetCommandString(cmd->getOpType());

            if (ImGui::Selectable(commandName, isSelected)) {
                currentCommandIdx = i;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (currentCommandIdx >= 0 && currentCommandIdx < debugCanvas->getSize()) {
        ImGui::Separator();
        ImGui::Text("JSON for Command #%d:", currentCommandIdx);

        DrawCommand* cmd = debugCanvas->getDrawCommandAt(currentCommandIdx);

        SkDynamicMemoryWStream stream;
        SkJSONWriter   writer(&stream, ToolUtils::default_serial_procs(), SkJSONWriter::Mode::kPretty);
        UrlDataManager urlDataManager(SkString("data"));
        writer.beginObject();
        cmd->toJSON(writer, urlDataManager);
        writer.endObject();

        writer.flush();

        stream.write8(0);
        sk_sp<SkData> data = stream.detachAsData();
        char* dataString = const_cast<char*>(static_cast<const char*>(data->data()));

        ImGui::InputTextMultiline("###json_output",
                                  dataString,
                                  data->size(),
                                  ImVec2(-1, -1),
                                  ImGuiInputTextFlags_ReadOnly);
    }
    ImGui::End();
}

void drawMetadata(SkCapture* capture,
                  int& currentPictureIndex,
                  int& currentSubmissionIndex,
                  int& currentSubPictureIndex,
                  bool& invalidate) {
    ImGui::Begin("SkCapture Explorer");

    if (ImGui::BeginTabBar("ExplorerTabBar")) {
        // Tab 1: Timeline
        if (ImGui::BeginTabItem("Submission Timeline")) {
            ImGui::Text("Total Submissions: %u", capture->getMetadata().numRecordingCaptures);
            ImGui::Separator();

            // List of submissions
            ImGui::BeginChild("SubmissionList", ImVec2(0, 150), true);
            for (int i = 0; i < (int)capture->getMetadata().numRecordingCaptures; ++i) {
                const auto* rec = capture->getRecordingCapture(i);
                if (!rec) continue;
                char label[128];
                sprintf(label, "Submission #%d", i);
                const bool isSelected = (currentSubmissionIndex == i);
                if (ImGui::Selectable(label, isSelected)) {
                    if (currentSubmissionIndex != i) {
                        currentSubmissionIndex = i;
                        currentSubPictureIndex = 0; // reset sub-picture selection
                        invalidate = true;
                    }
                }
            }
            ImGui::EndChild();

            // Show selected submission details
            const auto* activeRec = capture->getRecordingCapture(currentSubmissionIndex);
            if (activeRec) {
                ImGui::Separator();
                ImGui::Text("DrawTasks in this Recording (%d):", activeRec->fDrawTasks.size());

                ImGui::BeginChild("SubPictureList", ImVec2(0, 100), true);
                for (int j = 0; j < (int)activeRec->fDrawTasks.size(); ++j) {
                    const auto& task = activeRec->fDrawTasks[j];
                    uint32_t assetIdx = task.fAssetIndex;
                    auto pic = capture->getAsset(assetIdx);
                    char label[128];
                    if (pic) {
                        sprintf(label, "Task #%d: Asset Index %u (%dx%d)",
                                j, assetIdx, (int)pic->cullRect().width(), (int)pic->cullRect().height());
                    } else {
                        sprintf(label, "Task #%d: Asset Index %u (Null)", j, assetIdx);
                    }
                    const bool isSelected = (currentSubPictureIndex == j);
                    if (ImGui::Selectable(label, isSelected)) {
                        if (currentSubPictureIndex != j) {
                            currentSubPictureIndex = j;
                            invalidate = true;
                        }
                    }
                }
                ImGui::EndChild();

                // Update currentPictureIndex to point to the selected sub-picture's asset index
                if (currentSubPictureIndex >= 0 && currentSubPictureIndex < (int)activeRec->fDrawTasks.size()) {
                    currentPictureIndex = activeRec->fDrawTasks[currentSubPictureIndex].fAssetIndex;
                }
            }
            ImGui::EndTabItem();
        }

        // Tab 2: Raw Catalog
        if (ImGui::BeginTabItem("Flat Asset Catalog")) {
            ImGui::Text("Total Picture Assets: %u", capture->getMetadata().numAssets);
            ImGui::Separator();
            ImGui::BeginChild("FlatCatalogList", ImVec2(0, 0), true);
            for (int i = 0; i < (int)capture->getMetadata().numAssets; ++i) {
                auto pic = capture->getAsset(i);
                char label[128];
                if (pic) {
                    sprintf(label, "Asset Index %d (%dx%d)",
                            i, (int)pic->cullRect().width(), (int)pic->cullRect().height());
                } else {
                    sprintf(label, "Asset Index %d (Null)", i);
                }
                const bool isSelected = (currentPictureIndex == i);
                if (ImGui::Selectable(label, isSelected)) {
                    if (currentPictureIndex != i) {
                        currentPictureIndex = i;
                        invalidate = true;
                    }
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void CaptureSlide::draw(SkCanvas* canvas) {
    if (!fCapture) {
        return;
    }

    drawMetadata(fCapture.get(),
                 fCurrentPictureIndex,
                 fCurrentSubmissionIndex,
                 fCurrentSubPictureIndex,
                 fInvalidate);

    auto focusPicture = fCapture->getAsset(fCurrentPictureIndex);
    if (!focusPicture) {
        return;
    }

    auto bounds = focusPicture->cullRect().roundOut();
    DebugCanvas debugCanvas(bounds.width(), bounds.height());
    focusPicture->playback(&debugCanvas);

    canvas->save();
    canvas->clipIRect(bounds, SkClipOp::kIntersect);
    canvas->drawPicture(focusPicture);
    canvas->restore();

    drawCommandHistory(&debugCanvas);
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
        case 'N': {
            int numSubs = fMetadata.numRecordingCaptures;
            if (numSubs > 0) {
                fCurrentSubmissionIndex = (fCurrentSubmissionIndex + 1) % numSubs;
                fCurrentSubPictureIndex = 0;
                fInvalidate = true;
            }
            return true;
        }
        case 'P': {
            int numSubs = fMetadata.numRecordingCaptures;
            if (numSubs > 0) {
                fCurrentSubmissionIndex = (fCurrentSubmissionIndex + numSubs - 1) % numSubs;
                fCurrentSubPictureIndex = 0;
                fInvalidate = true;
            }
            return true;
        }
        case 'n': {
            const auto* activeRec = fCapture->getRecordingCapture(fCurrentSubmissionIndex);
            if (activeRec && activeRec->fDrawTasks.size() > 0) {
                int numPics = activeRec->fDrawTasks.size();
                fCurrentSubPictureIndex = (fCurrentSubPictureIndex + 1) % numPics;
                fInvalidate = true;
            }
            return true;
        }
        case 'p': {
            const auto* activeRec = fCapture->getRecordingCapture(fCurrentSubmissionIndex);
            if (activeRec && activeRec->fDrawTasks.size() > 0) {
                int numPics = activeRec->fDrawTasks.size();
                fCurrentSubPictureIndex = (fCurrentSubPictureIndex + numPics - 1) % numPics;
                fInvalidate = true;
            }
            return true;
        }
        case 'F':
            ImGui::GetIO().FontGlobalScale = ImGui::GetIO().FontGlobalScale + 0.1f;
            return true;
        case 'S':
            ImGui::GetIO().FontGlobalScale = ImGui::GetIO().FontGlobalScale - 0.1f;
            return true;
    }

    return Slide::onChar(c);
}
