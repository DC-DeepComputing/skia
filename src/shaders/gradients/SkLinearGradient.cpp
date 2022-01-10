/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/gradients/SkLinearGradient.h"

#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/gradients/Sk4fLinearGradient.h"

#include "experimental/graphite/include/Encoding.h"
#include "include/private/SkPaintParamsKey.h"

static SkMatrix pts_to_unit_matrix(const SkPoint pts[2]) {
    SkVector    vec = pts[1] - pts[0];
    SkScalar    mag = vec.length();
    SkScalar    inv = mag ? SkScalarInvert(mag) : 0;

    vec.scale(inv);
    SkMatrix matrix;
    matrix.setSinCos(-vec.fY, vec.fX, pts[0].fX, pts[0].fY);
    matrix.postTranslate(-pts[0].fX, -pts[0].fY);
    matrix.postScale(inv, inv);
    return matrix;
}

///////////////////////////////////////////////////////////////////////////////

// TODO: For the sprint we only support 4 stops in the gradients
static constexpr int kMaxStops1 = 4;

struct GradData {
    SkColor4f fColors[kMaxStops1];
    float     fOffsets[kMaxStops1];
    SkPoint   fStartPoint;
    SkPoint   fEndPoint;
    float     fRadius0;
    float     fRadius1;
};

int SkLinearGradient::Encode1(skgpu::ShaderCodeDictionary*,
                              Mapping mapping,
                              SkPaintParamsKey* key,
                              const GradData& data) {
    switch (mapping) {
        case Mapping::kGraphite: {
            int code = 7;
//            SkASSERT(data == Decode(code));
            key->addInt1(code);
            return 1;
        }
        case Mapping::kGanesh:
        case Mapping::kSkVM:
            return 0;
    }

    SkUNREACHABLE;
}

#if defined(SK_DEBUG) || GR_TEST_UTILS || defined(SK_ENABLE_DUMP_GPU)
void SkLinearGradient::Dump(const uint32_t* data, int numIntsAccessible) {
    SkASSERT(CodeSnippetID::kBlendShader == ExtractCodeSnippetID(data[0]));

    SkBlendMode bm = ExtractBlendMode(data[1]);
    SkDebugf("BlendMode: %s\n", BlendModeToStr(bm));
    SkDebugf("\nFirst:  ");
    int numConsumed1 = SkPaintParamsKey::DumpFrom(&data[2]);
    SkASSERT(1+numConsumed1 < numIntsAccessible);
    SkDebugf("Second: ");
    int numConsumed2 = SkPaintParamsKey::DumpFrom(&data[numConsumed1+2]);
    SkASSERT(2+numConsumed1+numConsumed2 <= numIntsAccessible);
}
#endif

int SkLinearGradient::toCodeS(skgpu::ShaderCodeDictionary* dict,
                              Mapping mapping,
                              SkPaintParamsKey* key,
                              std::vector<sk_sp<skgpu::UniformData>>*) const {
    switch (mapping) {
        case Mapping::kGraphite: {
            GradData data;
            data.fStartPoint = fStart;
            data.fEndPoint = fEnd;
            data.fRadius0 = 0.0f; // unused
            data.fRadius1 = 0.0f; // unused

            int index = key->writeBlockHeader(CodeSnippetID::kLinearGradientShader);
            int size = Encode1(dict, mapping, key, data);
            key->patchBlockHeader(index, CodeSnippetID::kLinearGradientShader, size);
            return size + 1;
        }
        default:
            break;
    }
    return 0;
}

SkLinearGradient::SkLinearGradient(const SkPoint pts[2], const Descriptor& desc)
    : SkGradientShaderBase(desc, pts_to_unit_matrix(pts))
    , fStart(pts[0])
    , fEnd(pts[1]) {
}

sk_sp<SkFlattenable> SkLinearGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return nullptr;
    }
    SkPoint pts[2];
    pts[0] = buffer.readPoint();
    pts[1] = buffer.readPoint();
    return SkGradientShader::MakeLinear(pts, desc.fColors, std::move(desc.fColorSpace), desc.fPos,
                                        desc.fCount, desc.fTileMode, desc.fGradFlags,
                                        desc.fLocalMatrix);
}

void SkLinearGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fStart);
    buffer.writePoint(fEnd);
}

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
SkShaderBase::Context* SkLinearGradient::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    // make sure our colorspaces are compatible with legacy blits
    if (!rec.isLegacyCompatible(fColorSpace.get())) {
        return nullptr;
    }
    // Can't use legacy blit if we can't represent our colors as SkColors
    if (!this->colorsCanConvertToSkColor()) {
        return nullptr;
    }

    return fTileMode != SkTileMode::kDecal
        ? CheckedMakeContext<LinearGradient4fContext>(alloc, *this, rec)
        : nullptr;
}
#endif

void SkLinearGradient::appendGradientStages(SkArenaAlloc*, SkRasterPipeline*,
                                            SkRasterPipeline*) const {
    // No extra stage needed for linear gradients.
}

skvm::F32 SkLinearGradient::transformT(skvm::Builder* p, skvm::Uniforms*,
                                       skvm::Coord coord, skvm::I32* mask) const {
    // We've baked getting t in x into the matrix, so this is pretty trivial.
    return coord.x;
}

SkShader::GradientType SkLinearGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fStart;
        info->fPoint[1] = fEnd;
    }
    return kLinear_GradientType;
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "src/gpu/gradients/GrGradientShader.h"

std::unique_ptr<GrFragmentProcessor> SkLinearGradient::asFragmentProcessor(
        const GrFPArgs& args) const {
    return GrGradientShader::MakeLinear(*this, args);
}

#endif
