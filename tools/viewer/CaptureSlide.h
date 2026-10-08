/*
* Copyright 2025 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef CaptureSlide_DEFINED
#define CaptureSlide_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "src/capture/SkCapture.h"
#include "tools/debugger/DebugCanvas.h"
#include "tools/viewer/Slide.h"
#include "tools/viewer/WebGPUCaptureView.h"
#include "tools/viewer/WebGPUCaptureReplay.h"
#include "include/core/SkImage.h"
#include "include/core/SkPoint.h"

#include <string>
#include <vector>

class SkCanvas;
class SkPicture;
class SkStream;
class SkString;

class CaptureSlide : public Slide {
public:
    CaptureSlide(const SkString& name, const SkString& path);
    ~CaptureSlide() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas* canvas) override;
    bool animate(double) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    bool onChar(SkUnichar) override;

private:
    sk_sp<SkCapture> fCapture;
    int fCurrentAssetIndex = 0;
    int fCurrentRecordingCaptureIndex = 0;
    int fCurrentDrawTaskIndex = 0;
    bool fInvalidate = false;
    SkCapture::Metadata fMetadata;
    std::unique_ptr<DebugCanvas> fDebugCanvas;
    // Dawn WebGPU capture commands loaded from "<capture path>.dawn_commands", if present.
    std::unique_ptr<WebGPUCaptureView> fWebGPUView;

    // SkCapture draw id tracking (see src/capture/SkCaptureDrawId.h). fAssetToken is the draw id
    // token stored in the focused asset (0 if untracked); fCommandOrdinals maps each fDebugCanvas
    // command index to its draw ordinal (-1 for non-draw commands).
    uint32_t fAssetToken = 0;
    std::vector<int> fCommandOrdinals;
    // The selected Skia draw, shared between the Command History and WebGPU Commands windows.
    WebGPUCaptureView::DrawRef fSelectedDraw;
    WebGPUCaptureView::DrawRef fLastHistorySelection;

    void updateActiveAssetIndex();
    void computeDrawIds(const SkPicture* picture);
    void drawCommandHistory();

    // ---- Draw ID (depth) view: maps pixels to Skia draws via the per-pass depth attachments kept
    // by SkCapture draw id tracking, read back by replaying the Dawn capture.
    void drawDrawIdWindow();
    bool ensureReplay();
    // Reads back a render pass's color target and depth from the current replay state.
    void loadPass(int passIndex);
    // Step mode: replays the capture up to (and including) the last GPU draw of 'ref' and loads
    // that pass.
    void stepTo(WebGPUCaptureView::DrawRef ref);
    // GPU-order step mode: replays GPU draws 0..gpuDrawIndex and loads that draw's pass.
    void stepToGpuDraw(int gpuDrawIndex);
    void setStepMode(bool enabled);
    // Moves the selection by 'count' skia draws that have GPU draws (negative: backwards).
    void moveSelection(int count);
    // Depth values whose pixels are highlighted: the current GPU draw's (GPU-order step mode) or
    // the selected skia draw's, in the loaded pass.
    std::vector<uint32_t> highlightDepths() const;
    void rebuildIdImage();
    void rebuildDimImage();
    // Returns the depth entry for a pixel of the loaded pass depth (nullptr if none).
    const WebGPUCaptureView::PassDepthEntry* entryAt(int x, int y, uint32_t* depthBits) const;
    bool onMouse(SkScalar x, SkScalar y, skui::InputState, skui::ModifierKey) override;

    std::string fCapturePath;
    std::unique_ptr<WebGPUCaptureReplay> fReplay;
    bool fReplayAttempted = false;
    std::string fReplayError;
    int fDrawIdPass = -1;          // selected render pass (index into fWebGPUView->passes())
    int fLoadedDepthPass = -1;     // pass whose depth is in fDepthBits (and color in fPassColor)
    std::string fDepthError;
    uint32_t fDepthWidth = 0, fDepthHeight = 0;
    std::vector<uint32_t> fDepthBits;  // per pixel PaintersDepth (round((1 - z) * 0xFFFF))
    sk_sp<SkImage> fPassColor;         // replayed color target of fLoadedDepthPass

    // What the canvas shows.
    enum class CanvasView {
        kSkiaPlayback,  // the SkCapture played back by the viewer (CPU/viewer backend)
        kGpuColor,      // the replayed GPU color target of the loaded pass
        kDrawIds,       // each pixel colored by a hash of the skia draw that last wrote it
    };
    CanvasView fCanvasView = CanvasView::kSkiaPlayback;
    // Dims every pixel that the selected skia draw did not write last (on top of any view).
    bool fHighlightSelected = false;
    sk_sp<SkImage> fIdImage;
    bool fIdImageDirty = true;
    sk_sp<SkImage> fDimImage;
    bool fDimImageDirty = true;
    WebGPUCaptureView::DrawRef fDimSelection;
    int fDimGpuDraw = -1;
    SkIPoint fHoverPixel = {-1, -1};

    // Step mode: replay only part of the capture.
    enum class StepBy {
        kSkiaDraw,  // up to the selected skia draw's last GPU draw
        kGpuDraw,   // GPU draws 0..N in real GPU (sorted/batched) order
    };
    bool fStepMode = false;
    StepBy fStepBy = StepBy::kSkiaDraw;
    WebGPUCaptureView::DrawRef fStepSelection;  // selection the replay was made for
    int fGpuStep = 0;                           // GPU-order step: target GPU draw index
    int fGpuStepReplayed = -1;                  // GPU draw index the replay was made for
    int fStepPass = -1;
    int fStepLastGpuDraw = -1;
    std::string fStepMessage;
};

#endif
