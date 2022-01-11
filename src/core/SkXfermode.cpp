/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/private/SkOnce.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/core/SkXfermodePriv.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/GrCustomXfermode.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkProcCoeffXfermode : public SkXfermode {
public:
    SkProcCoeffXfermode(SkBlendMode mode) : fMode(mode) {}

    void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                const SkAlpha aa[]) const override {
        SkASSERT(dst && src && count >= 0);

        SkRasterPipeline_<256> p;

        SkRasterPipeline_MemoryCtx dst_ctx = { (void*)dst, 0 },
                                   src_ctx = { (void*)src, 0 },
                                    aa_ctx = { (void*)aa,  0 };

        p.append_load    (kN32_SkColorType, &src_ctx);
        p.append_load_dst(kN32_SkColorType, &dst_ctx);

        if (SkBlendMode_ShouldPreScaleCoverage(fMode, /*rgb_coverage=*/false)) {
            if (aa) {
                p.append(SkRasterPipeline::scale_u8, &aa_ctx);
            }
            SkBlendMode_AppendStages(fMode, &p);
        } else {
            SkBlendMode_AppendStages(fMode, &p);
            if (aa) {
                p.append(SkRasterPipeline::lerp_u8, &aa_ctx);
            }
        }

        p.append_store(kN32_SkColorType, &dst_ctx);
        p.run(0, 0, count,1);
    }

private:
    const SkBlendMode fMode;

    using INHERITED = SkXfermode;
};

const char* SkTileMode_Name(SkTileMode tm) {
    static_assert((int) SkTileMode::kClamp  == 0);
    static_assert((int) SkTileMode::kRepeat == 1);
    static_assert((int) SkTileMode::kMirror == 2);
    static_assert((int) SkTileMode::kDecal  == 3);
    static_assert(kSkTileModeCount == 4);

    const char* kStrings[kSkTileModeCount] = { "kClamp", "kRepeat", "kMirror", "kDecal" };
    return kStrings[(int) tm];
}

const char* SkBlendMode_Name(SkBlendMode bm) {
    static_assert(static_cast<int>(SkBlendMode::kClear)       == 0);
    static_assert(static_cast<int>(SkBlendMode::kSrc)         == 1);
    static_assert(static_cast<int>(SkBlendMode::kDst)         == 2);
    static_assert(static_cast<int>(SkBlendMode::kSrcOver)     == 3);
    static_assert(static_cast<int>(SkBlendMode::kDstOver)     == 4);
    static_assert(static_cast<int>(SkBlendMode::kSrcIn)       == 5);
    static_assert(static_cast<int>(SkBlendMode::kDstIn)       == 6);
    static_assert(static_cast<int>(SkBlendMode::kSrcOut)      == 7);
    static_assert(static_cast<int>(SkBlendMode::kDstOut)      == 8);
    static_assert(static_cast<int>(SkBlendMode::kSrcATop)     == 9);
    static_assert(static_cast<int>(SkBlendMode::kDstATop)     == 10);
    static_assert(static_cast<int>(SkBlendMode::kXor)         == 11);
    static_assert(static_cast<int>(SkBlendMode::kPlus)        == 12);
    static_assert(static_cast<int>(SkBlendMode::kModulate)    == 13);
    static_assert(static_cast<int>(SkBlendMode::kScreen)      == 14);

    static_assert(static_cast<int>(SkBlendMode::kOverlay)     == 15);
    static_assert(static_cast<int>(SkBlendMode::kDarken)      == 16);
    static_assert(static_cast<int>(SkBlendMode::kLighten)     == 17);
    static_assert(static_cast<int>(SkBlendMode::kColorDodge)  == 18);
    static_assert(static_cast<int>(SkBlendMode::kColorBurn)   == 19);
    static_assert(static_cast<int>(SkBlendMode::kHardLight)   == 20);
    static_assert(static_cast<int>(SkBlendMode::kSoftLight)   == 21);
    static_assert(static_cast<int>(SkBlendMode::kDifference)  == 22);
    static_assert(static_cast<int>(SkBlendMode::kExclusion)   == 23);
    static_assert(static_cast<int>(SkBlendMode::kMultiply)    == 24);

    static_assert(static_cast<int>(SkBlendMode::kHue)         == 25);
    static_assert(static_cast<int>(SkBlendMode::kSaturation)  == 26);
    static_assert(static_cast<int>(SkBlendMode::kColor)       == 27);
    static_assert(static_cast<int>(SkBlendMode::kLuminosity)  == 28);

    const char* kStrings[kSkBlendModeCount] = {
            "Clear",     "Src",        "Dst",        "SrcOver",    "DstOver",
            "SrcIn",     "DstIn",      "SrcOut",     "DstOut",     "SrcATop",
            "DstATop",   "Xor",        "Plus",       "Modulate",   "Screen",

            "Overlay",   "Darken",     "Lighten",    "ColorDodge", "ColorBurn",
            "HardLight", "SoftLight",  "Difference", "Exclusion",  "Multiply",

            "Hue",       "Saturation", "Color",      "Luminosity"
    };

    return kStrings[(int) bm];
}

sk_sp<SkXfermode> SkXfermode::Make(SkBlendMode mode) {
    if ((unsigned)mode > (unsigned)SkBlendMode::kLastMode) {
        // report error
        return nullptr;
    }

    // Skia's "default" mode is srcover. nullptr in SkPaint is interpreted as srcover
    // so we can just return nullptr from the factory.
    if (SkBlendMode::kSrcOver == mode) {
        return nullptr;
    }

    const int COUNT_BLENDMODES = (int)SkBlendMode::kLastMode + 1;

    static SkOnce        once[COUNT_BLENDMODES];
    static SkXfermode* cached[COUNT_BLENDMODES];

    once[(int)mode]([mode] {
        if (auto xfermode = SkOpts::create_xfermode(mode)) {
            cached[(int)mode] = xfermode;
        } else {
            cached[(int)mode] = new SkProcCoeffXfermode(mode);
        }
    });
    return sk_ref_sp(cached[(int)mode]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkXfermode::IsOpaque(SkBlendMode mode, SrcColorOpacity opacityType) {
    SkBlendModeCoeff src, dst;
    if (!SkBlendMode_AsCoeff(mode, &src, &dst)) {
        return false;
    }

    switch (src) {
        case SkBlendModeCoeff::kDA:
        case SkBlendModeCoeff::kDC:
        case SkBlendModeCoeff::kIDA:
        case SkBlendModeCoeff::kIDC:
            return false;
        default:
            break;
    }

    switch (dst) {
        case SkBlendModeCoeff::kZero:
            return true;
        case SkBlendModeCoeff::kISA:
            return kOpaque_SrcColorOpacity == opacityType;
        case SkBlendModeCoeff::kSA:
            return kTransparentBlack_SrcColorOpacity == opacityType ||
            kTransparentAlpha_SrcColorOpacity == opacityType;
        case SkBlendModeCoeff::kSC:
            return kTransparentBlack_SrcColorOpacity == opacityType;
        default:
            return false;
    }
    return false;
}

#if SK_SUPPORT_GPU
const GrXPFactory* SkBlendMode_AsXPFactory(SkBlendMode mode) {
    if (SkBlendMode_AsCoeff(mode, nullptr, nullptr)) {
        const GrXPFactory* result = GrPorterDuffXPFactory::Get(mode);
        SkASSERT(result);
        return result;
    }

    SkASSERT(GrCustomXfermode::IsSupportedMode(mode));
    return GrCustomXfermode::Get(mode);
}
#endif
