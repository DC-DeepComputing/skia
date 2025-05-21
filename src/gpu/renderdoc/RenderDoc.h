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
    void startFrame() const;
    void endFrame() const;

private:
    void* fRenderDocModule;
    void* fApi;
};

#endif
