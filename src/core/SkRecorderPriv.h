/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecorderPriv_DEFINED
#define SkRecorderPriv_DEFINED

#include "include/core/SkRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

class SkRecorderPriv final {
public:
    static sk_sp<SkSurface> MakeSurface(SkRecorder* recorder, const SkImageInfo& info, const SkSurfaceProps& props) {
        return recorder->makeSurface(info, props);
    }

private:
    SkRecorderPriv() = delete;
};

#endif
