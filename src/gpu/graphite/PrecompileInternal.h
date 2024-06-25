/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PrecompileInternal_DEFINED
#define skgpu_graphite_PrecompileInternal_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/PrecompileBase.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"

#include <functional>
#include <optional>
#include <vector>

class SkRuntimeEffect;

namespace skgpu::graphite {

enum class Coverage;
class Context;


class UniquePaintParamsID;

// Create the Pipelines specified by 'options' by combining the shading portion w/ the specified
// 'drawTypes' and a stock set of RenderPass descriptors (e.g., kDepth+msaa, kDepthStencil+msaa)
void PrecompileCombinations(Context* context,
                            const PaintOptions& options,
                            const KeyContext& keyContext,
                            DrawTypeFlags drawTypes,
                            bool withPrimitiveBlender,
                            Coverage coverage);

//--------------------------------------------------------------------------------------------------
class PrecompileMaskFilter : public PrecompileBase {
public:
    PrecompileMaskFilter() : PrecompileBase(Type::kMaskFilter) {}

private:
    friend class PaintOptions;  // for createPipelines() access

    // The PrecompileMaskFilter classes do not use the PrecompileBase::addToKey virtual since
    // they, in general, do not themselves contribute to a given SkPaint/Pipeline but, rather,
    // create separate SkPaints/Pipelines from whole cloth (in createPipelines).
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const final {
        SkASSERT(false);
    }

    virtual void createPipelines(const KeyContext&,
                                 PipelineDataGatherer*,
                                 const PaintOptionsPriv::ProcessCombination&) const = 0;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PrecompileInternal_DEFINED
