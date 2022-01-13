/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMallocPixelRef.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkKeyBuilder.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkBitmapProcShader.h"
#include "src/shaders/SkImageShader.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/SkTransformShader.h"

#if SK_SUPPORT_GPU
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#endif

#ifdef SK_ENABLE_SKSL
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkPaintParamsKey.h"
#endif

SkShaderBase::SkShaderBase(const SkMatrix* localMatrix)
    : fLocalMatrix(localMatrix ? *localMatrix : SkMatrix::I()) {
    // Pre-cache so future calls to fLocalMatrix.getType() are threadsafe.
    (void)fLocalMatrix.getType();
}

SkShaderBase::~SkShaderBase() {}

void SkShaderBase::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    bool hasLocalM = !fLocalMatrix.isIdentity();
    buffer.writeBool(hasLocalM);
    if (hasLocalM) {
        buffer.writeMatrix(fLocalMatrix);
    }
}

SkTCopyOnFirstWrite<SkMatrix>
SkShaderBase::totalLocalMatrix(const SkMatrix* preLocalMatrix) const {
    SkTCopyOnFirstWrite<SkMatrix> m(fLocalMatrix);

    if (preLocalMatrix) {
        m.writable()->preConcat(*preLocalMatrix);
    }

    return m;
}

bool SkShaderBase::computeTotalInverse(const SkMatrix& ctm,
                                       const SkMatrix* outerLocalMatrix,
                                       SkMatrix* totalInverse) const {
    return SkMatrix::Concat(ctm, *this->totalLocalMatrix(outerLocalMatrix)).invert(totalInverse);
}

bool SkShaderBase::asLuminanceColor(SkColor* colorPtr) const {
    SkColor storage;
    if (nullptr == colorPtr) {
        colorPtr = &storage;
    }
    if (this->onAsLuminanceColor(colorPtr)) {
        *colorPtr = SkColorSetA(*colorPtr, 0xFF);   // we only return opaque
        return true;
    }
    return false;
}

SkShaderBase::Context* SkShaderBase::makeContext(const ContextRec& rec, SkArenaAlloc* alloc) const {
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    // We always fall back to raster pipeline when perspective is present.
    if (rec.fMatrix->hasPerspective() ||
        fLocalMatrix.hasPerspective() ||
        (rec.fLocalMatrix && rec.fLocalMatrix->hasPerspective()) ||
        !this->computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, nullptr)) {
        return nullptr;
    }

    return this->onMakeContext(rec, alloc);
#else
    return nullptr;
#endif
}

SkShaderBase::Context::Context(const SkShaderBase& shader, const ContextRec& rec)
    : fShader(shader), fCTM(*rec.fMatrix)
{
    // We should never use a context with perspective.
    SkASSERT(!rec.fMatrix->hasPerspective());
    SkASSERT(!rec.fLocalMatrix || !rec.fLocalMatrix->hasPerspective());
    SkASSERT(!shader.getLocalMatrix().hasPerspective());

    // Because the context parameters must be valid at this point, we know that the matrix is
    // invertible.
    SkAssertResult(fShader.computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, &fTotalInverse));

    fPaintAlpha = rec.fPaintAlpha;
}

SkShaderBase::Context::~Context() {}

bool SkShaderBase::ContextRec::isLegacyCompatible(SkColorSpace* shaderColorSpace) const {
    // In legacy pipelines, shaders always produce premul (or opaque) and the destination is also
    // always premul (or opaque).  (And those "or opaque" caveats won't make any difference here.)
    SkAlphaType shaderAT = kPremul_SkAlphaType,
                   dstAT = kPremul_SkAlphaType;
    return 0 == SkColorSpaceXformSteps{shaderColorSpace, shaderAT,
                                         fDstColorSpace,    dstAT}.flags.mask();
}

SkImage* SkShader::isAImage(SkMatrix* localMatrix, SkTileMode xy[2]) const {
    return as_SB(this)->onIsAImage(localMatrix, xy);
}

