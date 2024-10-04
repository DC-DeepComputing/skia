
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_win_DEFINED
#define WindowContextFactory_win_DEFINED

#include "include/core/SkRefCnt.h"

#include <Windows.h>

#include <memory>

namespace skwindow {

class WindowContext;
struct DisplayParams;

#ifdef SK_VULKAN
std::unique_ptr<WindowContext> MakeVulkanForWin(HWND, sk_sp<DisplayParams>);
#if defined(SK_GRAPHITE)
std::unique_ptr<WindowContext> MakeGraphiteVulkanForWin(HWND, sk_sp<DisplayParams>);
#endif
#endif

#ifdef SK_GL
std::unique_ptr<WindowContext> MakeGLForWin(HWND, sk_sp<DisplayParams>);
#endif

#ifdef SK_ANGLE
std::unique_ptr<WindowContext> MakeANGLEForWin(HWND, sk_sp<DisplayParams>);
#endif

#ifdef SK_DIRECT3D
std::unique_ptr<WindowContext> MakeD3D12ForWin(HWND, sk_sp<DisplayParams>);
#endif

#ifdef SK_DAWN
#if defined(SK_GRAPHITE)
std::unique_ptr<WindowContext> MakeGraphiteDawnD3D12ForWin(HWND, sk_sp<DisplayParams>);
#endif
#endif

std::unique_ptr<WindowContext> MakeRasterForWin(HWND, sk_sp<DisplayParams>);

}  // namespace skwindow

#endif
