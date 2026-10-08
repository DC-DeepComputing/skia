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
#include "include/core/SkString.h"
#include "include/private/SkDebug.h"
#include "include/private/SkLog.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSamplingOptions.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "tools/skui/InputState.h"
#include "src/capture/SkCapture.h"
#include "src/capture/SkCaptureDrawId.h"
#include "src/core/SkRecord.h"
#include "src/utils/SkJSONWriter.h"
#include "tools/ProcsUtils.h"
#include "tools/UrlDataManager.h"
#include "tools/debugger/DebugCanvas.h"
#include "tools/debugger/DrawCommand.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <unordered_map>
#include <string>

const char* RecordTypeNames[] = {
#define CASE(T) #T,
        SK_RECORD_TYPES(CASE)
#undef CASE
};

const int JSON_TEXT_BOX_HEIGHT = 200;
// A width of -1 means use all available width
const int JSON_TEXT_BOX_WIDTH = -1;

const float EXPLORER_DEFAULT_WIDTH = 450.0f;
const float EXPLORER_DEFAULT_HEIGHT = 500.0f;
const float EXPLORER_DEFAULT_POS_X = 20.0f;
const float EXPLORER_DEFAULT_POS_Y = 20.0f;

CaptureSlide::CaptureSlide(const SkString& name, const SkString& path) : fCapturePath(path.c_str()) {
    auto data = SkData::MakeFromFileName(path.c_str());
    fCapture = SkCapture::MakeFromData(data);
    if (fCapture) {
        fMetadata = fCapture->getMetadata();
        this->updateActiveAssetIndex();
    } else {
        SKIA_LOG_E("Couldn't load capture %s", path.c_str());
    }

    // A Dawn WebGPU-on-WebGPU capture may have been written next to the SkCapture, e.g.
    // foo.capt + foo.capt.dawn_commands (see example/external_client/src/graphite_dawn_capture.cpp).
    fWebGPUView = WebGPUCaptureView::MakeFromFile(std::string(path.c_str()) + ".dawn_commands");
    if (fWebGPUView) {
        SKIA_LOG_I("Loaded WebGPU capture commands %s: %zu root commands, %zu GPU objects",
                   fWebGPUView->path().c_str(),
                   fWebGPUView->numRootCommands(),
                   fWebGPUView->numObjects());
        if (!fWebGPUView->error().empty()) {
            SKIA_LOG_E("Error parsing WebGPU capture: %s", fWebGPUView->error().c_str());
        }
    }
}

CaptureSlide::~CaptureSlide() {}

namespace {

// Finds the SkCapture draw id token annotation (SkCaptureDrawId::kAnnotationKey) of an asset.
class DrawIdTokenFinder final : public SkNoDrawCanvas {
public:
    DrawIdTokenFinder(int w, int h) : SkNoDrawCanvas(w, h) {}
    uint32_t fToken = 0;

protected:
    void onDrawAnnotation(const SkRect&, const char key[], SkData* value) override {
        if (!fToken && key && strcmp(key, SkCaptureDrawId::kAnnotationKey) == 0 && value &&
            value->size() == sizeof(uint32_t)) {
            memcpy(&fToken, value->data(), sizeof(uint32_t));
        }
    }
};

// Whether a DebugCanvas command corresponds to an SkCaptureCanvas draw call, i.e. one that
// advances the capture draw ordinal (see SkCaptureCanvas: all onDraw* except onDrawAnnotation).
bool is_capture_draw(DrawCommand::OpType type) {
    switch (type) {
        case DrawCommand::kClear_OpType:
        case DrawCommand::kDrawBitmap_OpType:
        case DrawCommand::kDrawBitmapRect_OpType:
        case DrawCommand::kDrawDRRect_OpType:
        case DrawCommand::kDrawImage_OpType:
        case DrawCommand::kDrawImageLattice_OpType:
        case DrawCommand::kDrawImageRect_OpType:
        case DrawCommand::kDrawImageRectLayer_OpType:
        case DrawCommand::kDrawOval_OpType:
        case DrawCommand::kDrawArc_OpType:
        case DrawCommand::kDrawPaint_OpType:
        case DrawCommand::kDrawPatch_OpType:
        case DrawCommand::kDrawPath_OpType:
        case DrawCommand::kDrawPoints_OpType:
        case DrawCommand::kDrawRect_OpType:
        case DrawCommand::kDrawRRect_OpType:
        case DrawCommand::kDrawRegion_OpType:
        case DrawCommand::kDrawShadow_OpType:
        case DrawCommand::kDrawTextBlob_OpType:
        case DrawCommand::kDrawVertices_OpType:
        case DrawCommand::kDrawAtlas_OpType:
        case DrawCommand::kDrawDrawable_OpType:
        case DrawCommand::kDrawEdgeAAQuad_OpType:
        case DrawCommand::kDrawEdgeAAImageSet_OpType:
            return true;
        default:
            return false;
    }
}

}  // namespace