SkShader::GradientType SkShader::asAGradient(GradientInfo* info) const {
    return kNone_GradientType;
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkShaderBase::asFragmentProcessor(const GrFPArgs&) const {
    return nullptr;
}
#endif

sk_sp<SkShader> SkShaderBase::makeAsALocalMatrixShader(SkMatrix*) const {
    return nullptr;
}

SkUpdatableShader* SkShaderBase::updatableShader(SkArenaAlloc* alloc) const {
    if (auto updatable = this->onUpdatableShader(alloc)) {
        return updatable;
    }

    return alloc->make<SkTransformShader>(*as_SB(this));
}

SkUpdatableShader* SkShaderBase::onUpdatableShader(SkArenaAlloc* alloc) const {
    return nullptr;
}

#ifdef SK_ENABLE_SKSL
// TODO: add implementations for derived classes
void SkShaderBase::addToKey(const SkKeyContext& keyContext,
                            SkPaintParamsKeyBuilder* builder,
                            SkPipelineDataGatherer* gatherer) const {
    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, {1, 0, 0, 1});
    builder->endBlock();
}
#endif

sk_sp<SkShader> SkBitmap::makeShader(SkTileMode tmx, SkTileMode tmy,
                                     const SkSamplingOptions& sampling,
                                     const SkMatrix* lm) const {
    if (lm && !lm->invert(nullptr)) {
        return nullptr;
    }
    return SkImageShader::Make(SkMakeImageFromRasterBitmap(*this, kIfMutable_SkCopyPixelsMode),
                               tmx, tmy, sampling, lm);
}

bool SkShaderBase::appendStages(const SkStageRec& rec) const {
    return this->onAppendStages(rec);
}

bool SkShaderBase::onAppendStages(const SkStageRec& rec) const {
    // SkShader::Context::shadeSpan() handles the paint opacity internally,
    // but SkRasterPipelineBlitter applies it as a separate stage.
    // We skip the internal shadeSpan() step by forcing the paint opaque.
    SkTCopyOnFirstWrite<SkPaint> opaquePaint(rec.fPaint);
    if (rec.fPaint.getAlpha() != SK_AlphaOPAQUE) {
        opaquePaint.writable()->setAlpha(SK_AlphaOPAQUE);
    }

    ContextRec cr(*opaquePaint, rec.fMatrixProvider.localToDevice(), rec.fLocalM, rec.fDstColorType,
                  sk_srgb_singleton());

    struct CallbackCtx : SkRasterPipeline_CallbackCtx {
        sk_sp<const SkShader> shader;
        Context*              ctx;
    };
    auto cb = rec.fAlloc->make<CallbackCtx>();
    cb->shader = sk_ref_sp(this);
    cb->ctx = as_SB(this)->makeContext(cr, rec.fAlloc);
    cb->fn  = [](SkRasterPipeline_CallbackCtx* self, int active_pixels) {
        auto c = (CallbackCtx*)self;
        int x = (int)c->rgba[0],
            y = (int)c->rgba[1];
        SkPMColor tmp[SkRasterPipeline_kMaxStride];
        c->ctx->shadeSpan(x,y, tmp, active_pixels);

        for (int i = 0; i < active_pixels; i++) {
            auto rgba_4f = SkPMColor4f::FromPMColor(tmp[i]);
            memcpy(c->rgba + 4*i, rgba_4f.vec(), 4*sizeof(float));
        }
    };

    if (cb->ctx) {
        rec.fPipeline->append(SkRasterPipeline::seed_shader);
        rec.fPipeline->append(SkRasterPipeline::callback, cb);
        rec.fAlloc->make<SkColorSpaceXformSteps>(sk_srgb_singleton(), kPremul_SkAlphaType,
                                                 rec.fDstCS,          kPremul_SkAlphaType)
            ->apply(rec.fPipeline);
        return true;
    }
    return false;
}

