/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_fontations_factory_DEFINED
#define SkTypeface_fontations_factory_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"

#include <memory>

SK_API sk_sp<SkTypeface> Make_SkTypeface_Fontations(std::unique_ptr<SkStreamAsset> fontData,
                                                    uint32_t ttcIndex);

#endif
