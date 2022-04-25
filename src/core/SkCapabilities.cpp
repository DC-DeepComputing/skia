/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCapabilities.h"

sk_sp<SkCapabilities> SkCapabilities::RasterBackend() {
    static SkCapabilities* sCaps = [](){
        SkCapabilities* caps = new SkCapabilities;
        caps->fSkSLVersion = SkCapabilities::SkSLVersion::k100ES;
        return caps;
    }();

    return sk_ref_sp(sCaps);
}