skvm::Color SkShaderBase::program(skvm::Builder* builder,
                                  skvm::Coord device,
                                  skvm::Coord local,
                                  skvm::Color paint,
                                  const SkMatrixProvider& matrices,
                                  const SkMatrix* localM,
                                  const SkColorInfo& dstInfo,
                                  skvm::UniformsAllocator* uniforms) const {
    // Shader subclasses should always act as if the destination were premul or opaque.
    // SkVMBlitter handles all the coordination of unpremul itself, via premul.
    SkColorInfo tweaked = dstInfo.alphaType() == kUnpremul_SkAlphaType
                           ? dstInfo.makeAlphaType(kPremul_SkAlphaType)
                           : dstInfo;

    // Force opaque alpha for all opaque shaders.
    //
    // This is primarily nice in that we usually have a 1.0f constant splat
    // somewhere in the program anyway, and this will let us drop the work the
    // shader notionally does to produce alpha, p->extract(...), etc. in favor
    // of that simple hoistable splat.
    //
    // TODO(skia:12824): Rewrite this comment to be correct?
    // More subtly, it makes isOpaque() a parameter to all shader program
    // generation, guaranteeing that is-opaque bit is mixed into the overall
    // shader program hash and blitter Key.  This makes it safe for us to use
    // that bit to make decisions when constructing an SkVMBlitter, like doing
    // SrcOver -> Src strength reduction.
    if (auto color = this->onProgram(
                builder, device, local, paint, matrices, localM, dstInfo, uniforms)) {
        if (this->isOpaque()) {
            color.a = builder->splat(1.0f);
        }
        return color;
    }
    return {};
}

bool SkShaderBase::keyAndUniforms(SkKeyBuilder* key,
                                  skvm::UniformsData* uniforms,
                                  const SkMatrixProvider& matrices,
                                  const SkMatrix* localM,
                                  const SkColorInfo& dstInfo,
                                  SkArenaAlloc* alloc) const {
    // Use the type name (pointer) as our type identifier.
    // TODO(skia:12824): Replace this with something stable and cheaper.
    const char* typeName = this->getTypeName();
    SkASSERT(typeName);
    key->addBytes(sizeof(typeName), &typeName, typeName);

    key->addBool(this->isOpaque(), "opaque");
    return this->onKeyAndUniforms(key, uniforms, matrices, localM, dstInfo, alloc);
}

// need a cheap way to invert the alpha channel of a shader (i.e. 1 - a)
sk_sp<SkShader> SkShaderBase::makeInvertAlpha() const {
    return this->makeWithColorFilter(SkColorFilters::Blend(0xFFFFFFFF, SkBlendMode::kSrcOut));
}

skvm::Coord SkShaderBase::ApplyMatrix(skvm::Builder* p,
                                      const SkMatrix& m,
                                      skvm::Coord coord,
                                      skvm::UniformsAllocator* uniforms) {
    skvm::F32 x = coord.x,
              y = coord.y;
    if (m.isIdentity()) {
        // That was easy.
    } else if (m.isTranslate()) {
        x = p->add(x, p->uniformF(uniforms->pushF()));
        y = p->add(y, p->uniformF(uniforms->pushF()));
    } else if (m.isScaleTranslate()) {
        skvm::F32 m0 = p->uniformF(uniforms->pushF()),
                  m2 = p->uniformF(uniforms->pushF()),
                  m4 = p->uniformF(uniforms->pushF()),
                  m5 = p->uniformF(uniforms->pushF());
        x = p->mad(x, m0, m2);
        y = p->mad(y, m4, m5);
    } else {  // Affine or perspective.
        skvm::F32 m_[9];
        for (int i = 0; i < 9; i++) {
            m_[i] = p->uniformF(uniforms->pushF());
        }
        auto dot = [&,x,y](int row) {
            return p->mad(x, m_[3*row+0],
                   p->mad(y, m_[3*row+1],
                             m_[3*row+2]));
        };
        x = dot(0);
        y = dot(1);
        if (m.hasPerspective()) {
            x = x * (1.0f / dot(2));
            y = y * (1.0f / dot(2));
        }
    }
    return {x,y};
}

void SkShaderBase::MatrixKeyAndUniforms(SkKeyBuilder* key,
                                        skvm::UniformsData* uniforms,
                                        const SkMatrix& m) {
    uint32_t matrixType = 0;
    if (m.isIdentity()) {
        // That was easy.
    } else if (m.isTranslate()) {
        matrixType = 1;
        uniforms->pushF(m[2]);
        uniforms->pushF(m[5]);
    } else if (m.isScaleTranslate()) {
        matrixType = 2;
        uniforms->pushF(m[0]);
        uniforms->pushF(m[2]);
        uniforms->pushF(m[4]);
        uniforms->pushF(m[5]);
    } else {
        matrixType = 3;
        if (m.hasPerspective()) {
            matrixType = 4;
        }
        for (int i = 0; i < 9; i++) {
            uniforms->pushF(m[i]);
        }
    }
    key->addBits(3, matrixType, "matrixType");
}
