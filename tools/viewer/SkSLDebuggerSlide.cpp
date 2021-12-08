/*
* Copyright 2021 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/SkSLDebuggerSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/sksl/SkSLDebugTrace.h"
#include "src/core/SkEnumerate.h"
#include "tools/Resources.h"
#include "tools/viewer/Viewer.h"

#include <algorithm>
#include <cstdio>
#include "imgui.h"

using namespace sk_app;

///////////////////////////////////////////////////////////////////////////////

SkSLDebuggerSlide::SkSLDebuggerSlide() {
    fName = "Debugger";
}

void SkSLDebuggerSlide::load(SkScalar winWidth, SkScalar winHeight) {}

void SkSLDebuggerSlide::unload() {
    // Reconstruct the debug trace from scratch.
    fTrace.~SkVMDebugTrace();
    new (&fTrace) SkSL::SkVMDebugTrace;
}

void SkSLDebuggerSlide::showLoadTraceGUI() {
    ImGui::InputText("Trace Path", fTraceFile, SK_ARRAY_COUNT(fTraceFile));
    bool load = ImGui::Button("Load Debug Trace");

    if (load) {
        SkFILEStream file(fTraceFile);
        if (!file.isValid()) {
            ImGui::OpenPopup("Can't Open Trace");
        } else if (!fTrace.readTrace(&file)) {
            ImGui::OpenPopup("Invalid Trace");
        } else {
            // Trace loaded successfully. On the next refresh, the user will see the debug UI.
            return;
        }
    }

    if (ImGui::BeginPopupModal("Can't Open Trace", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("The trace file doesn't exist.");
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Invalid Trace", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("The trace data could not be parsed.");
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SkSLDebuggerSlide::showRootGUI() {
    if (fTrace.fSource.empty()) {
        this->showLoadTraceGUI();
        return;
    }

    ImGui::Text("Trace loaded successfully.");
    ImGui::Separator();
    for (const std::string& line : fTrace.fSource) {
        ImGui::Text("%s", line.c_str());
    }
}

void SkSLDebuggerSlide::draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    ImGui::Begin("Debugger", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    this->showRootGUI();
    ImGui::End();
}

bool SkSLDebuggerSlide::animate(double nanos) {
    return true;
}
