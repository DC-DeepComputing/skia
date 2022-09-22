/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PaintParams.h"

#include "include/core/SkShader.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/core/SkUniform.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu::graphite {

PaintParams::PaintParams(const SkColor4f& color,
                         sk_sp<SkBlender> finalBlender,
                         sk_sp<SkShader> shader,
                         sk_sp<SkColorFilter> colorFilter,
                         sk_sp<SkBlender> primitiveBlender,
                         bool skipColorXform)
        : fColor(color)
        , fFinalBlender(std::move(finalBlender))
        , fShader(std::move(shader))
        , fColorFilter(std::move(colorFilter))
        , fPrimitiveBlender(std::move(primitiveBlender))
        , fSkipColorXform(skipColorXform) {}

PaintParams::PaintParams(const SkPaint& paint,
                         sk_sp<SkBlender> primitiveBlender,
                         bool skipColorXform)
        : fColor(paint.getColor4f())
        , fFinalBlender(paint.refBlender())
        , fShader(paint.refShader())
        , fColorFilter(paint.refColorFilter())
        , fPrimitiveBlender(std::move(primitiveBlender))
        , fSkipColorXform(skipColorXform) {}

PaintParams::PaintParams(const PaintParams& other) = default;
PaintParams::~PaintParams() = default;
PaintParams& PaintParams::operator=(const PaintParams& other) = default;

std::optional<SkBlendMode> PaintParams::asFinalBlendMode() const {
    return fFinalBlender ? as_BB(fFinalBlender)->asBlendMode()
                         : SkBlendMode::kSrcOver;
}

sk_sp<SkBlender> PaintParams::refFinalBlender() const { return fFinalBlender; }

sk_sp<SkShader> PaintParams::refShader() const { return fShader; }

sk_sp<SkColorFilter> PaintParams::refColorFilter() const { return fColorFilter; }

sk_sp<SkBlender> PaintParams::refPrimitiveBlender() const { return fPrimitiveBlender; }

static bool key_needs_paint_color(const SkKeyContext& keyContext,
                                  SkPaintParamsKeyBuilder* builder,
                                  SkShader* shader) {
    // If there's no shader, the paint color is always used.
    if (!shader) {
        return true;
    }

    // Convert the shader to a temporary key (without actually gathering data).
    as_SB(shader)->addToKey(keyContext, builder, /*gatherer=*/nullptr);

    // Convert the temporary key to an SkShaderInfo; this identifies all the snippets' requirements.
    SkPaintParamsKey key = builder->lockAsKey();
    SkShaderInfo shaderInfo;
    key.toShaderInfo(keyContext.dict(), &shaderInfo);

    // If any of the shader's snippets use the prior-stage output, we will need the paint color.
    return shaderInfo.needsPriorStageOutput();
}

void PaintParams::toKey(const SkKeyContext& keyContext,
                        SkPaintParamsKeyBuilder* builder,
                        SkPipelineDataGatherer* gatherer) const {
    // We perform the paint-color check before starting to build the key, so we can use `builder` as
    // a scratchpad.
    bool needsPaintColor = key_needs_paint_color(keyContext, builder, fShader.get());

    if (fPrimitiveBlender) {
        as_BB(fPrimitiveBlender)->addToKey(keyContext, builder, gatherer,
                                           /*primitiveColorBlender=*/true);
    }

    if (needsPaintColor) {
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, fColor.premul());
        builder->endBlock();
    }

    if (fShader) {
        as_SB(fShader)->addToKey(keyContext, builder, gatherer);
    }

    if (fShader) {
        as_SB(fShader)->addToKey(keyContext, builder, gatherer);
    }

    if (fColorFilter) {
        as_CFB(fColorFilter)->addToKey(keyContext, builder, gatherer);
    }

    if (fFinalBlender) {
        as_BB(fFinalBlender)->addToKey(keyContext, builder, gatherer,
                                       /*primitiveColorBlender=*/false);
    } else {
        BlendModeBlock::BeginBlock(keyContext, builder, gatherer, SkBlendMode::kSrcOver);
        builder->endBlock();
    }

    if (gatherer) {
        if (gatherer->needsLocalCoords()) {
#ifdef SK_DEBUG
            static constexpr SkUniform kDev2LocalUniform[] = {{ "dev2Local", SkSLType::kFloat4x4 }};
            UniformExpectationsValidator uev(gatherer,
                                             SkSpan<const SkUniform>(kDev2LocalUniform, 1));
#endif

            gatherer->write(keyContext.dev2Local());
        }
    }

    SkASSERT(builder->sizeInBytes() > 0);
}

} // namespace skgpu::graphite
