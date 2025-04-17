/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGaneshRecorder_DEFINED
#define SkGaneshRecorder_DEFINED

#include "include/core/SkRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"

struct SkImageInfo;
class SkSurfaceProps;
class SkSurface;

class SkGaneshRecorder final : public SkRecorder {
public:
    SkGaneshRecorder(GrRecordingContext* ctx): fGaneshCtx(ctx) {}

    Type type() const override { return SkRecorder::Type::kGanesh; }

    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    GrRecordingContext* recordingContext() const {
        return fGaneshCtx;
    }

    GrDirectContext* directContext() const {
        return GrAsDirectContext(fGaneshCtx);
    }

private:
    GrRecordingContext* fGaneshCtx;
};

#endif
