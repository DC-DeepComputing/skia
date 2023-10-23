/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnResourceWithSerial_DEFINED
#define skgpu_graphite_DawnResourceWithSerial_DEFINED

#include <atomic>

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkNoncopyable.h"

namespace skgpu::graphite {

class DawnSharedContext;
class DawnTexture;

class DawnResourceWithSerial : SkNoncopyable {
public:
    DawnResourceWithSerial() = delete;
    DawnResourceWithSerial(uint32_t serial) : fSerial(serial) {}

    uint32_t getSerial() const {
        SkASSERT(fSerial > 0);
        return fSerial;
    }

private:
    const uint32_t fSerial;
};

class DawnResourceSerialAllocator {
public:
    uint32_t allocateSerial();
private:
    std::atomic<uint32_t> fSerialCounter = {1};
};

}

#endif
