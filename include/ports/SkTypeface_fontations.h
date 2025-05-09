/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_fontations_DEFINED
#define SkTypeface_fontations_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"

#include <memory>

SK_API sk_sp<SkTypeface> SkTypeface_Make_Fontations(
    std::unique_ptr<SkStreamAsset> fontData,
    const SkFontArguments& args,
    // TODO(https://crbug.com/414823091): Stop defaulting to `nullptr`.
    SkCodecs::MakeFromStreamCallback decodePngCallback = nullptr);

SK_API sk_sp<SkTypeface> SkTypeface_Make_Fontations(
    sk_sp<SkData> fontData,
    const SkFontArguments& args,
    // TODO(https://crbug.com/414823091): Stop defaulting to `nullptr`.
    SkCodecs::MakeFromStreamCallback decodePngCallback = nullptr);

#endif
