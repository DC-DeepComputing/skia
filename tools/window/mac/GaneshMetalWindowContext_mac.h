/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GaneshMetalWindowContext_mac_DEFINED
#define GaneshMetalWindowContext_mac_DEFINED

#include "include/core/SkRefCnt.h"

#include <memory>

namespace skwindow {
class WindowContext;
struct DisplayParams;
struct MacWindowInfo;

std::unique_ptr<WindowContext> MakeGaneshMetalForMac(const MacWindowInfo&, sk_sp<DisplayParams>);
}  // namespace skwindow

#endif