void CaptureSlide::computeDrawIds(const SkPicture* picture) {
    fAssetToken = 0;
    fCommandOrdinals.clear();
    if (!picture || !fDebugCanvas) {
        return;
    }
    SkIRect bounds = picture->cullRect().roundOut();
    DrawIdTokenFinder finder(bounds.width(), bounds.height());
    picture->playback(&finder);
    fAssetToken = finder.fToken;

    fCommandOrdinals.resize(fDebugCanvas->getSize(), -1);
    int ordinal = 0;
    int pictureDepth = 0;  // nested DrawPicture is a single capture draw
    for (int i = 0; i < fDebugCanvas->getSize(); ++i) {
        DrawCommand::OpType type = fDebugCanvas->getDrawCommandAt(i)->getOpType();
        if (type == DrawCommand::kBeginDrawPicture_OpType) {
            if (pictureDepth++ == 0) {
                fCommandOrdinals[i] = ordinal++;
            }
        } else if (type == DrawCommand::kEndDrawPicture_OpType) {
            pictureDepth = std::max(0, pictureDepth - 1);
        } else if (pictureDepth == 0 && is_capture_draw(type)) {
            fCommandOrdinals[i] = ordinal++;
        }
    }
}

void CaptureSlide::drawCommandHistory() {
    const DebugCanvas* debugCanvas = fDebugCanvas.get();
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Appearing);
    if (!ImGui::Begin("Command History")) {
        ImGui::End();
        return;
    }

    ImGui::Text("Total Commands: %d", debugCanvas->getSize());
    const bool tracked = fAssetToken != 0;
    if (tracked) {
        ImGui::SameLine();
        ImGui::Text("  draw id token %u%s", fAssetToken,
                    fWebGPUView && fWebGPUView->hasDrawIds() ? "" : " (no GPU markers loaded)");
        ImGui::TextDisabled("Click a draw to select it; its GPU draws are highlighted in "
                            "WebGPU Commands.");
    }
    ImGui::Separator();

    // Scroll to the selection if it was changed from another window.
    const bool externalSelection = !(fSelectedDraw == fLastHistorySelection);
    fLastHistorySelection = fSelectedDraw;

    ImGui::BeginChild("CommandList", ImVec2(0, 0), true);
    for (int i = 0; i < debugCanvas->getSize(); ++i) {
        DrawCommand* cmd = debugCanvas->getDrawCommandAt(i);
        const char* commandName = DrawCommand::GetCommandString(cmd->getOpType());
        const int ordinal = i < (int)fCommandOrdinals.size() ? fCommandOrdinals[i] : -1;
        const WebGPUCaptureView::DrawRef ref{fAssetToken, (uint32_t)std::max(ordinal, 0)};
        const bool isSelected = tracked && ordinal >= 0 && ref == fSelectedDraw;

        SkString label = SkStringPrintf("%d: %s", i, commandName);
        if (tracked && ordinal >= 0) {
            const std::vector<int>* gpuDraws = fWebGPUView ? fWebGPUView->gpuDrawsFor(ref)
                                                           : nullptr;
            label.appendf("   [draw %d -> %zu GPU draw%s]", ordinal,
                          gpuDraws ? gpuDraws->size() : 0,
                          gpuDraws && gpuDraws->size() == 1 ? "" : "s");
        }

        ImGui::PushID(i);
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.85f, 0.2f, 1.f));
            if (externalSelection) {
                ImGui::SetScrollHereY();
            }
        }
        const bool open = ImGui::TreeNode(commandName, "%s", label.c_str());
        if (isSelected) {
            ImGui::PopStyleColor();
        }
        if (tracked && ordinal >= 0 && ImGui::IsItemClicked()) {
            fSelectedDraw = ref;
            fLastHistorySelection = ref;
        }
        if (open) {
            SkDynamicMemoryWStream stream;
            SkJSONWriter writer(
                    &stream, ToolUtils::default_serial_procs(), SkJSONWriter::Mode::kPretty);
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
                                      ImVec2(JSON_TEXT_BOX_WIDTH, JSON_TEXT_BOX_HEIGHT),
                                      ImGuiInputTextFlags_ReadOnly);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::End();
}

void drawRecordingTimelineTab(SkCapture* capture,
                              int& currentAssetIndex,
                              int& currentRecordingCaptureIndex,
                              int& currentDrawTaskIndex,
                              bool& invalidate) {
    ImGui::Text("Total Recordings: %u", capture->getMetadata().numRecordingCaptures);
    ImGui::Separator();

    ImGui::Columns(2, "TimelineSplit", true);

    // Column 0: Recordings List
    ImGui::Text("Recordings:");
    ImGui::BeginChild("RecordingList", ImVec2(0, 300), true);
    for (int i = 0; i < (int)capture->getMetadata().numRecordingCaptures; ++i) {
        const auto* rec = capture->getRecordingCapture(i);
        if (!rec) continue;
        SkString label = SkStringPrintf("Recording #%d", i);
        const bool isSelected = (currentRecordingCaptureIndex == i);
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            if (currentRecordingCaptureIndex != i) {
                currentRecordingCaptureIndex = i;
                currentDrawTaskIndex = 0; // reset draw task selection
                invalidate = true;
            }
        }
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    // Column 1: DrawTasks List
    const auto* activeRec = capture->getRecordingCapture(currentRecordingCaptureIndex);
    if (activeRec) {
        ImGui::Text("DrawTasks (%d):", (int)activeRec->fDrawTasks.size());
        ImGui::BeginChild("DrawTaskList", ImVec2(0, 300), true);
        for (int j = 0; j < (int)activeRec->fDrawTasks.size(); ++j) {
            const auto& task = activeRec->fDrawTasks[j];
            uint32_t assetIdx = task.fAssetIndex;
            auto pic = capture->getAsset(assetIdx);
            SkString label;
            if (pic) {
                label = SkStringPrintf("Task #%d: Asset %u (%dx%d)",
                                       j, assetIdx, (int)pic->cullRect().width(), (int)pic->cullRect().height());
            } else {
                label = SkStringPrintf("Task #%d: Asset %u (Null)", j, assetIdx);
            }
            const bool isSelected = (currentDrawTaskIndex == j);
            if (ImGui::Selectable(label.c_str(), isSelected)) {
                if (currentDrawTaskIndex != j) {
                    currentDrawTaskIndex = j;
                    invalidate = true;
                }
            }
        }
        ImGui::EndChild();

        // Update currentAssetIndex to point to the selected sub-picture's asset index
        if (currentDrawTaskIndex >= 0 && currentDrawTaskIndex < (int)activeRec->fDrawTasks.size()) {
            currentAssetIndex = activeRec->fDrawTasks[currentDrawTaskIndex].fAssetIndex;
        } else {
            currentAssetIndex = -1;
        }
    } else {
        ImGui::Text("No active recording selected.");
    }

    ImGui::Columns(1); // Reset back to single column
}

