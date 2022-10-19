/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/VolatileProxyContext.h"

#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

VolatileProxyContext::VolatileProxyContext() {}
VolatileProxyContext::~VolatileProxyContext() {}

void VolatileProxyContext::insert(sk_sp<TextureProxy> volatileProxy) {
    fVolatileProxies.insert(std::move(volatileProxy));
}

} // namespace skgpu::graphite
