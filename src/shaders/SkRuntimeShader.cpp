/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/shaders/SkRuntimeShader.h"

#include "include/core/SkCapabilities.h"
#include "include/core/SkData.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/sksl/SkSLDebugTrace.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#if defined(SK_BUILD_FOR_DEBUGGER)
    constexpr bool kLenientSkSLDeserialization = true;
#else
    constexpr bool kLenientSkSLDeserialization = false;
#endif

class SkColorSpace;
struct SkIPoint;

SkRuntimeShader::SkRuntimeShader(sk_sp<SkRuntimeEffect> effect,
                                 sk_sp<SkSL::DebugTracePriv> debugTrace,
                                 sk_sp<const SkData> uniforms,
                                 SkSpan<const SkRuntimeEffect::ChildPtr> children)
        : fEffect(std::move(effect))
        , fDebugTrace(std::move(debugTrace))
        , fUniformData(std::move(uniforms))
        , fChildren(children.begin(), children.end()) {}

SkRuntimeShader::SkRuntimeShader(sk_sp<SkRuntimeEffect> effect,
                                 sk_sp<SkSL::DebugTracePriv> debugTrace,
                                 UniformsCallback uniformsCallback,
                                 SkSpan<const SkRuntimeEffect::ChildPtr> children)
        : fEffect(std::move(effect))
        , fDebugTrace(std::move(debugTrace))
        , fUniformsCallback(std::move(uniformsCallback))
        , fChildren(children.begin(), children.end()) {}

static sk_sp<SkSL::DebugTracePriv> make_debug_trace(SkRuntimeEffect* effect,
                                                    const SkIPoint& coord) {
    auto debugTrace = sk_make_sp<SkSL::DebugTracePriv>();
    debugTrace->setSource(effect->source());
    debugTrace->setTraceCoord(coord);
    return debugTrace;
}

SkRuntimeEffect::TracedShader SkRuntimeShader::makeTracedClone(const SkIPoint& coord) {
    sk_sp<SkRuntimeEffect> unoptimized = fEffect->makeUnoptimizedClone();
    sk_sp<SkSL::DebugTracePriv> debugTrace = make_debug_trace(unoptimized.get(), coord);
    auto debugShader = sk_make_sp<SkRuntimeShader>(
            unoptimized, debugTrace, this->uniformData(nullptr), SkSpan(fChildren));

    return SkRuntimeEffect::TracedShader{std::move(debugShader), std::move(debugTrace)};
}

bool SkRuntimeShader::isConstant(SkColor4f* color) const {
    // NOTE: We don't use SupportsConstantOutputForConstantInput because that is for SkColorFilters
    // that have a color input. A runtime effect shader will evaluate to a constant color if:
    //   1. This effect doesn't use the input coords except to pass through to children
    //   2. Every child is a shader type and reports that it's a constant color.
    //
    // In this case, all child values are constant and the varying input is ignored, so the result
    // must be constant and can be evaluated with raster pipeline.
    if (fEffect->usesSampleCoords()) {
        // Hypothetically the sample coord usage could just be explicit sampling of children that
        // are constants, but the analysis doesn't differentiate.
        return false;
    }

    // Collect all child inputs (if they are constants, otherwise exit early).
    skia_private::STArray<2, SkPMColor4f> inputColors;
    inputColors.reserve(this->children().size());
    for (auto&& child : this->children()) {
        SkColor4f c;
        if (child.type() != SkRuntimeEffect::ChildType::kShader ||
            !as_SB(child.shader())->isConstant(&c)) {
            return false;
        }

        inputColors.push_back(c.premul());
    }

    class ConstantChildShaders_SkRPCallbacks : public SkSL::RP::Callbacks {
    public:
        ConstantChildShaders_SkRPCallbacks(SkRasterPipeline* pipeline,
                                           SkArenaAlloc* alloc,
                                           SkSpan<const SkPMColor4f> inputColors)
                : fPipeline(pipeline)
                , fAlloc(alloc)
                , fInputColors(inputColors) {}

        bool appendShader(int index) override {
            // Instead of appending the child's raster pipeline (as a regular CPU runtime effect
            // program would produce), just append the constant color corresponding to `index`.
            fPipeline->appendConstantColor(fAlloc, fInputColors[index].vec());
            return true;
        }
        bool appendColorFilter(int index) override {
           SkDEBUGFAIL("constant-child-shaders unsupported when color filters present");
           return false;
        }
        bool appendBlender(int index) override {
           SkDEBUGFAIL("constant-child-shaders unsupported when blenders present");
           return false;
        }
        void toLinearSrgb(const void* color) override { /* identity color conversion */ }
        void fromLinearSrgb(const void* color) override { /* identity color conversion */ }

    private:
        SkRasterPipeline* fPipeline;
        SkArenaAlloc* fAlloc;
        SkSpan<const SkPMColor4f> fInputColors;
    };

    if (const SkSL::RP::Program* program = fEffect->getRPProgram(/*debugTrace=*/nullptr)) {
        // We want to return the unpremul color in sRGB, so color uniforms should be in sRGB.
        sk_sp<const SkData> uniformData = this->uniformData(sk_srgb_singleton());
        SkSpan<const float> uniforms{reinterpret_cast<const float*>(uniformData->data()),
                                     uniformData->size() / sizeof(float)};
        SkSTArenaAlloc<2048> alloc;  // sufficient for a tiny SkSL program
        SkRasterPipeline pipeline(&alloc);
        ConstantChildShaders_SkRPCallbacks callbacks{&pipeline, &alloc, inputColors};
        if (program->appendStages(&pipeline, &alloc, &callbacks, uniforms)) {
            SkPMColor4f outputColor;
            SkRasterPipelineContexts::MemoryCtx outputCtx = {&outputColor, 0};
            pipeline.append(SkRasterPipelineOp::store_f32, &outputCtx);
            pipeline.run(0, 0, 1, 1);

            if (color) {
                // Convert to unpremul. Note that we don't check for `color` being null earlier and
                // skip evaluating the raster pipeline. This could lead to cases where
                // `isConstant()` return true while `isConstant(color)` would not.
                *color = outputColor.unpremul();
            }
            return true;
        }
    }

    // Otherwise we weren't able to execute the Raster Pipeline program
    return false;
}

