/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef AHardwareBufferUtils_DEFINED
#define AHardwareBufferUtils_DEFINED

#include "include/core/SkColorType.h"
#include "include/core/SkTypes.h"

#if __ANDROID_API__ >= 26

namespace AHardwareBufferUtils {

// Given that we only use this texture as a source, colorType will not impact how Skia uses the
// texture. The only potential affect this is anticipated to have is that for some format types if
// we are not bound as an OES texture we may get invalid results for SKP capture if we read back the
// texture. VulkanCaps expects kRGBA_8888_SkColorType color type for textures with an external
// format.
static const SkColorType kExternalFormatColorType = SkColorType::kRGBA_8888_SkColorType;

SkColorType GetSkColorTypeFromBufferFormat(uint32_t bufferFormat);

}  // namespace AHardwareBufferUtils

#endif

#endif // AHardwareBufferUtils_DEFINED
