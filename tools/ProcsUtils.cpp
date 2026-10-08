/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/ProcsUtils.h"

#include "include/core/SkImage.h"

#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
#include "include/encode/SkPngRustEncoder.h"
#else
#include "include/encode/SkPngEncoder.h"
#endif

namespace ToolUtils {

SkSerialProcs default_serial_procs() {
    static SkSerialProcs procs;

    procs.fImageProc = [](SkImage* img, void*) -> sk_sp<const SkData> {
#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
        return SkPngRustEncoder::Encode(nullptr, img, {});
#else
        // TODO: This catches SkImageEncoder_NDK (or other).
        return SkPngEncoder::Encode(nullptr, img, {});
#endif
    };

    return procs;
}

}  // namespace ToolUtils
