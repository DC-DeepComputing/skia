/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextPreshape_DEFINED
#define SkottieTextPreshape_DEFINED

#include "src/utils/SkJSON.h"

class SkWStream;

namespace skottie_utils {

bool Preshape(const skjson::DOM&, SkWStream*);

} //  namespace skottie_utils

#endif //  SkottieTextPreshape_DEFINED
