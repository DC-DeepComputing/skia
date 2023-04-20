/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDashingMaskFilter_DEFINED
#define SkDashingMaskFilter_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

class SkMaskFilter;

class SK_API SkDashingMaskFilter {
public:
    static sk_sp<SkMaskFilter> Make();

private:
    static void RegisterFlattenables();
    friend class SkFlattenable;
};

#endif // SkDashingMaskFilter_DEFINED