void drawFlatAssetCatalogTab(SkCapture* capture,
                             int& currentAssetIndex,
                             bool& invalidate) {
    ImGui::Text("Total Picture Assets: %u", capture->getMetadata().numAssets);
    ImGui::Separator();
    ImGui::BeginChild("FlatCatalogList", ImVec2(0, 0), true);
    for (int i = 0; i < (int)capture->getMetadata().numAssets; ++i) {
        auto pic = capture->getAsset(i);
        SkString label;
        if (pic) {
            label = SkStringPrintf("Asset Index %d (%dx%d)",
                                   i, (int)pic->cullRect().width(), (int)pic->cullRect().height());
        } else {
            label = SkStringPrintf("Asset Index %d (Null)", i);
        }
        const bool isSelected = (currentAssetIndex == i);
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            if (currentAssetIndex != i) {
                currentAssetIndex = i;
                invalidate = true;
            }
        }
    }
    ImGui::EndChild();
}

void drawCaptureExplorer(SkCapture* capture,
                         int& currentAssetIndex,
                         int& currentRecordingCaptureIndex,
                         int& currentDrawTaskIndex,
                         bool& invalidate) {
    ImGui::SetNextWindowSize(ImVec2(EXPLORER_DEFAULT_WIDTH, EXPLORER_DEFAULT_HEIGHT),
                             ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(EXPLORER_DEFAULT_POS_X, EXPLORER_DEFAULT_POS_Y),
                            ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("SkCapture Explorer")) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("ExplorerTabBar")) {
        if (ImGui::BeginTabItem("Recording Timeline")) {
            drawRecordingTimelineTab(capture,
                                     currentAssetIndex,
                                     currentRecordingCaptureIndex,
                                     currentDrawTaskIndex,
                                     invalidate);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Flat Asset Catalog")) {
            drawFlatAssetCatalogTab(capture, currentAssetIndex, invalidate);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void CaptureSlide::draw(SkCanvas* canvas) {
    if (fWebGPUView) {
        fWebGPUView->draw(&fSelectedDraw);
        this->drawDrawIdWindow();
    }

    if (!fCapture) {
        return;
    }

    // drawCaptureExplorer may update fCurrentAssetIndex if the user clicks on a different asset.
    // If so, fDebugCanvas must be reset with the newly focused draw commands.
    int prevAssetIndex = fCurrentAssetIndex;
    drawCaptureExplorer(fCapture.get(),
                        fCurrentAssetIndex,
                        fCurrentRecordingCaptureIndex,
                        fCurrentDrawTaskIndex,
                        fInvalidate);
    if (prevAssetIndex != fCurrentAssetIndex) {
        fDebugCanvas.reset();
    }

    auto focusPicture = fCapture->getAsset(fCurrentAssetIndex);
    if (!focusPicture) {
        // TODO handle this case with an elegant UI popup.
        return;
    }

    auto bounds = focusPicture->cullRect().roundOut();

    if (!fDebugCanvas) {
        fDebugCanvas = std::make_unique<DebugCanvas>(bounds.width(), bounds.height());
        focusPicture->playback(fDebugCanvas.get());
        this->computeDrawIds(focusPicture.get());
    }

    // The GPU views show the replayed pass on a cleared canvas, aligned 1:1 with the pass pixels.
    const bool passLoaded = fLoadedDepthPass >= 0;
    if (fCanvasView == CanvasView::kGpuColor && passLoaded) {
        canvas->clear(SK_ColorWHITE);
        if (fPassColor) {
            canvas->drawImage(fPassColor, 0, 0);
        }
    } else if (fCanvasView == CanvasView::kDrawIds && passLoaded) {
        canvas->clear(SK_ColorWHITE);
        if (fIdImageDirty) {
            this->rebuildIdImage();
        }
        if (fIdImage) {
            canvas->drawImage(fIdImage, 0, 0);
        }
    } else {
        canvas->save();
        canvas->clipIRect(bounds, SkClipOp::kIntersect);
        canvas->drawPicture(focusPicture);
        canvas->restore();
    }

    // Highlight the selected skia draw by dimming everything else.
    if (fHighlightSelected && passLoaded) {
        const int dimGpuDraw = fStepMode && fStepBy == StepBy::kGpuDraw ? fStepLastGpuDraw : -1;
        if (fDimImageDirty || !(fDimSelection == fSelectedDraw) || fDimGpuDraw != dimGpuDraw) {
            this->rebuildDimImage();
        }
        if (fDimImage) {
            canvas->drawImage(fDimImage, 0, 0);
        }
    }

    this->drawCommandHistory();
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
                fCurrentRecordingCaptureIndex = (fCurrentRecordingCaptureIndex + 1) % numSubs;
                fCurrentDrawTaskIndex = 0;
                this->updateActiveAssetIndex();
                fInvalidate = true;
                fDebugCanvas.reset();
            }
            return true;
        }
        case 'P': {
            int numSubs = fMetadata.numRecordingCaptures;
            if (numSubs > 0) {
                fCurrentRecordingCaptureIndex = (fCurrentRecordingCaptureIndex + numSubs - 1)
                    % numSubs;
                fCurrentDrawTaskIndex = 0;
                this->updateActiveAssetIndex();
                fInvalidate = true;
                fDebugCanvas.reset();
            }
            return true;
        }
        case 'n': {
            const auto* activeRec = fCapture->getRecordingCapture(fCurrentRecordingCaptureIndex);
            if (activeRec && activeRec->fDrawTasks.size() > 0) {
                int numPics = activeRec->fDrawTasks.size();
                fCurrentDrawTaskIndex = (fCurrentDrawTaskIndex + 1) % numPics;
                this->updateActiveAssetIndex();
                fInvalidate = true;
                fDebugCanvas.reset();
            }
            return true;
        }
        case 'p': {
            const auto* activeRec = fCapture->getRecordingCapture(fCurrentRecordingCaptureIndex);
            if (activeRec && activeRec->fDrawTasks.size() > 0) {
                int numPics = activeRec->fDrawTasks.size();
                fCurrentDrawTaskIndex = (fCurrentDrawTaskIndex + numPics - 1) % numPics;
                this->updateActiveAssetIndex();
                fInvalidate = true;
                fDebugCanvas.reset();
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

void CaptureSlide::updateActiveAssetIndex() {
    if (!fCapture) {
        return;
    }
    const auto* activeRec = fCapture->getRecordingCapture(fCurrentRecordingCaptureIndex);
    if (activeRec && !activeRec->fDrawTasks.empty()) {
        fCurrentDrawTaskIndex = SkTPin(fCurrentDrawTaskIndex, 0, activeRec->fDrawTasks.size() - 1);
        fCurrentAssetIndex = activeRec->fDrawTasks[fCurrentDrawTaskIndex].fAssetIndex;
    } else {
        fCurrentAssetIndex = -1;
    }
}

// ---- Draw ID (depth) view ----------------------------------------------------------------------

namespace {

SkColor color_for_draw(uint32_t ordinal) {
    // Golden-ratio hue spacing so neighboring draws get distinct colors.
    float hue = std::fmod(ordinal * 0.618033988f, 1.f) * 360.f;
    SkScalar hsv[3] = {hue, 0.75f, 1.f};
    return SkHSVToColor(hsv);
}

}  // namespace

const WebGPUCaptureView::PassDepthEntry* CaptureSlide::entryAt(int x, int y,
                                                               uint32_t* depthBits) const {
    if (!fWebGPUView || fLoadedDepthPass < 0 || x < 0 || y < 0 || x >= (int)fDepthWidth ||
        y >= (int)fDepthHeight) {
        return nullptr;
    }
    const uint32_t bits = fDepthBits[size_t(y) * fDepthWidth + x];
    if (depthBits) {
        *depthBits = bits;
    }
    const auto& table = fWebGPUView->passes()[fLoadedDepthPass].fDepthToDraw;
    auto it = table.find(bits);
    return it == table.end() ? nullptr : &it->second;
}

bool CaptureSlide::ensureReplay() {
    if (!fReplayAttempted) {
        fReplayAttempted = true;
        fReplay = WebGPUCaptureReplay::Make(fWebGPUView->path(),
                                            fCapturePath + ".dawn_content",
                                            &fReplayError);
        if (fReplay) {
            SKIA_LOG_I("Replayed WebGPU capture %s", fWebGPUView->path().c_str());
        } else {
            SKIA_LOG_E("WebGPU capture replay failed: %s", fReplayError.c_str());
        }
    }
    return fReplay != nullptr;
}

void CaptureSlide::loadPass(int passIndex) {
    fLoadedDepthPass = -1;
    fDepthBits.clear();
    fPassColor.reset();
    fDepthError.clear();
    fIdImageDirty = true;
    fDimImageDirty = true;
    if (!fWebGPUView || passIndex < 0 || passIndex >= (int)fWebGPUView->passes().size()) {
        return;
    }
    const auto& pass = fWebGPUView->passes()[passIndex];
    if (!pass.fDepthTextureId) {
        fDepthError = "This render pass has no depth attachment";
        return;
    }
    if (!this->ensureReplay()) {
        fDepthError = "Replay failed: " + fReplayError;
        return;
    }
    WebGPUCaptureReplay::DepthImage depth;
    if (!fReplay->readDepth(pass.fDepthTextureId, &depth, &fDepthError)) {
        return;
    }
    fDepthWidth = depth.fWidth;
    fDepthHeight = depth.fHeight;
    fDepthBits.resize(depth.fDepth.size());
    for (size_t i = 0; i < depth.fDepth.size(); ++i) {
        // Graphite writes z = 1 - PaintersDepth / 0xFFFF (DrawOrder::depthAsFloat()).
        fDepthBits[i] = (uint32_t)std::lround((1.0 - depth.fDepth[i]) * 0xFFFF);
    }
    fLoadedDepthPass = passIndex;

    // The pass color target, as left by the current replay.
    WebGPUCaptureReplay::ColorImage color;
    std::string error;
    if (!pass.fColorTextureId) {
        fDepthError = "The render pass has no color target";
    } else if (!fReplay->readColor(pass.fColorTextureId, &color, &error)) {
        fDepthError = "Color: " + error;
    } else {
        SkImageInfo info = SkImageInfo::Make(color.fWidth, color.fHeight,
                                             kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        fPassColor = SkImages::RasterFromData(
                info, SkData::MakeWithCopy(color.fRGBA.data(), color.fRGBA.size()),
                info.minRowBytes());
    }
    fInvalidate = true;
}

void CaptureSlide::setStepMode(bool enabled) {
    fStepMode = enabled;
    fStepSelection = {};
    fGpuStepReplayed = -1;
    fStepPass = -1;
    fStepLastGpuDraw = -1;
    fStepMessage.clear();
    if (!enabled && fReplay && fReplay->drawLimit() != WebGPUCaptureReplay::kNoDrawLimit) {
        // Back to the final state of the capture.
        std::string error;
        if (!fReplay->play(WebGPUCaptureReplay::kNoDrawLimit, &error)) {
            fDepthError = error;
        } else if (fLoadedDepthPass >= 0) {
            this->loadPass(fLoadedDepthPass);
        }
    }
    fInvalidate = true;
}

void CaptureSlide::stepTo(WebGPUCaptureView::DrawRef ref) {
    fStepSelection = ref;
    fStepMessage.clear();
    const std::vector<int>* gpuDraws = ref.isValid() ? fWebGPUView->gpuDrawsFor(ref) : nullptr;
    if (!gpuDraws || gpuDraws->empty()) {
        fStepMessage = ref.isValid() ? "The selected skia draw has no GPU draws (showing the "
                                       "previous step)"
                                     : "Select a skia draw to replay up to it";
        return;
    }
    const int lastGpuDraw = *std::max_element(gpuDraws->begin(), gpuDraws->end());
    const int passIndex = fWebGPUView->passOfGpuDraw(lastGpuDraw);
    if (passIndex < 0) {
        fStepMessage = "The selected skia draw's GPU draw is not in a render pass";
        return;
    }
    if (!this->ensureReplay()) {
        fStepMessage = "Replay failed: " + fReplayError;
        return;
    }
    std::string error;
    if (!fReplay->play((uint64_t)lastGpuDraw + 1, &error)) {
        fStepMessage = error;
        return;
    }
    fStepPass = passIndex;
    fStepLastGpuDraw = lastGpuDraw;
    fDrawIdPass = passIndex;
    this->loadPass(passIndex);  // color and depth after the same draws
}

void CaptureSlide::stepToGpuDraw(int gpuDrawIndex) {
    fGpuStepReplayed = gpuDrawIndex;
    fStepMessage.clear();
    if (!fWebGPUView->gpuDraw(gpuDrawIndex)) {
        fStepMessage = "No such GPU draw";
        return;
    }
    const int passIndex = fWebGPUView->passOfGpuDraw(gpuDrawIndex);
    if (passIndex < 0) {
        fStepMessage = "This GPU draw is in a render bundle (no render pass to show)";
        return;
    }
    if (!this->ensureReplay()) {
        fStepMessage = "Replay failed: " + fReplayError;
        return;
    }
    std::string error;
    if (!fReplay->play((uint64_t)gpuDrawIndex + 1, &error)) {
        fStepMessage = error;
        return;
    }
    fStepPass = passIndex;
    fStepLastGpuDraw = gpuDrawIndex;
    fDrawIdPass = passIndex;
    this->loadPass(passIndex);
}

std::vector<uint32_t> CaptureSlide::highlightDepths() const {
    std::vector<uint32_t> depths;
    if (fLoadedDepthPass < 0) {
        return depths;
    }
    if (fStepMode && fStepBy == StepBy::kGpuDraw) {
        if (fWebGPUView->passOfGpuDraw(fStepLastGpuDraw) == fLoadedDepthPass) {
            if (const auto* node = fWebGPUView->gpuDraw(fStepLastGpuDraw)) {
                for (const auto& tag : node->fTags) {
                    depths.push_back(tag.fDepth);
                }
            }
        }
        return depths;
    }
    for (const auto& [bits, entry] : fWebGPUView->passes()[fLoadedDepthPass].fDepthToDraw) {
        if (entry.fTag.fRef.isValid() && entry.fTag.fRef == fSelectedDraw) {
            depths.push_back(bits);
        }
    }
    return depths;
}

void CaptureSlide::moveSelection(int count) {
    const bool forward = count > 0;
    WebGPUCaptureView::DrawRef ref = fSelectedDraw;
    for (int i = 0; i < std::abs(count); ++i) {
        WebGPUCaptureView::DrawRef next = fWebGPUView->adjacentDraw(ref, forward);
        if (!next.isValid()) {
            break;  // clamp at the first / last draw
        }
        ref = next;
    }
    if (ref.isValid()) {
        fSelectedDraw = ref;
        fInvalidate = true;
    }
}

void CaptureSlide::rebuildIdImage() {
    fIdImageDirty = false;
    fIdImage.reset();
    if (fLoadedDepthPass < 0 || fDepthBits.empty()) {
        return;
    }
    const auto& pass = fWebGPUView->passes()[fLoadedDepthPass];
    // Only cover the pass's color target (the depth attachment may be larger, e.g. approx-fit).
    const int w = pass.fWidth ? std::min<int>(pass.fWidth, fDepthWidth) : fDepthWidth;
    const int h = pass.fHeight ? std::min<int>(pass.fHeight, fDepthHeight) : fDepthHeight;

    std::unordered_map<uint32_t, SkPMColor> colors;
    colors[0] = 0;  // cleared depth: nothing drawn (transparent, shows the white background)
    for (const auto& [bits, entry] : pass.fDepthToDraw) {
        colors[bits] = SkPreMultiplyColor(entry.fTag.fRef.isValid()
                                                  ? color_for_draw(entry.fTag.fRef.fOrdinal)
                                                  : SkColorSetRGB(128, 128, 128));  // untracked
    }
    const SkPMColor unknown = SkPreMultiplyColor(SK_ColorMAGENTA);  // depth without a marker

    SkBitmap bm;
    bm.allocN32Pixels(w, h);
    for (int y = 0; y < h; ++y) {
        SkPMColor* row = bm.getAddr32(0, y);
        for (int x = 0; x < w; ++x) {
            auto it = colors.find(fDepthBits[size_t(y) * fDepthWidth + x]);
            row[x] = it != colors.end() ? it->second : unknown;
        }
    }
    bm.setImmutable();
    fIdImage = bm.asImage();
}

void CaptureSlide::rebuildDimImage() {
    fDimImageDirty = false;
    fDimSelection = fSelectedDraw;
    fDimGpuDraw = fStepMode && fStepBy == StepBy::kGpuDraw ? fStepLastGpuDraw : -1;
    fDimImage.reset();
    if (fLoadedDepthPass < 0 || fDepthBits.empty()) {
        return;
    }
    const auto& pass = fWebGPUView->passes()[fLoadedDepthPass];
    const int w = pass.fWidth ? std::min<int>(pass.fWidth, fDepthWidth) : fDepthWidth;
    const int h = pass.fHeight ? std::min<int>(pass.fHeight, fDepthHeight) : fDepthHeight;

    // Depth values to keep bright (a skia draw can have several, one per RenderStep; a GPU draw
    // has one per batched skia draw).
    const std::vector<uint32_t> selected = this->highlightDepths();
    const SkPMColor dim = SkPreMultiplyColor(SkColorSetARGB(190, 0, 0, 0));
    SkBitmap bm;
    bm.allocN32Pixels(w, h);
    for (int y = 0; y < h; ++y) {
        SkPMColor* row = bm.getAddr32(0, y);
        for (int x = 0; x < w; ++x) {
            const uint32_t bits = fDepthBits[size_t(y) * fDepthWidth + x];
            const bool isSelected =
                    std::find(selected.begin(), selected.end(), bits) != selected.end();
            row[x] = isSelected ? 0 : dim;
        }
    }
    bm.setImmutable();
    fDimImage = bm.asImage();
}

bool CaptureSlide::onMouse(SkScalar x, SkScalar y, skui::InputState state, skui::ModifierKey) {
    if (fLoadedDepthPass < 0) {
        return false;
    }
    SkIPoint px = {(int)std::floor(x), (int)std::floor(y)};
    if (px != fHoverPixel) {
        fHoverPixel = px;
        fInvalidate = true;
    }
    if (state == skui::InputState::kDown && fCanvasView != CanvasView::kSkiaPlayback) {
        if (const auto* entry = this->entryAt(px.x(), px.y(), nullptr)) {
            if (entry->fTag.fRef.isValid()) {
                fSelectedDraw = entry->fTag.fRef;
                fInvalidate = true;
                return true;  // consume the click (don't start a pan)
            }
        }
    }
    return false;
}

void CaptureSlide::drawDrawIdWindow() {
    const auto& passes = fWebGPUView->passes();
    ImGui::SetNextWindowSize(ImVec2(560, 460), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Draw ID (depth)")) {
        ImGui::End();
        return;
    }
    if (passes.empty()) {
        ImGui::TextDisabled("The WebGPU capture has no render passes.");
        ImGui::End();
        return;
    }
    if (!fWebGPUView->hasDrawIds()) {
        ImGui::TextDisabled("No skia draw id markers: capture with --track-draw-ids.");
    }

    if (fDrawIdPass < 0) {
        // Default to the pass with the most GPU draws that kept its depth.
        int best = 0;
        for (int i = 0; i < (int)passes.size(); ++i) {
            const bool stored = passes[i].fDepthStored && passes[i].fDepthBindable;
            const bool bestStored = passes[best].fDepthStored && passes[best].fDepthBindable;
            if ((stored && !bestStored) ||
                (stored == bestStored && passes[i].fNumGpuDraws > passes[best].fNumGpuDraws)) {
                best = i;
            }
        }
        fDrawIdPass = best;
    }

    // ---- 1. Replay -------------------------------------------------------------------------
    auto passLabel = [](const WebGPUCaptureView::RenderPassInfo& p) {
        return SkStringPrintf("#%d %ux%u, %d GPU draws, color=%s, depth=%s%s", p.fIndex, p.fWidth,
                              p.fHeight, p.fNumGpuDraws, p.fColorTarget.c_str(),
                              p.fDepthTextureLabel.empty() ? "<none>"
                                                           : p.fDepthTextureLabel.c_str(),
                              p.fDepthTextureId && !(p.fDepthStored && p.fDepthBindable)
                                      ? " (not kept)"
                                      : "");
    };
    SkString current = passLabel(passes[fDrawIdPass]);
    if (ImGui::BeginCombo("Render pass", current.c_str())) {
        for (int i = 0; i < (int)passes.size(); ++i) {
            SkString label = passLabel(passes[i]);
            if (ImGui::Selectable(label.c_str(), i == fDrawIdPass)) {
                fDrawIdPass = i;
                if (fReplay) {
                    this->loadPass(i);  // replay already done: switching is cheap
                }
            }
        }
        ImGui::EndCombo();
    }
    if (fLoadedDepthPass != fDrawIdPass) {
        if (ImGui::Button(fReplay ? "Read pass" : "Replay capture")) {
            this->loadPass(fDrawIdPass);
            if (fLoadedDepthPass >= 0 && fCanvasView == CanvasView::kSkiaPlayback) {
                fCanvasView = CanvasView::kGpuColor;
            }
        }
    }
    if (!fDepthError.empty()) {
        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", fDepthError.c_str());
    }
    if (fReplay && !fReplay->deviceErrors().empty()) {
        ImGui::TextColored(ImVec4(1, 0.6f, 0.2f, 1), "Replay device errors: %zu (first: %s)",
                           fReplay->deviceErrors().size(),
                           fReplay->deviceErrors().front().c_str());
    }
    ImGui::Separator();

    // ---- 2. Canvas view ----------------------------------------------------------------------
    ImGui::Text("Canvas:");
    ImGui::SameLine();
    auto viewRadio = [this](const char* label, CanvasView view, bool needsPass) {
        if (needsPass && fLoadedDepthPass < 0) {
            ImGui::BeginDisabled();
        }
        if (ImGui::RadioButton(label, fCanvasView == view)) {
            fCanvasView = view;
            fInvalidate = true;
        }
        if (needsPass && fLoadedDepthPass < 0) {
            ImGui::EndDisabled();
        }
    };
    viewRadio("Skia playback", CanvasView::kSkiaPlayback, false);
    ImGui::SameLine();
    viewRadio("GPU color", CanvasView::kGpuColor, true);
    ImGui::SameLine();
    viewRadio("Draw ID colors", CanvasView::kDrawIds, true);
    if (fLoadedDepthPass < 0) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Checkbox("Highlight selected skia draw", &fHighlightSelected)) {
        fInvalidate = true;
    }
    if (fLoadedDepthPass < 0) {
        ImGui::EndDisabled();
    }
    ImGui::TextDisabled(
            "Skia playback: the SkCapture drawn by the viewer.\n"
            "GPU color: the pass color target from the replayed WebGPU capture.\n"
            "Draw ID colors: each pixel colored by the skia draw that last wrote its depth.\n"
            "Highlight: dims all pixels the selected skia draw did not write last.");
    ImGui::Separator();

    // ---- 3. Selection / stepping -----------------------------------------------------------
    bool stepMode = fStepMode;
    if (ImGui::Checkbox("Step: replay only part of the capture", &stepMode)) {
        this->setStepMode(stepMode);
        if (stepMode && fCanvasView == CanvasView::kSkiaPlayback) {
            fCanvasView = CanvasView::kGpuColor;
        }
    }
    const bool gpuOrder = fStepMode && fStepBy == StepBy::kGpuDraw;
    if (fStepMode) {
        ImGui::SameLine();
        ImGui::Text("by:");
        ImGui::SameLine();
        if (ImGui::RadioButton("Skia draw", fStepBy == StepBy::kSkiaDraw)) {
            fStepBy = StepBy::kSkiaDraw;
            fStepSelection = {};  // replay for the current selection
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("GPU draw (real GPU order)", fStepBy == StepBy::kGpuDraw)) {
            if (fStepBy != StepBy::kGpuDraw) {
                // Start from the selected skia draw's last GPU draw, if any.
                if (const auto* draws = fWebGPUView->gpuDrawsFor(fSelectedDraw)) {
                    fGpuStep = *std::max_element(draws->begin(), draws->end());
                }
                fGpuStepReplayed = -1;
            }
            fStepBy = StepBy::kGpuDraw;
        }
    }
    const int numGpuDraws = (int)fWebGPUView->numGpuDraws();
    const struct { const char* fLabel; int fCount; } kMoves[] = {
            {"<<100", -100}, {"<<10", -10}, {"< Prev", -1},
            {"Next >", 1},   {"10>>", 10},  {"100>>", 100},
    };
    for (size_t i = 0; i < std::size(kMoves); ++i) {
        if (i) {
            ImGui::SameLine();
        }
        if (ImGui::Button(kMoves[i].fLabel)) {
            if (gpuOrder) {
                fGpuStep = std::clamp(fGpuStep + kMoves[i].fCount, 0, std::max(numGpuDraws - 1, 0));
            } else {
                this->moveSelection(kMoves[i].fCount);
            }
        }
    }
    ImGui::SameLine();
    if (gpuOrder) {
        ImGui::Text("GPU draw %d / %d", fGpuStep, numGpuDraws);
    } else if (fSelectedDraw.isValid()) {
        ImGui::Text("skia draw %u", fSelectedDraw.fOrdinal);
    } else {
        ImGui::TextDisabled("no skia draw selected");
    }

    if (gpuOrder && numGpuDraws > 0) {
        // Scrub through GPU draws; the replay runs when the slider is released.
        ImGui::SliderInt("GPU draw", &fGpuStep, 0, numGpuDraws - 1);
        const bool dragging = ImGui::IsItemActive();
        if (fSelectedDraw.isValid()) {
            ImGui::SameLine();
            if (ImGui::Button("Go to selected skia draw")) {
                if (const auto* draws = fWebGPUView->gpuDrawsFor(fSelectedDraw)) {
                    fGpuStep = *std::max_element(draws->begin(), draws->end());
                }
            }
        }
        if (!dragging && fGpuStep != fGpuStepReplayed) {
            this->stepToGpuDraw(fGpuStep);
        }
        if (const auto* node = fWebGPUView->gpuDraw(fGpuStepReplayed)) {
            ImGui::TextWrapped("GPU draw %d (pass #%d): %s", fGpuStepReplayed,
                               fWebGPUView->passOfGpuDraw(fGpuStepReplayed), node->fText.c_str());
            if (node->fTags.empty()) {
                ImGui::TextDisabled("  no skia draw markers");
            }
            // The skia draws batched into this GPU draw; click to select one.
            for (size_t t = 0; t < node->fTags.size(); ++t) {
                const auto& tag = node->fTags[t];
                SkString label = tag.fRef.isValid()
                                         ? SkStringPrintf("  skia draw %u  %s%s  d=%u##tag%zu",
                                                          tag.fRef.fOrdinal, tag.fStep.c_str(),
                                                          tag.fDepthOnly ? " (clip)" : "",
                                                          tag.fDepth, t)
                                         : SkStringPrintf("  untracked  %s  d=%u##tag%zu",
                                                          tag.fStep.c_str(), tag.fDepth, t);
                if (ImGui::Selectable(label.c_str(),
                                      tag.fRef.isValid() && tag.fRef == fSelectedDraw)) {
                    if (tag.fRef.isValid()) {
                        fSelectedDraw = tag.fRef;
                    }
                }
            }
        }
    } else if (fStepMode) {
        if (!(fStepSelection == fSelectedDraw)) {
            this->stepTo(fSelectedDraw);
        }
        if (fStepPass >= 0) {
            ImGui::Text("Replayed GPU draws 0..%d of %d (pass #%d)", fStepLastGpuDraw,
                        numGpuDraws, fStepPass);
        }
    }
    if (fStepMode && !fStepMessage.empty()) {
        ImGui::TextColored(ImVec4(1, 0.6f, 0.2f, 1), "%s", fStepMessage.c_str());
    }
    ImGui::Separator();

    // ---- 4. Pixel inspector ------------------------------------------------------------------
    if (fLoadedDepthPass >= 0) {
        const auto& pass = passes[fLoadedDepthPass];
        ImGui::Text("Pass #%d: depth %ux%u (%s, %u sample%s), %zu tagged depth values",
                    fLoadedDepthPass, fDepthWidth, fDepthHeight, pass.fDepthFormat.c_str(),
                    pass.fDepthSampleCount, pass.fDepthSampleCount == 1 ? "" : "s, sample 0",
                    pass.fDepthToDraw.size());
        ImGui::TextDisabled("Hover the canvas to inspect a pixel; click to select its skia draw.");

        uint32_t bits = 0;
        const auto* entry = this->entryAt(fHoverPixel.x(), fHoverPixel.y(), &bits);
        if (fHoverPixel.x() >= 0 && fHoverPixel.x() < (int)fDepthWidth && fHoverPixel.y() >= 0 &&
            fHoverPixel.y() < (int)fDepthHeight) {
            ImGui::Text("Pixel (%d, %d): depth %u", fHoverPixel.x(), fHoverPixel.y(), bits);
            if (bits == 0) {
                ImGui::Text("  cleared (no draw wrote depth here)");
            } else if (!entry) {
                ImGui::Text("  no draw with this depth in the pass markers");
            } else {
                const auto& tag = entry->fTag;
                if (tag.fRef.isValid()) {
                    ImGui::Text("  skia draw %u (token %u)%s", tag.fRef.fOrdinal, tag.fRef.fToken,
                                tag.fRef.fToken == fAssetToken ? "" : "  [other asset]");
                } else {
                    ImGui::Text("  untracked draw");
                }
                ImGui::Text("  GPU draw %d, %s%s", entry->fGpuDrawIndex, tag.fStep.c_str(),
                            tag.fDepthOnly ? " (depth-only/clip)" : "");
                // Show the matching command, if it is in the focused asset.
                if (tag.fRef.isValid() && tag.fRef.fToken == fAssetToken && fDebugCanvas) {
                    for (int i = 0; i < (int)fCommandOrdinals.size(); ++i) {
                        if (fCommandOrdinals[i] == (int)tag.fRef.fOrdinal) {
                            ImGui::Text("  command %d: %s", i,
                                        DrawCommand::GetCommandString(
                                                fDebugCanvas->getDrawCommandAt(i)->getOpType()));
                            break;
                        }
                    }
                }
            }
        }
    }
    ImGui::End();
}
