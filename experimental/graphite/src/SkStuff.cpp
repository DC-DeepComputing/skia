/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/SkStuff.h"

#include "experimental/graphite/src/Device.h"
#include "experimental/graphite/src/Recorder.h"
#include "experimental/graphite/src/Surface_Graphite.h"

std::unique_ptr<SkSurface> MakeGraphite(sk_sp<skgpu::Recorder> recorder, const SkImageInfo& ii) {

    sk_sp<skgpu::Device> device = skgpu::Device::Make(std::move(recorder), ii);
    if (!device) {
        return nullptr;
    }

    return std::make_unique<skgpu::Surface_Graphite>(std::move(device));
}
