/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_GraphiteDawnToggles_DEFINED
#define skiatest_graphite_GraphiteDawnToggles_DEFINED

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skiatest::graphite {

wgpu::DawnTogglesDescriptor GetInstanceToggles();

wgpu::DawnTogglesDescriptor GetDeviceToggles();

}  // namespace skiatest::graphite

#endif  // skiatest_graphite_GraphiteDawnToggles_DEFINED
