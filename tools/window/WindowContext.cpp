/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/WindowContext.h"

#include "include/gpu/ganesh/GrDirectContext.h"

namespace skwindow {

WindowContext::WindowContext(const DisplayParams& params)
        : fDisplayParams(params) {}

WindowContext::~WindowContext() {}

void WindowContext::swapBuffers() {
    this->onSwapBuffers();
}

}  // namespace skwindow
