/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/ThreadSafeProxy.h"

#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/ThreadSafeProxyPriv.h"

namespace skgpu::graphite {

ThreadSafeProxy::~ThreadSafeProxy() {}

ThreadSafeProxy::ThreadSafeProxy(sk_sp<SharedContext> sharedContext) : fSharedContext(sharedContext) {}

} // namespace skgpu::graphite
