/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/private/SkPaintParamsKey.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkColorShader.h"
#include "src/shaders/SkComposeShader.h"

#include "experimental/graphite/include/Encoding.h"

namespace {

struct LocalMatrixStageRec final : public SkStageRec {
    LocalMatrixStageRec(const SkStageRec& rec, const SkMatrix& lm)
        : INHERITED(rec) {
        if (!lm.isIdentity()) {
            if (fLocalM) {
                fStorage.setConcat(lm, *fLocalM);
                fLocalM = fStorage.isIdentity() ? nullptr : &fStorage;
            } else {
                fLocalM = &lm;
            }
        }
    }

private:
    SkMatrix fStorage;

    using INHERITED = SkStageRec;
};

} // namespace

sk_sp<SkShader> SkShaders::Blend(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src) {
    if (!src || !dst) {
        return nullptr;
    }
    switch (mode) {
        case SkBlendMode::kClear: return Color(0);
        case SkBlendMode::kDst:   return dst;
        case SkBlendMode::kSrc:   return src;
        default: break;
    }
    return sk_sp<SkShader>(new SkShader_Blend(mode, std::move(dst), std::move(src)));
}

sk_sp<SkShader> SkShaders::Blend(sk_sp<SkBlender> blender, sk_sp<SkShader> dst, sk_sp<SkShader> src) {
    if (!src || !dst) {
        return nullptr;
    }
    if (!blender) {
        return SkShaders::Blend(SkBlendMode::kSrcOver, std::move(dst), std::move(src));
    }
    return sk_sp<SkShader>(new SkShader_Blend(std::move(blender), std::move(dst), std::move(src)));
}

///////////////////////////////////////////////////////////////////////////////

/**************************************************************************************************/
int SkShader_Blend::Encode1(skgpu::ShaderCodeDictionary* dict,
                            SkBackend backend,
                            SkPaintParamsKey* key,
                            const BlendData& data) {
    key->addInt1((int) data.fBM);
    int numDstInts = data.fDst->addToKeyS(dict, backend, key, nullptr);
    int numSrcInts = data.fSrc->addToKeyS(dict, backend, key, nullptr);
    return 1 + numDstInts + numSrcInts;
}

#if defined(SK_DEBUG) || GR_TEST_UTILS || defined(SK_ENABLE_DUMP_GPU)
void SkShader_Blend::Dump(const uint32_t* data, int numIntsAccessible) {
    SkASSERT(CodeSnippetID::kBlendShader == ExtractCodeSnippetID(data[0]));

    SkBlendMode bm = ExtractBlendMode(data[1]);
    SkDebugf("BlendMode: %s\n", SkBlendMode_Name(bm));
    SkDebugf("\nFirst:  ");
    int numConsumed1 = SkPaintParamsKey::DumpFrom(&data[2]);
    SkASSERT(1+numConsumed1 < numIntsAccessible);
    SkDebugf("Second: ");
    int numConsumed2 = SkPaintParamsKey::DumpFrom(&data[numConsumed1+2]);
    SkASSERT(2+numConsumed1+numConsumed2 <= numIntsAccessible);
}
#endif

int SkShader_Blend::addToKeyS(skgpu::ShaderCodeDictionary* dict,
                              SkBackend backend,
                              SkPaintParamsKey* key,
                              std::vector<sk_sp<skgpu::UniformData>>*) const {
    if (!fBlender) {

    }

    int index = key->writeBlockHeader(CodeSnippetID::kBlendShader);

    BlendData blendData { fDst.get(), fSrc.get(), fMode };

    // Maybe add a getCurIndex call and do this math independently?
    int size = Encode1(dict, backend, key, blendData);

    key->patchBlockHeader(index, CodeSnippetID::kBlendShader, size);

    return size + 1;
}
/**************************************************************************************************/

SkShader_Blend::SkShader_Blend(sk_sp<SkBlender> blender, sk_sp<SkShader> dst, sk_sp<SkShader> src)
        : fDst(std::move(dst))
        , fSrc(std::move(src))
        , fBlender(std::move(blender))
        , fMode((SkBlendMode)kCustom_SkBlendMode) {
    if (skstd::optional<SkBlendMode> bm = as_BB(fBlender)->asBlendMode(); bm.has_value()) {
        fMode = *bm;
        fBlender.reset();
    }
}

