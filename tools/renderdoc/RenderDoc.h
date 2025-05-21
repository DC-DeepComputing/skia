/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RenderDoc_DEFINED
#define RenderDoc_DEFINED

class RenderDoc {
public:
    RenderDoc();
    ~RenderDoc();

    void attach();
    void startFrame();
    void endFrame();

private:
    void* fRenderDocModule;
    void* fApi;
};

class [[nodiscard]] RenderDocCapture {
public:
    RenderDocCapture(RenderDoc* renderdoc) : fRenderDoc(renderdoc) {
        if (fRenderDoc != nullptr) {
            fRenderDoc->startFrame();
        }
    }
    ~RenderDocCapture() {
        if (fRenderDoc != nullptr) {
            fRenderDoc->endFrame();
        }
    }

private:
    RenderDoc* fRenderDoc;
};

#endif
