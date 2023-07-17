/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRefCnt.h"
#include "include/effects/SkColorMatrix.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Surface.h"

#include "modules/skcms/skcms.h"

#include "src/core/SkBlenderBase.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PaintOptionsPriv.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PublicPrecompile.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"

#include "tools/ToolUtils.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/graphite/ContextFactory.h"

// remove?!
#include "src/base/SkRandom.h"


#if 0
#include "fuzz/FuzzCommon.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkVertices.h"
#include "include/docs/SkPDFDocument.h"
#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/Sk2DPathEffect.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTo.h"
#include "include/svg/SkSVGCanvas.h"
#include "include/utils/SkNullCanvas.h"
#include "src/base/SkUTF.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/utils/SkJSONWriter.h"
#include "tools/UrlDataManager.h"
#include "tools/debugger/DebugCanvas.h"
#include "tools/flags/CommandLineFlags.h"

#if defined(SK_GANESH)
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tools/gpu/GrContextFactory.h"
#endif

#ifdef SK_GL
#include "include/gpu/gl/GrGLFunctions.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#endif

#include <iostream>
#include <utility>

using namespace skia_private;
#endif

using namespace skgpu::graphite;

namespace {

SkBlendMode random_blend_mode(SkRandom* rand) {
    return static_cast<SkBlendMode>(rand->nextULessThan(kSkBlendModeCount));
}

SkColor random_opaque_color(SkRandom* rand) {
    return 0xff000000 | rand->nextU();
}

SkColor4f random_color(SkRandom* rand) {
    SkColor4f result = { rand->nextRangeF(0.0f, 1.0f),
                         rand->nextRangeF(0.0f, 1.0f),
                         rand->nextRangeF(0.0f, 1.0f),
                         rand->nextRangeF(0.0f, 1.0f) };

    if (rand->nextBool()) {
        result.fA = 1.0f;
    }

    return result;
}

SkPath make_path() {
    SkPathBuilder path;
    path.moveTo(0, 0);
    path.lineTo(8, 2);
    path.lineTo(16, 0);
    path.lineTo(14, 8);
    path.lineTo(16, 16);
    path.lineTo(8, 14);
    path.lineTo(0, 16);
    path.lineTo(2, 8);
    path.close();
    return path.detach();
}

#ifdef SK_DEBUG
void dump(ShaderCodeDictionary* dict, UniquePaintParamsID id) {
    dict->lookup(id).dump(dict);
}
#endif

//--------------------------------------------------------------------------------------------------
// color spaces

static constexpr skcms_TransferFunction gTransferFunctions[] = {
        SkNamedTransferFn::kSRGB,
        SkNamedTransferFn::k2Dot2,
        SkNamedTransferFn::kLinear,
        SkNamedTransferFn::kRec2020,
        SkNamedTransferFn::kPQ,
        SkNamedTransferFn::kHLG,
};

static constexpr int kTransferFunctionCount = std::size(gTransferFunctions);

static constexpr skcms_Matrix3x3 gGamuts[] = {
        SkNamedGamut::kSRGB,
        SkNamedGamut::kAdobeRGB,
        SkNamedGamut::kDisplayP3,
        SkNamedGamut::kRec2020,
        SkNamedGamut::kXYZ,
};

static constexpr int kGamutCount = std::size(gGamuts);

enum class ColorSpaceType {
    kNone,
    kSRGB,
    kSRGBLinear,
    kRGB,

