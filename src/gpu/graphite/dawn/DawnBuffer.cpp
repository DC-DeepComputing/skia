/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnBuffer.h"

namespace skgpu::graphite {


void DawnBuffer::onMap() {
    // TODO: https://b.corp.google.com/issues/259442778
    SkASSERT(false);
}

void DawnBuffer::onUnmap() {
    // TODO: https://b.corp.google.com/issues/259442778
    SkASSERT(false);
}

void DawnBuffer::freeGpuData() {
    // TODO: https://b.corp.google.com/issues/259442778
    SkASSERT(false);
}

} // namespace skgpu::graphite

