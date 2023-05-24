/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/ports/SkTypeface_fontations_factory.h"
#include "src/ports/SkTypeface_fontations.h"

SK_API sk_sp<SkTypeface> Make_SkTypeface_Fontations(std::unique_ptr<SkStreamAsset> fontData,
                                                    uint32_t ttcIndex) {
    return sk_sp<SkTypeface>(new SkTypeface_Fontations(std::move(fontData), ttcIndex));
}
