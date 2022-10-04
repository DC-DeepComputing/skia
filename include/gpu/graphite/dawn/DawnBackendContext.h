/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnBackendContext_DEFINED
#define skgpu_graphite_DawnBackendContext_DEFINED

namespace skgpu::graphite {

// The DawnBackendContext contains all of the base Metal objects needed by the graphite Metal
// backend. The client will create this object and pass it into the Context::MakeMetal factory call
// when setting up Skia.
struct SK_API DawnBackendContext {
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnBackendContext_DEFINED
