/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkLayerMaskFilter.h"
#include "src/core/SkMaskFilterBase.h"

class SkLayerMF : public SkMaskFilterBase {
    std::vector<Layer> fLayers;

public:
    SkLayerMF(const Layer layers[], int count) : fLayers(layers, layers + count) {
    }

    SkMask::Format getFormat() const override { return SkMask::kA8_Format; }

    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

    void computeFastBounds(const SkRect& src, SkRect* dst) const override {
        *dst = src;
    }

    bool asABlur(BlurRec*) const override { return false; }

private:
    SK_FLATTENABLE_HOOKS(SkLayerMF)

    SkLayerMF(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;

    using INHERITED = SkMaskFilterBase;
};

sk_sp<SkFlattenable> SkShaderMF::CreateProc(SkReadBuffer& buffer) {
    return SkShaderMaskFilter::Make(buffer.readShader());
}

void SkShaderMF::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader.get());
}

static void rect_memcpy(void* dst, size_t dstRB, const void* src, size_t srcRB,
                        size_t copyBytes, int rows) {
    for (int i = 0; i < rows; ++i) {
        memcpy(dst, src, copyBytes);
        dst = (char*)dst + dstRB;
        src = (const char*)src + srcRB;
    }
}

//void SkShaderMaskFilter::RegisterFlattenables() { SK_REGISTER_FLATTENABLE(SkShaderMF); }


sk_sp<SkMaskFilter> SkLayerMaskFilter::Make(const Layer layers[], int count) {
    if (count < 1) {
        return nullptr;
    }
    return sk_sp<SkMaskFilter>(new SkLayerMF(layers, count);
}
