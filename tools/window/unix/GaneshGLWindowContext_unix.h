/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GaneshGLWindowContext_unix_DEFINED
#define GaneshGLWindowContext_unix_DEFINED

#include "include/core/SkRefCnt.h"

#include <memory>

namespace skwindow {
class WindowContext;
struct DisplayParams;
struct XlibWindowInfo;

std::unique_ptr<WindowContext> MakeGaneshGLForXlib(const XlibWindowInfo&, sk_sp<DisplayParams>);
}  // namespace skwindow

#endif
