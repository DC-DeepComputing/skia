/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RecorderOptions_DEFINED
#define skgpu_graphite_RecorderOptions_DEFINED

#include "include/gpu/graphite/ImageProvider.h"

namespace skgpu::graphite {

class ImageProvider;

struct SK_API RecorderOptions {
    RecorderOptions() {}

    sk_sp<ImageProvider> fImageProvider;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_RecorderOptions_DEFINED
