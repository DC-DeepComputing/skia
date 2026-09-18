/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRecorder.h"

#include "include/core/SkImageInfo.h"
#include "src/core/SkRecorderPriv.h"

class SkCanvas;
class SkSurface;
class SkRecorder;
class SkImageInfo;
class SkSurfaceProps;

namespace skcpu {
    class Recorder;
}

SkRecorderPriv SkRecorder::priv() { return SkRecorderPriv(this); }

sk_sp<SkSurface> SkRecorderPriv::makeSurface(SkImageInfo info,
                                             size_t rowBytes,
                                             const SkSurfaceProps* props = nullptr) {
    return fRecorder->makeSurface(info, rowBytes, props);
}

sk_sp<SkSurface> SkRecorder::makeSurface(SkImageInfo info,
                                         size_t rowBytes,
                                         const SkSurfaceProps* props = nullptr) {
    switch (fType){
        case Type::kCPU:
            return skcpu::AsRecorder(this)->makeSurface(info, rowBytes, props);
        break;
        case Type::kGanesh:
        break;
        case Type::kGraphite:
        break;
    }

    return nullptr;
}
