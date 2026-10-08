/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WebGPUCaptureView_DEFINED
#define WebGPUCaptureView_DEFINED

#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Loads a Dawn WebGPU-on-WebGPU capture command stream (the `.dawn_commands` file written by
// dawn::native::webgpu::StartCapture) and displays it in an ImGui window. GPU objects referenced
// by commands are shown with their WebGPU labels.
//
// If the capture was made with SkCapture draw id tracking (ContextOptions::
// fEnableCaptureDrawIdTracking), Graphite inserted a debug marker per (Skia draw, RenderStep)
// before each GPU draw call:
//     "sk d=<PaintersDepth> c=<capture canvas token> i=<capture draw ordinal> s=<RenderStep>[ clip]"
// These markers are attached to the GPU draw call that follows them, so GPU draws and Skia draw
// commands can be cross-linked.
//
// This header intentionally has no Dawn dependencies; the implementation is only compiled when
// SK_VIEWER_WEBGPU_CAPTURE is defined (GN: skia_use_dawn).
class WebGPUCaptureView {
public:
    // Identifies one Skia draw command: the asset's draw id token + the draw's ordinal within it.
    struct DrawRef {
        uint32_t fToken = 0;  // 0 = none
        uint32_t fOrdinal = 0;

        bool isValid() const { return fToken != 0; }
        bool operator==(const DrawRef& o) const {
            return fToken == o.fToken && fOrdinal == o.fOrdinal;
        }
        bool operator<(const DrawRef& o) const {
            return fToken < o.fToken || (fToken == o.fToken && fOrdinal < o.fOrdinal);
        }
    };

    // One parsed "sk ..." marker.
    struct DrawTag {
        DrawRef fRef;
        uint32_t fDepth = 0;  // Graphite PaintersDepth (16 bit)
        std::string fStep;    // RenderStep name
        bool fDepthOnly = false;
    };

    struct Node {
        std::string fText;
        std::vector<Node> fChildren;
        // For GPU draw calls: index among all GPU draw calls (-1 otherwise) and the Skia draws
        // that were batched into it.
        int fGpuDrawIndex = -1;
        std::vector<DrawTag> fTags;
        // For BeginRenderPass nodes: index into passes() (-1 otherwise).
        int fPassIndex = -1;
    };

    // The draw that wrote a given depth value in a render pass.
    struct PassDepthEntry {
        DrawTag fTag;
        int fGpuDrawIndex = -1;
    };

    struct RenderPassInfo {
        int fIndex = 0;
        std::string fLabel;
        std::string fColorTarget;  // description of color[0] (resolve target if MSAA)
        uint64_t fColorTextureId = 0;  // texture of color[0] (resolve target if MSAA)
        uint32_t fWidth = 0, fHeight = 0;
        // Depth/stencil attachment (empty label if none). With draw id tracking the texture is
        // unique per pass, stored, and labeled "SkDrawIdDepth#<n>".
        uint64_t fDepthTextureId = 0;
        std::string fDepthTextureLabel;
        std::string fDepthFormat;
        uint32_t fDepthSampleCount = 1;
        bool fDepthStored = false;
        bool fDepthBindable = false;
        int fNumGpuDraws = 0;
        // PaintersDepth -> draw. Color-writing draws take precedence over depth-only (clip) draws.
        std::map<uint32_t, PassDepthEntry> fDepthToDraw;
    };

    struct ObjectInfo {
        uint64_t fId = 0;
        std::string fType;
        std::string fLabel;
    };

    // Returns nullptr if the file doesn't exist. If the file exists but fails to parse, a view is
    // still returned that displays the error (and whatever was parsed before it).
    static std::unique_ptr<WebGPUCaptureView> MakeFromFile(const std::string& commandsPath);

    // Draws the "WebGPU Commands" ImGui window. 'selected' is the Skia draw command selected in
    // the viewer (shared with the SkCapture command list); clicking a Skia draw listed under a GPU
    // draw call updates it.
    void draw(DrawRef* selected);

    const std::string& path() const { return fPath; }
    const std::string& error() const { return fError; }
    size_t numRootCommands() const { return fCommands.size(); }
    size_t numObjects() const { return fObjects.size(); }

    // Draw id tracking info (empty if the capture has no "sk" markers).
    bool hasDrawIds() const { return !fGpuDraws.empty() && fNumTags > 0; }
    size_t numGpuDraws() const { return fGpuDraws.size(); }
    // The command tree node of a GPU draw call (its text and its Skia draw tags), or nullptr.
    const Node* gpuDraw(int gpuDrawIndex) const {
        return gpuDrawIndex >= 0 && gpuDrawIndex < (int)fGpuDraws.size() ? fGpuDraws[gpuDrawIndex]
                                                                         : nullptr;
    }
    // GPU draw call indices that contain the given Skia draw.
    const std::vector<int>* gpuDrawsFor(DrawRef ref) const {
        auto it = fGpuDrawsBySkiaDraw.find(ref);
        return it == fGpuDrawsBySkiaDraw.end() ? nullptr : &it->second;
    }
    const std::vector<RenderPassInfo>& passes() const { return fPasses; }
    // The next (forward) or previous Skia draw, in (token, ordinal) order, that has GPU draws.
    // Returns an invalid ref if there is none.
    DrawRef adjacentDraw(DrawRef ref, bool forward) const {
        if (forward) {
            auto it = fGpuDrawsBySkiaDraw.upper_bound(ref);
            return it == fGpuDrawsBySkiaDraw.end() ? DrawRef{} : it->first;
        }
        auto it = fGpuDrawsBySkiaDraw.lower_bound(ref);
        return it == fGpuDrawsBySkiaDraw.begin() ? DrawRef{} : std::prev(it)->first;
    }
    // Render pass (index into passes()) containing the given GPU draw, or -1.
    int passOfGpuDraw(int gpuDrawIndex) const {
        return gpuDrawIndex >= 0 && gpuDrawIndex < (int)fGpuDrawPass.size()
                       ? fGpuDrawPass[gpuDrawIndex]
                       : -1;
    }

private:
    WebGPUCaptureView() = default;

    // Assigns GPU draw indices and builds fGpuDraws / fGpuDrawsBySkiaDraw (after parsing).
    void buildDrawIndex();
    void indexNode(const Node& node, int passIndex);

    void drawNode(const Node& node, int& id, DrawRef* selected);
    bool nodeShown(const Node& node, const DrawRef& selected) const;

    std::string fPath;
    size_t fFileSize = 0;
    std::string fError;
    std::vector<Node> fCommands;
    std::vector<ObjectInfo> fObjects;
    char fFilter[128] = {};
    bool fOnlySelectedDraws = false;

    std::vector<RenderPassInfo> fPasses;
    std::vector<const Node*> fGpuDraws;
    std::vector<int> fGpuDrawPass;  // pass index per GPU draw (-1 for render bundle draws)
    std::map<DrawRef, std::vector<int>> fGpuDrawsBySkiaDraw;
    size_t fNumTags = 0;
};

#endif  // WebGPUCaptureView_DEFINED