bool SkRuntimeShader::appendStages(const SkStageRec& rec, const SkShaders::MatrixRec& mRec) const {
    if (!SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(), fEffect.get())) {
        // SkRP has support for many parts of #version 300 already, but for now, we restrict its
        // usage in runtime effects to just #version 100.
        return false;
    }
    if (const SkSL::RP::Program* program = fEffect->getRPProgram(fDebugTrace.get())) {
        std::optional<SkShaders::MatrixRec> newMRec = mRec.apply(rec);
        if (!newMRec.has_value()) {
            return false;
        }
        SkSpan<const float> uniforms =
                SkRuntimeEffectPriv::UniformsAsSpan(fEffect->uniforms(),
                                                    this->uniformData(rec.fDstCS),
                                                    /*alwaysCopyIntoAlloc=*/fUniformData == nullptr,
                                                    rec.fDstCS,
                                                    rec.fAlloc);
        RuntimeEffectRPCallbacks callbacks(rec, *newMRec, fChildren, fEffect->fSampleUsages);
        bool success = program->appendStages(rec.fPipeline, rec.fAlloc, &callbacks, uniforms);
        return success;
    }
    return false;
}

sk_sp<const SkData> SkRuntimeShader::uniformData(const SkColorSpace* dstCS) const {
    if (fUniformData) {
        return fUniformData;
    }

    // We want to invoke the uniforms-callback each time a paint occurs.
    SkASSERT(fUniformsCallback);
    sk_sp<const SkData> uniforms = fUniformsCallback({dstCS});
    SkASSERT(uniforms && uniforms->size() == fEffect->uniformSize());
    return uniforms;
}

void SkRuntimeShader::flatten(SkWriteBuffer& buffer) const {
    if (SkKnownRuntimeEffects::IsSkiaKnownRuntimeEffect(fEffect->fStableKey)) {
        // We only serialize Skia-internal stableKeys. First party stable keys are not serialized.
        buffer.write32(fEffect->fStableKey);
    } else {
        buffer.write32(0);
        buffer.writeString(fEffect->source().c_str());
    }
    buffer.writeDataAsByteArray(this->uniformData(nullptr).get());
    SkRuntimeEffectPriv::WriteChildEffects(buffer, fChildren);
}

sk_sp<SkFlattenable> SkRuntimeShader::CreateProc(SkReadBuffer& buffer) {
    if (!buffer.validate(buffer.allowSkSL())) {
        return nullptr;
    }

    sk_sp<SkRuntimeEffect> effect;
    if (!buffer.isVersionLT(SkPicturePriv::kSerializeStableKeys)) {
        uint32_t candidateStableKey = buffer.readUInt();
        effect = SkKnownRuntimeEffects::MaybeGetKnownRuntimeEffect(candidateStableKey);
        if (!effect && !buffer.validate(candidateStableKey == 0)) {
            return nullptr;
        }
    }

    if (!effect) {
        SkString sksl;
        buffer.readString(&sksl);
        effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForShader, std::move(sksl));
    }
    if constexpr (!kLenientSkSLDeserialization) {
        if (!buffer.validate(effect != nullptr)) {
            return nullptr;
        }
    }

    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    SkTLazy<SkMatrix> localM;
    if (buffer.isVersionLT(SkPicturePriv::kNoShaderLocalMatrix)) {
        uint32_t flags = buffer.read32();
        if (flags & kHasLegacyLocalMatrix_Flag) {
            buffer.readMatrix(localM.init());
        }
    }

    skia_private::STArray<4, SkRuntimeEffect::ChildPtr> children;
    if (!SkRuntimeEffectPriv::ReadChildEffects(buffer, effect.get(), &children)) {
        return nullptr;
    }

    if constexpr (kLenientSkSLDeserialization) {
        if (!effect) {
            // If any children were SkShaders, return the first one. This is a reasonable fallback.
            for (int i = 0; i < children.size(); i++) {
                if (children[i].shader()) {
                    SkDebugf("Serialized SkSL failed to compile. Replacing shader with child %d.\n",
                             i);
                    return sk_ref_sp(children[i].shader());
                }
            }

            // We don't know what to do, so just return nullptr (but *don't* poison the buffer).
            SkDebugf("Serialized SkSL failed to compile. Ignoring/dropping SkSL shader.\n");
            return nullptr;
        }
    }

    return effect->makeShader(std::move(uniforms), SkSpan(children), localM.getMaybeNull());
}
