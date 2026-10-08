/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WebGPUCaptureView_DEFINED
#define WebGPUCaptureView_DEFINED

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Loads a Dawn WebGPU-on-WebGPU capture command stream (the `.dawn_commands` file written by
// dawn::native::webgpu::StartCapture) and displays it in an ImGui window. GPU objects referenced
// by commands are shown with their WebGPU labels.
//
// This header intentionally has no Dawn dependencies; the implementation is only compiled when
// SK_VIEWER_WEBGPU_CAPTURE is defined (GN: skia_use_dawn).
class WebGPUCaptureView {
public:
    struct Node {
        std::string fText;
        std::vector<Node> fChildren;
    };

    struct ObjectInfo {
        uint64_t fId = 0;
        std::string fType;
        std::string fLabel;
    };

    // Returns nullptr if the file doesn't exist. If the file exists but fails to parse, a view is
    // still returned that displays the error (and whatever was parsed before it).
    static std::unique_ptr<WebGPUCaptureView> MakeFromFile(const std::string& commandsPath);

    // Draws the "WebGPU Commands" ImGui window.
    void draw();

    const std::string& path() const { return fPath; }
    const std::string& error() const { return fError; }
    size_t numRootCommands() const { return fCommands.size(); }
    size_t numObjects() const { return fObjects.size(); }

private:
    WebGPUCaptureView() = default;

    void drawNode(const Node& node, int& id);

    std::string fPath;
    size_t fFileSize = 0;
    std::string fError;
    std::vector<Node> fCommands;
    std::vector<ObjectInfo> fObjects;
    char fFilter[128] = {};
};

#endif  // WebGPUCaptureView_DEFINED