sk_sp<SkFlattenable> SkShader_Blend::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> dst(buffer.readShader());
    sk_sp<SkShader> src(buffer.readShader());
    if (!buffer.validate(dst && src)) {
        return nullptr;
    }

    sk_sp<SkBlender> blender(nullptr);
    unsigned        mode = buffer.read32();

    if (mode == kCustom_SkBlendMode) {
        blender = buffer.readBlender();
        if (buffer.validate(blender != nullptr)) {
            return SkShaders::Blend(std::move(blender), std::move(dst), std::move(src));
        }
    } else {
        if (buffer.validate(mode <= (unsigned)SkBlendMode::kLastMode)) {
            return SkShaders::Blend(static_cast<SkBlendMode>(mode), std::move(dst), std::move(src));
        }
    }
    return nullptr;
}

void SkShader_Blend::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fDst.get());
    buffer.writeFlattenable(fSrc.get());
    if (fBlender) {
        buffer.write32(kCustom_SkBlendMode);
        buffer.writeFlattenable(fBlender.get());
    } else {
        buffer.write32((int)fMode);
    }
}

// Returns the output of e0, and leaves the output of e1 in r,g,b,a
static float* append_two_shaders(const SkStageRec& rec, SkShader* s0, SkShader* s1) {
    struct Storage {
        float   fRes0[4 * SkRasterPipeline_kMaxStride];
    };
    auto storage = rec.fAlloc->make<Storage>();

    if (!as_SB(s0)->appendStages(rec)) {
        return nullptr;
    }
    rec.fPipeline->append(SkRasterPipeline::store_src, storage->fRes0);

    if (!as_SB(s1)->appendStages(rec)) {
        return nullptr;
    }
    return storage->fRes0;
}

bool SkShader_Blend::onAppendStages(const SkStageRec& orig_rec) const {
    if (fBlender) {
        return false;
    }

    const LocalMatrixStageRec rec(orig_rec, this->getLocalMatrix());

    float* res0 = append_two_shaders(rec, fDst.get(), fSrc.get());
    if (!res0) {
        return false;
    }

    rec.fPipeline->append(SkRasterPipeline::load_dst, res0);
    SkBlendMode_AppendStages(fMode, rec.fPipeline);
    return true;
}

skvm::Color SkShader_Blend::onProgram(skvm::Builder* p,
                                      skvm::Coord device, skvm::Coord local, skvm::Color paint,
                                      const SkMatrixProvider& mats, const SkMatrix* localM,
                                      const SkColorInfo& cinfo,
                                      skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::Color d,s;
    if ((d = as_SB(fDst)->program(p, device,local, paint, mats,localM, cinfo, uniforms,alloc)) &&
        (s = as_SB(fSrc)->program(p, device,local, paint, mats,localM, cinfo, uniforms,alloc)))
    {
        if (fBlender) {
            return as_BB(fBlender)->program(p, s,d, cinfo, uniforms,alloc);
        } else {
            return p->blend(fMode, s,d);
        }
    }
    return {};
}

#if SK_SUPPORT_GPU

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/GrBlendFragmentProcessor.h"

std::unique_ptr<GrFragmentProcessor> SkShader_Blend::asFragmentProcessor(
        const GrFPArgs& orig_args) const {
    GrFPArgs::WithPreLocalMatrix args(orig_args, this->getLocalMatrix());
    auto fpA = as_SB(fDst)->asFragmentProcessor(args);
    auto fpB = as_SB(fSrc)->asFragmentProcessor(args);
    if (!fpA || !fpB) {
        // This is unexpected. Both src and dst shaders should be valid. Just fail.
        return nullptr;
    }
    if (fBlender) {
        return as_BB(fBlender)->asFragmentProcessor(std::move(fpB), std::move(fpA), orig_args);
    } else {
        return GrBlendFragmentProcessor::Make(std::move(fpB), std::move(fpA), fMode);
    }
}
#endif
