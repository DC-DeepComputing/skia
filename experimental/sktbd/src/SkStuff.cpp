/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/sktbd/include/SkStuff.h"

sk_sp<SkSurface> MakeTBD(const SkImageInfo& imageInfo) {

    sk_sp<SkDevice> device = sk_make_sp<sktbd::Device>(ii);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}
