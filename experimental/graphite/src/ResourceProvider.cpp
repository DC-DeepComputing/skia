/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ResourceProvider.h"

#include "experimental/graphite/src/CommandBuffer.h"

namespace skgpu {

ResourceProvider::ResourceProvider() {
}

ResourceProvider::~ResourceProvider() {
}

sk_sp<CommandBuffer> ResourceProvider::createCommandBuffer() {
    sk_sp<CommandBuffer> cb = this->onCreateCommandBuffer();

    return cb;
}

} // namespace skgpu
