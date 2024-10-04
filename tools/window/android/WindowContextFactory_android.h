/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_android_DEFINED
#define WindowContextFactory_android_DEFINED

#include "include/core/SkRefCnt.h"

#include <android/native_window_jni.h>

#include <memory>

namespace skwindow {

class WindowContext;
struct DisplayParams;

std::unique_ptr<WindowContext> MakeVulkanForAndroid(ANativeWindow*, sk_sp<DisplayParams>);

std::unique_ptr<WindowContext> MakeGraphiteVulkanForAndroid(ANativeWindow*, sk_sp<DisplayParams>);

std::unique_ptr<WindowContext> MakeGLForAndroid(ANativeWindow*, sk_sp<DisplayParams>);

std::unique_ptr<WindowContext> MakeRasterForAndroid(ANativeWindow*, sk_sp<DisplayParams>);

}  // namespace skwindow

#endif
