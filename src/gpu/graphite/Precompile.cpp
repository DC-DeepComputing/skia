/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/DitherUtils.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/FactoryFunctionsPriv.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PrecompileBasePriv.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileShader> PrecompileShader::makeWithLocalMatrix() {
    if (this->priv().isALocalMatrixShader()) {
        // SkShader::makeWithLocalMatrix collapses chains of localMatrix shaders so we need to
        // follow suit here
        return sk_ref_sp(this);
    }

    return PrecompileShaders::LocalMatrix({ sk_ref_sp(this) });
}

sk_sp<PrecompileShader> PrecompileShader::makeWithColorFilter(sk_sp<PrecompileColorFilter> cf) {
    if (!cf) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::ColorFilter({ sk_ref_sp(this) }, { std::move(cf) });
}

sk_sp<PrecompileShader> PrecompileShader::makeWithWorkingColorSpace(sk_sp<SkColorSpace> cs) {
    if (!cs) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::WorkingColorSpace({ sk_ref_sp(this) }, { std::move(cs) });
}

sk_sp<PrecompileColorFilter> PrecompileColorFilter::makeComposed(
        sk_sp<PrecompileColorFilter> inner) const {
    if (!inner) {
        return sk_ref_sp(this);
    }

    return PrecompileColorFilters::Compose({ sk_ref_sp(this) }, { std::move(inner) });
}

//--------------------------------------------------------------------------------------------------



namespace {

void create_image_drawing_pipelines(const KeyContext& keyContext,
                                    PipelineDataGatherer* gatherer,
                                    const PaintOptionsPriv::ProcessCombination& processCombination,
                                    const PaintOptions& orig) {
    PaintOptions imagePaintOptions;

    // For imagefilters we know we don't have alpha-only textures and don't need cubic filtering.
    sk_sp<PrecompileShader> imageShader = PrecompileShadersPriv::Image(
            PrecompileImageShaderFlags::kExcludeAlpha | PrecompileImageShaderFlags::kExcludeCubic);

    imagePaintOptions.setShaders({ imageShader });
    imagePaintOptions.setBlendModes(orig.getBlendModes());
    imagePaintOptions.setBlenders(orig.getBlenders());
    imagePaintOptions.setColorFilters(orig.getColorFilters());
    imagePaintOptions.addColorFilter(nullptr);

    imagePaintOptions.priv().buildCombinations(keyContext,
                                               gatherer,
                                               DrawTypeFlags::kSimpleShape,
                                               /* withPrimitiveBlender= */ false,
                                               Coverage::kSingleChannel,
                                               processCombination);
}

} // anonymous namespace

void PaintOptions::buildCombinations(
        const KeyContext& keyContext,
        PipelineDataGatherer* gatherer,
        DrawTypeFlags drawTypes,
        bool withPrimitiveBlender,
        Coverage coverage,
        const ProcessCombination& processCombination) const {

    PaintParamsKeyBuilder builder(keyContext.dict());

    if (!fImageFilterOptions.empty() || !fMaskFilterOptions.empty()) {
        // TODO: split this out into a create_restore_draw_pipelines method
        PaintOptions tmp = *this;

        // When image filtering, the original blend mode is taken over by the restore paint
        tmp.setImageFilters({});
        tmp.setMaskFilters({});
        tmp.addBlendMode(SkBlendMode::kSrcOver);

        if (!fImageFilterOptions.empty()) {
            std::vector<sk_sp<PrecompileColorFilter>> newCFs(tmp.fColorFilterOptions.begin(),
                                                             tmp.fColorFilterOptions.end());
            if (newCFs.empty()) {
                // TODO: I (robertphillips) believe this is unnecessary and is just a result of the
                // base SkPaint generated in the PaintParamsKeyTest not correctly taking CFIFs into
                // account.
                newCFs.push_back(nullptr);
            }

            // As in SkCanvasPriv::ImageToColorFilter, we fuse CFIFs into the base draw's CFs.
            // TODO: in SkCanvasPriv::ImageToColorFilter this fusing of CFIFs and CFs is skipped
            // when there is a maskfilter. For now we over-generate.
            for (const sk_sp<PrecompileImageFilter>& o : fImageFilterOptions) {
                sk_sp<PrecompileColorFilter> imageFiltersCF = o ? o->asAColorFilter() : nullptr;
                if (imageFiltersCF) {
                    if (!tmp.fColorFilterOptions.empty()) {
                        for (const sk_sp<PrecompileColorFilter>& cf : tmp.fColorFilterOptions) {
                            // TODO: if a CFIF was fully handled here it should be removed from the
                            // later loop over fImageFilterOptions. For now we over-generate.
                            sk_sp<PrecompileColorFilter> newCF = imageFiltersCF->makeComposed(cf);
                            newCFs.push_back(std::move(newCF));
                        }
                    } else {
                        newCFs.push_back(imageFiltersCF);
                    }
                }
            }

            tmp.setColorFilters(newCFs);
        }

        tmp.buildCombinations(keyContext, gatherer, drawTypes, withPrimitiveBlender, coverage,
                              processCombination);

        create_image_drawing_pipelines(keyContext, gatherer, processCombination, *this);

        for (const sk_sp<PrecompileImageFilter>& o : fImageFilterOptions) {
            o->createPipelines(keyContext, gatherer, processCombination);
        }
        for (const sk_sp<PrecompileMaskFilter>& o : fMaskFilterOptions) {
            o->createPipelines(keyContext, gatherer, processCombination);
        }
    } else {
        int numCombinations = this->numCombinations();
        for (int i = 0; i < numCombinations; ++i) {
            // Since the precompilation path's uniforms aren't used and don't change the key,
            // the exact layout doesn't matter
            gatherer->resetWithNewLayout(Layout::kMetal);

            this->createKey(keyContext, &builder, gatherer, i, withPrimitiveBlender, coverage);

            // The 'findOrCreate' calls lockAsKey on builder and then destroys the returned
            // PaintParamsKey. This serves to reset the builder.
            UniquePaintParamsID paintID = keyContext.dict()->findOrCreate(&builder);

            processCombination(paintID, drawTypes, withPrimitiveBlender, coverage);
        }
    }
}

} // namespace skgpu::graphite
