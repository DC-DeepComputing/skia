/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkTableColorFilter_DEFINED
#define SkTableColorFilter_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSLSampleUsage.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/RecorderPriv.h"

namespace skgpu::graphite {
class PipelineDataGatherer;
}
#endif

#if defined(SK_ENABLE_SKSL) && defined(SK_ENABLE_SKVM)
#include "src/core/SkVM.h"
#endif

class SkTableColorFilter final : public SkColorFilterBase {
public:
    SkTableColorFilter(const uint8_t tableA[],
                       const uint8_t tableR[],
                       const uint8_t tableG[],
                       const uint8_t tableB[]);

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kTable; }

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override;
#endif

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeByteArray(fBitmap.getAddr8(0,0), 4*256);
    }

private:
    friend void ::SkRegisterTableColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkTableColorFilter)

    SkBitmap fBitmap;
};

#endif