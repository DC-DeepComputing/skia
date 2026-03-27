/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/private/SkHdrMetadata.h"

#include <cstddef>
#include <cstdint>

void FuzzHdrAgtm(const uint8_t* data, size_t size) {
    sk_sp<SkData> skdata = SkData::MakeWithoutCopy(data, size);
    skhdr::AdaptiveGlobalToneMap agtm;
    if (agtm.parse(skdata.get())) {
        auto serialized = agtm.serialize();
        (void)serialized;
    }
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size > 1024) {
        return 0;
    }
    FuzzHdrAgtm(data, size);
    return 0;
}
#endif
