
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_ios_DEFINED
#define WindowContextFactory_ios_DEFINED

#include "tools/window/WindowContext.h"

#import <UIKit/UIKit.h>

#include <memory>

namespace sk_app {
class Window_ios;
}

namespace skwindow {

struct DisplayParams;

struct IOSWindowInfo {
    sk_app::Window_ios* fWindow;
    UIViewController*   fViewController;
};

#ifdef SK_VULKAN
inline std::unique_ptr<WindowContext> MakeVulkanForIOS(const IOSWindowInfo&, const DisplayParams&) {
    // No Vulkan support on iOS yet.
    return nullptr;
}
#endif

#ifdef SK_METAL
std::unique_ptr<WindowContext> MakeMetalForIOS(const IOSWindowInfo&, const DisplayParams&);
#if defined(SK_GRAPHITE)
std::unique_ptr<WindowContext> MakeGraphiteMetalForIOS(const IOSWindowInfo&, const DisplayParams&);
#endif
#endif

#ifdef SK_GL
std::unique_ptr<WindowContext> MakeGLForIOS(const IOSWindowInfo&, const DisplayParams&);
std::unique_ptr<WindowContext> MakeRasterForIOS(const IOSWindowInfo&, const DisplayParams&);
#endif

}  // namespace skwindow

#endif
