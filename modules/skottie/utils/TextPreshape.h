/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextPreshape_DEFINED
#define SkottieTextPreshape_DEFINED

#include "include/core/SkRefCnt.h"

class SkFontMgr;
class SkWStream;

namespace skjson { class DOM; }
namespace skresources { class ResourceProvider; }

namespace skottie_utils {

bool Preshape(const skjson::DOM&, SkWStream*,
              sk_sp<skresources::ResourceProvider>, sk_sp<SkFontMgr>);

} //  namespace skottie_utils

#endif //  SkottieTextPreshape_DEFINED
