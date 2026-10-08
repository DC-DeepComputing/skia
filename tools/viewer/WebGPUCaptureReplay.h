/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WebGPUCaptureReplay_DEFINED
#define WebGPUCaptureReplay_DEFINED

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <vector>

// Replays a Dawn WebGPU capture (`.dawn_commands` + `.dawn_content`) on the viewer's own Dawn
// device, then reads back textures of the replayed capture by capture object id. Used to inspect
// the depth attachments kept by SkCapture draw id tracking, and to step through the capture one
// draw at a time (replaying only the first N draw calls, see dawn::replay::Replay::SetDrawLimit).
//
// This header has no Dawn dependencies; without SK_VIEWER_WEBGPU_CAPTURE, Make() returns nullptr.
class WebGPUCaptureReplay {
public:
    static constexpr uint64_t kNoDrawLimit = std::numeric_limits<uint64_t>::max();

    struct DepthImage {
        uint32_t fWidth = 0;
        uint32_t fHeight = 0;
        std::vector<float> fDepth;  // fWidth * fHeight values in [0, 1] (sample 0 if MSAA)
    };

    struct ColorImage {
        uint32_t fWidth = 0;
        uint32_t fHeight = 0;
        std::vector<uint8_t> fRGBA;  // fWidth * fHeight premultiplied RGBA8 pixels
    };

    // Creates the replay device and replays the whole capture. Returns nullptr (and sets *error)
    // on failure.
    static std::unique_ptr<WebGPUCaptureReplay> Make(const std::string& commandsPath,
                                                     const std::string& contentPath,
                                                     std::string* error);
    virtual ~WebGPUCaptureReplay() = default;

    // Replays the capture again from the start on the same device, executing only the first
    // 'drawLimit' GPU draw calls (in capture order; the same numbering as WebGPUCaptureView's GPU
    // draw indices). All other commands still execute.
    virtual bool play(uint64_t drawLimit, std::string* error) = 0;
    virtual uint64_t drawLimit() const = 0;

    // Reads the depth aspect of a replayed texture (by capture object id). The texture must have
    // TextureBinding usage (true for SkCapture draw id tracking depth attachments).
    virtual bool readDepth(uint64_t textureId, DepthImage* out, std::string* error) = 0;

    // Reads mip 0 / layer 0 of a single-sampled color texture (by capture object id).
    virtual bool readColor(uint64_t textureId, ColorImage* out, std::string* error) = 0;

    // Validation errors reported by the replay device so far.
    virtual const std::vector<std::string>& deviceErrors() const = 0;

protected:
    WebGPUCaptureReplay() = default;
};

#endif  // WebGPUCaptureReplay_DEFINED
