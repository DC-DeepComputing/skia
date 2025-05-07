/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkMathPriv.h"
#include "tests/graphite/precompile/PrecompileTestUtils.h"

using namespace skgpu::graphite;

namespace {

#if defined(SK_DEBUG)
// This helper maps from the RenderPass string in the Pipeline label to the
// RenderPassProperties needed by the Precompile system
// TODO(robertphillips): converting this to a more piecemeal approach might better illuminate
// the mapping between the string and the RenderPassProperties
RenderPassProperties get_render_pass_properties(const char* str) {
    static const struct {
        RenderPassProperties fRenderPassProperties;
        const char* fStr;
    } kRenderPassPropertiesMapping[] = {
        { kR_1_D,     "RP((R8+D16 x1).a000)" },
        { kBGRA_1_D,  "RP((BGRA8+D16 x1).rgba)" },
        // These RPPs can generate two strings when Caps::loadOpAffectsMSAAPipelines.
        { kR_4_DS,    "RP((R8+D24_S8 x4->1).a000)" },
        { kR_4_DS,    "RP((R8+D24_S8 x4->1).a000 w/ msaa load)" },
        { kBGRA_4_D,  "RP((BGRA8+D16 x4->1).rgba)" },
        { kBGRA_4_D,  "RP((BGRA8+D16 x4->1).rgba w/ msaa load)" },
        { kBGRA_4_DS, "RP((BGRA8+D24_S8 x4->1).rgba)" },
        { kBGRA_4_DS, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load)" },
    };

    for (const auto& rppm : kRenderPassPropertiesMapping) {
        if (strstr(str, rppm.fStr)) {
            return rppm.fRenderPassProperties;
        }
    }

    SkAssertResult(0);
    return {};
}

// This helper maps from the RenderStep's name in the Pipeline label to the DrawTypeFlag that
// resulted in its use.
DrawTypeFlags get_draw_type_flags(const char* str) {
    static const struct {
        const char* fStr;
        DrawTypeFlags fFlags;
    } kDrawTypeFlagsMapping[] = {
        { "BitmapTextRenderStep[Mask]",                  DrawTypeFlags::kBitmapText_Mask  },
        { "BitmapTextRenderStep[LCD]",                   DrawTypeFlags::kBitmapText_LCD   },
        { "BitmapTextRenderStep[Color]",                 DrawTypeFlags::kBitmapText_Color },

        { "SDFTextRenderStep",                           DrawTypeFlags::kSDFText      },
        { "SDFTextLCDRenderStep",                        DrawTypeFlags::kSDFText_LCD  },

        { "VerticesRenderStep[Tris]",                    DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisTexCoords]",           DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisColor]",               DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisColorTexCoords]",      DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[Tristrips]",               DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsTexCoords]",      DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsColor]",          DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsColorTexCoords]", DrawTypeFlags::kDrawVertices },

        { "CircularArcRenderStep",                       DrawTypeFlags::kCircularArc  },

        { "AnalyticRRectRenderStep",                     DrawTypeFlags::kAnalyticRRect  },
        { "CoverBoundsRenderStep[NonAAFill]",            DrawTypeFlags::kNonAAFillRect  },
        { "PerEdgeAAQuadRenderStep",                     DrawTypeFlags::kPerEdgeAAQuad  },

        { "CoverageMaskRenderStep",                      DrawTypeFlags::kNonSimpleShape },
        { "CoverBoundsRenderStep[RegularCover]",         DrawTypeFlags::kNonSimpleShape },
        { "CoverBoundsRenderStep[InverseCover]",         DrawTypeFlags::kNonSimpleShape },
        { "MiddleOutFanRenderStep[EvenOdd]",             DrawTypeFlags::kNonSimpleShape },
        { "MiddleOutFanRenderStep[Winding]",             DrawTypeFlags::kNonSimpleShape },
        { "TessellateCurvesRenderStep[EvenOdd]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateCurvesRenderStep[Winding]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateStrokesRenderStep",                 DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[Convex]",          DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[EvenOdd]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[Winding]",         DrawTypeFlags::kNonSimpleShape },
    };

    for (const auto& dtfm : kDrawTypeFlagsMapping) {
        if (strstr(str, dtfm.fStr)) {
            SkAssertResult(dtfm.fFlags != DrawTypeFlags::kNone);
            return dtfm.fFlags;
        }
    }

    SkAssertResult(0);
    return DrawTypeFlags::kNone;
}

#endif // SK_DEBUG

} // anonymous namespace

#if defined(SK_DEBUG)

bool PrecompileSettings::isSubsetOf(const PrecompileSettings& superSet) const {
    SkASSERT(SkPopCount(static_cast<uint32_t>(fDrawTypeFlags)) == 1);

    // 'superSet' may have a wider range of DrawTypeFlags
    return (fDrawTypeFlags & superSet.fDrawTypeFlags) &&
           fRenderPassProps == superSet.fRenderPassProps;
}

void DeduceSettingsFromLabel(const char* testStr, PrecompileSettings* result) {
    result->fDrawTypeFlags = get_draw_type_flags(testStr);
    result->fRenderPassProps = get_render_pass_properties(testStr);
    if (strstr(testStr, "LinearGradient4 ColorSpaceTransformSRGB") ||
        strstr(testStr, "LinearGradient8 ColorSpaceTransformSRGB") ||
        strstr(testStr, "PrimitiveColor ColorSpaceTransformSRGB")) {
        result->fRenderPassProps.fDstCS = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                               SkNamedGamut::kAdobeRGB);
    } else if (strstr(testStr, "ColorSpaceTransformSRGB")) {
        result->fRenderPassProps.fDstCS = SkColorSpace::MakeSRGB();
    }
}

#endif // SK_DEBUG
