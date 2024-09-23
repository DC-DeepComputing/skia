/*
 * Copyright 2006-2012 The Android Open Source Project
 * Copyright 2012 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_Freetype_DEFINED
#define SkTypeface_Freetype_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/ports/SkFontScanner.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"

#include "include/core/SkFontArguments.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"

#include <memory>

SK_API sk_sp<SkTypeface> SkTypeface_Make_FreeType(std::unique_ptr<SkStreamAsset>,
                                                  const SkFontArguments&);

#endif // SkTypeface_Freetype_DEFINED
