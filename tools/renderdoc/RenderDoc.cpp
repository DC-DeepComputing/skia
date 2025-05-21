/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/renderdoc/RenderDoc.h"

#include "include/core/SkTypes.h"
#include "tools/library/LoadDynamicLibrary.h"

#include <stdio.h>

RenderDoc::RenderDoc() : fRenderDocModule(nullptr), fApi(nullptr) {}

RenderDoc::~RenderDoc() {
    if (fRenderDocModule) {
        SkFreeDynamicLibrary(fRenderDocModule);
    }
}

#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN)
#include "third_party/renderdoc/renderdoc_app.h"

#if defined(SK_BUILD_FOR_ANDROID)
constexpr char kRenderDocModuleName[] = "libVkLayer_GLES_RenderDoc.so";
#elif defined(SK_BUILD_FOR_WIN)
constexpr char kRenderDocModuleName[] = "renderdoc.dll";
#else
constexpr char kRenderDocModuleName[] = "librenderdoc.so";
#endif

void RenderDoc::attach() {
    fRenderDocModule =
            SkLoadDynamicLibrary(kRenderDocModuleName, DynamicLibrarySearch::AlreadyLoaded);
    if (fRenderDocModule == nullptr) {
        return;
    }
    void* getApi = SkGetProcedureAddress(fRenderDocModule, "RENDERDOC_GetAPI");
    if (getApi == nullptr) {
        return;
    }

    int result = reinterpret_cast<pRENDERDOC_GetAPI>(getApi)(eRENDERDOC_API_Version_1_1_2, &fApi);
    if (result != 1) {
        // RenderDoc module is present but API 1.1.2 is unavailable, need to get newer RenderDoc
        fApi = nullptr;
    }
}

void RenderDoc::startFrame() {
    if (fApi) {
        static_cast<RENDERDOC_API_1_1_2*>(fApi)->StartFrameCapture(nullptr, nullptr);
    }
}

void RenderDoc::endFrame() {
    if (fApi) {
        static_cast<RENDERDOC_API_1_1_2*>(fApi)->EndFrameCapture(nullptr, nullptr);
    }
}

#else  // defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX) ||
       // defined(SK_BUILD_FOR_WIN)

// Stub out the implementation on unsupported platforms.
void RenderDoc::attach() { fApi = nullptr; }

void RenderDoc::startFrame() {}

void RenderDoc::endFrame() {}

#endif  // defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX) ||
        // defined(SK_BUILD_FOR_WIN)