    kLast = kRGB
};

static constexpr int kColorSpaceTypeCount = static_cast<int>(ColorSpaceType::kLast) + 1;

ColorSpaceType random_colorspacetype(SkRandom* rand) {
    return static_cast<ColorSpaceType>(rand->nextULessThan(kColorSpaceTypeCount));
}

sk_sp<SkColorSpace> random_colorspace(SkRandom* rand) {
    ColorSpaceType cs = random_colorspacetype(rand);

    switch (cs) {
        case ColorSpaceType::kNone:
            return nullptr;
        case ColorSpaceType::kSRGB:
            return SkColorSpace::MakeSRGB();
        case ColorSpaceType::kSRGBLinear:
            return SkColorSpace::MakeSRGBLinear();
        case ColorSpaceType::kRGB:
            return SkColorSpace::MakeRGB(
                    gTransferFunctions[rand->nextULessThan(kTransferFunctionCount)],
                    gGamuts[rand->nextULessThan(kGamutCount)]);
    }

    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
// color filters

enum class ColorFilterType {
    kNone,
    kBlend,
    kMatrix,
    kHSLAMatrix,
    // TODO: add more color filters

    kLast = kHSLAMatrix
};

static constexpr int kColorFilterTypeCount = static_cast<int>(ColorFilterType::kLast) + 1;

ColorFilterType random_colorfiltertype(SkRandom* rand) {
    return static_cast<ColorFilterType>(rand->nextULessThan(kColorFilterTypeCount));
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_blend_colorfilter(
        SkRandom* rand) {

    sk_sp<SkColorFilter> cf;

    // SkColorFilters::Blend is clever and can weed out noop color filters. Loop until we get
    // a valid color filter.
    while (!cf) {
        cf = SkColorFilters::Blend(random_color(rand),
                                   random_colorspace(rand),
                                   random_blend_mode(rand));
    }

    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::Blend();

    return { cf, o };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_matrix_colorfilter() {
    sk_sp<SkColorFilter> cf = SkColorFilters::Matrix(
            SkColorMatrix::RGBtoYUV(SkYUVColorSpace::kJPEG_Full_SkYUVColorSpace));
    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::Matrix();

    return { cf, o };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_hsla_matrix_colorfilter() {
    sk_sp<SkColorFilter> cf = SkColorFilters::HSLAMatrix(
            SkColorMatrix::RGBtoYUV(SkYUVColorSpace::kJPEG_Full_SkYUVColorSpace));
    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::HSLAMatrix();

    return { cf, o };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_colorfilter(
        SkRandom* rand,
        ColorFilterType type,
        int depth) {

    switch (type) {
        case ColorFilterType::kNone:
            return { nullptr, nullptr };
        case ColorFilterType::kBlend:
            return create_blend_colorfilter(rand);
        case ColorFilterType::kMatrix:
            return create_matrix_colorfilter();
        case ColorFilterType::kHSLAMatrix:
            return create_hsla_matrix_colorfilter();
    }

    SkUNREACHABLE;
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_random_colorfilter(
        SkRandom* rand,
        int depth) {
    return create_colorfilter(rand, random_colorfiltertype(rand), depth);
}

//--------------------------------------------------------------------------------------------------
std::pair<SkPaint, PaintOptions> create_random_paint(SkRandom* rand, int depth) {
    if (depth <= 0) {
        return {};
    }

    SkPaint paint;
    paint.setColor(random_opaque_color(rand));

    PaintOptions paintOptions;

    {
        auto [cf, o] = create_random_colorfilter(rand, depth - 1);
        SkASSERT(!cf == !o);

        if (cf) {
            paint.setColorFilter(std::move(cf));
            paintOptions.setColorFilters({o});
        }
    }

    return { paint, paintOptions };
}

#if 0
static sk_sp<SkColorFilter> make_fuzz_colorfilter(Fuzz* fuzz, int depth) {
    if (depth <= 0) {
        return nullptr;
    }
    int colorFilterType;
    fuzz->nextRange(&colorFilterType, 0, 8);
    switch (colorFilterType) {
        case 0:
            return nullptr;
        case 1: {
            SkColor color;
            SkBlendMode mode;
            fuzz->next(&color);
            fuzz->nextEnum(&mode, SkBlendMode::kLastMode);
            return SkColorFilters::Blend(color, mode);
        }
        case 2: {
            sk_sp<SkColorFilter> outer = make_fuzz_colorfilter(fuzz, depth - 1);
            if (!outer) {
                return nullptr;
            }
            sk_sp<SkColorFilter> inner = make_fuzz_colorfilter(fuzz, depth - 1);
            // makeComposed should be able to handle nullptr.
            return outer->makeComposed(std::move(inner));
        }
        case 3: {
            float array[20];
            fuzz->nextN(array, std::size(array));
            return SkColorFilters::Matrix(array);
        }
        case 4: {
            SkColor mul, add;
            fuzz->next(&mul, &add);
            return SkColorFilters::Lighting(mul, add);
        }
        case 5: {
            bool grayscale;
            int invertStyle;
            float contrast;
            fuzz->next(&grayscale);
            fuzz->nextRange(&invertStyle, 0, 2);
            fuzz->nextRange(&contrast, -1.0f, 1.0f);
            return SkHighContrastFilter::Make(SkHighContrastConfig(
                    grayscale, SkHighContrastConfig::InvertStyle(invertStyle), contrast));
        }
        case 6:
            return SkLumaColorFilter::Make();
        case 7: {
            uint8_t table[256];
            fuzz->nextN(table, std::size(table));
            return SkColorFilters::Table(table);
        }
        case 8: {
            uint8_t tableA[256];
            uint8_t tableR[256];
            uint8_t tableG[256];
            uint8_t tableB[256];
            fuzz->nextN(tableA, std::size(tableA));
            fuzz->nextN(tableR, std::size(tableR));
            fuzz->nextN(tableG, std::size(tableG));
            fuzz->nextN(tableB, std::size(tableB));
            return SkColorFilters::TableARGB(tableA, tableR, tableG, tableB);
        }
        default:
            SkASSERT(false);
            break;
    }
    return nullptr;
}


static void fuzz_paint(Fuzz* fuzz, SkPaint* paint, int depth) {
    if (!fuzz || !paint || depth <= 0) {
        return;
    }

    paint->setAntiAlias(    make_fuzz_t<bool>(fuzz));
    paint->setDither(       make_fuzz_t<bool>(fuzz));
    paint->setColor(        make_fuzz_t<SkColor>(fuzz));
    paint->setBlendMode(    make_fuzz_enum_range<SkBlendMode>(fuzz, SkBlendMode::kLastMode));
    paint->setStyle(        make_fuzz_enum_range<SkPaint::Style>(fuzz,
                                                 SkPaint::Style::kStrokeAndFill_Style));
    paint->setShader(       make_fuzz_shader(fuzz, depth - 1));
    paint->setPathEffect(   make_fuzz_patheffect(fuzz, depth - 1));
    paint->setMaskFilter(   make_fuzz_maskfilter(fuzz));
    paint->setImageFilter(  make_fuzz_imageFilter(fuzz, depth - 1));
    paint->setColorFilter(  make_fuzz_colorfilter(fuzz, depth - 1));

    if (paint->getStyle() != SkPaint::kFill_Style) {
        paint->setStrokeWidth(make_fuzz_t<SkScalar>(fuzz));
        paint->setStrokeMiter(make_fuzz_t<SkScalar>(fuzz));
        paint->setStrokeCap(  make_fuzz_enum_range<SkPaint::Cap>(fuzz, SkPaint::kLast_Cap));
        paint->setStrokeJoin( make_fuzz_enum_range<SkPaint::Join>(fuzz, SkPaint::kLast_Join));
    }
}
#endif

//--------------------------------------------------------------------------------------------------
void check_draw(Context* context,
                Recorder* recorder,
                const SkPaint& paint,
                DrawTypeFlags dt,
                const SkPath& path) {
    int before = context->priv().globalCache()->numGraphicsPipelines();

    {
        // TODO: vary the colorType of the target surface too
        SkImageInfo ii = SkImageInfo::Make(16, 16,
                                           kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);

        sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(recorder, ii);
        SkCanvas* canvas = surf->getCanvas();

        switch (dt) {
            case DrawTypeFlags::kShape:
                canvas->drawRect(SkRect::MakeWH(16, 16), paint);
                canvas->drawPath(path, paint);
                break;
            default:
                SkASSERT(false);
                break;
        }

        std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
        context->insertRecording({ recording.get() });
        context->submit(SyncToCpu::kYes);
    }

    int after = context->priv().globalCache()->numGraphicsPipelines();

    // Actually using the SkPaint with the specified type of draw shouldn't have caused
    // any additional compilation
    SkASSERT(before == after);
}

void fuzz_graphite(Fuzz* fuzz, Context* context, int depth = 9) {
    auto recorder = context->makeRecorder();
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    SkColorInfo ci = SkColorInfo(kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                 SkColorSpace::MakeSRGB());

    std::unique_ptr<RuntimeEffectDictionary> rtDict = std::make_unique<RuntimeEffectDictionary>();
    KeyContext precompileKeyContext(recorder->priv().caps(),
                                    dict,
                                    rtDict.get(),
                                    ci,
                                    /* dstTexture= */ nullptr,
                                    /* dstOffset= */ {0, 0});

    sk_sp<TextureProxy> fakeDstTexture = TextureProxy::Make(recorder->priv().caps(),
                                                            SkISize::Make(1, 1),
                                                            kRGBA_8888_SkColorType,
                                                            skgpu::Mipmapped::kNo,
                                                            skgpu::Protected::kNo,
                                                            skgpu::Renderable::kYes,
                                                            skgpu::Budgeted::kNo);
    constexpr SkIPoint fakeDstOffset = SkIPoint::Make(0, 0);

    DrawTypeFlags kDrawType = DrawTypeFlags::kShape;
    SkPath path = make_path();

    SkRandom rand;

    PaintParamsKeyBuilder builder(dict);
    PipelineDataGatherer gatherer(Layout::kMetal);

    auto [paint, paintOptions] = create_random_paint(&rand, depth);


    bool hasCoverage = rand.nextBool();

    DstReadRequirement dstReadReq = DstReadRequirement::kNone;
    const SkBlenderBase* blender = as_BB(paint.getBlender());
    if (blender) {
        dstReadReq = GetDstReadRequirement(recorder->priv().caps(),
                                           blender->asBlendMode(),
                                           hasCoverage);
    }
    bool needsDstSample = dstReadReq == DstReadRequirement::kTextureCopy ||
                          dstReadReq == DstReadRequirement::kTextureSample;
    sk_sp<TextureProxy> curDst = needsDstSample ? fakeDstTexture : nullptr;

    auto [paintID, uData, tData] = ExtractPaintData(
            recorder.get(), &gatherer, &builder, Layout::kMetal, {},
            PaintParams(paint,
                        /* primitiveBlender= */ nullptr,
                        dstReadReq,
                        /* skipColorXform= */ false),
            curDst, fakeDstOffset, ci);

    std::vector<UniquePaintParamsID> precompileIDs;
    paintOptions.priv().buildCombinations(precompileKeyContext,
                                          /* addPrimitiveBlender= */ false,
                                          hasCoverage,
                                          [&](UniquePaintParamsID id) {
                                              precompileIDs.push_back(id);
                                          });

    // The specific key generated by ExtractPaintData should be one of the
    // combinations generated by the combination system.
    auto result = std::find(precompileIDs.begin(), precompileIDs.end(),
                            paintID);

#ifdef SK_DEBUG
    if (result == precompileIDs.end()) {
        SkDebugf("From paint: ");
        dump(dict, paintID);

        SkDebugf("From combination builder:");
        for (auto iter : precompileIDs) {
            dump(dict, iter);
        }
    }
#endif

    SkASSERT(result != precompileIDs.end());

    {
        context->priv().globalCache()->resetGraphicsPipelines();

        int before = context->priv().globalCache()->numGraphicsPipelines();
        Precompile(context, paintOptions, kDrawType);
        int after = context->priv().globalCache()->numGraphicsPipelines();

        SkASSERT(before == 0);
        SkASSERT(after > before);

        check_draw(context, recorder.get(), paint, kDrawType, path);
    }
}

} // anonymous namespace

DEF_FUZZ(Precompile, fuzz) {
    skiatest::graphite::ContextFactory factory;

    sk_gpu_test::GrContextFactory::ContextType contextType;
#if defined(SK_METAL)
    contextType = sk_gpu_test::GrContextFactory::kMetal_ContextType;
#elif defined(SK_VULKAN)
    contextType = sk_gpu_test::GrContextFactory::kVulkan_ContextType;
#elif defined(SK_DAWN)
    contextType = sk_gpu_test::GrContextFactory::kDawn_ContextType;
#else
    contextType = sk_gpu_test::GrContextFactory::kMock_ContextType;
#endif

    auto [_, context] = factory.getContextInfo(contextType);
    if (!context) {
        return;
    }

    fuzz_graphite(fuzz, context);
}
