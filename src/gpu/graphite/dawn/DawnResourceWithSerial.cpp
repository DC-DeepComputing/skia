/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnResourceWithSerial.h"

#include "include/private/base/SkAssert.h"

namespace skgpu::graphite {

uint32_t DawnResourceSerialAllocator::allocateSerial() {
    uint32_t serial = fSerialCounter.fetch_add(1);

    // Make sure the serial didn't wrap
    SkASSERT(serial > 0);

    return serial;
}

}  // namespace skgpu::graphite
