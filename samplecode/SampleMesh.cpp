
/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkCustomMesh.h"
#include "include/effects/SkGradientShader.h"
#include "samplecode/Sample.h"
#include "tools/Resources.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////
// Sample to demonstrate runtime mesh shaders

class SampleMesh : public Sample {
public:
    SampleMesh() {}

protected:
    void onOnceBeforeDraw() override {}

    SkString name() override { return SkString("SampleMesh"); }

    void onDrawContent(SkCanvas* canvas) override {
        using Attr    = SkCustomMeshSpecification::Attribute;
        using Varying = SkCustomMeshSpecification::Varying;

        struct V {
            uint32_t brag;
            float xuyv[4];
        };
        static const Attr kAttributes[]{
                {Attr::Type::kUByte4_unorm, SkString{"brag"}, 0},
                {Attr::Type::kFloat4,       SkString{"xuyv"}, 4},
        };
        static const Varying kVaryings[]{
                {Varying::Type::kHalf4,  SkString{"color"}},
                {Varying::Type::kFloat2, SkString{"uv"}},
        };
        static constexpr V kQuad[]{
                {0x00FFFF00, {20.f,  00.f,  20.f, 00.f}},
                {0x00FFFFFF, {120.f, 20.f,  20.f, 00.f}},
                {0xFFFF00FF, {20.f,  00.f, 120.f, 20.f}},
                {0xFFFFFF00, {120.f, 20.f, 120.f, 20.f}},
        };
        static constexpr char kVS[] = R"(
                float2 main(in Attributes attributes, out Varyings varyings) {
                    varyings.color = attributes.brag.garb;
                    varyings.uv    = attributes.xuyv.yw;
                    return attributes.xuyv.xz;
                }
        )";
        static constexpr char kFS[] = R"(
                float2 main(in Varyings varyings, out half4 color) {
                    color = varyings.color;
                    return varyings.uv;
                }
        )";
        auto [spec, error] = SkCustomMeshSpecification::Make(
                SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                sizeof(V),
                SkMakeSpan(kVaryings, SK_ARRAY_COUNT(kVaryings)),
                SkString(kVS),
                SkString(kFS));
        if (!spec) {
            SkDebugf("%s\n", error.c_str());
            return;
        }
        for (const sk_sp<SkBlender>& blender : {sk_sp<SkBlender>(nullptr),
                                                SkBlender::Mode(SkBlendMode::kSaturation)}) {
            canvas->save();
            for (bool shader : {false, true}) {
                for (uint8_t alpha : {0xFF, 0x40}) {
                    auto vb = std::make_unique<char[]>(sizeof(kQuad));
                    memcpy(vb.get(), kQuad, sizeof(kQuad));

                    SkCustomMesh rm;
                    rm.spec   = spec;
                    rm.vb     = std::move(vb);
                    rm.vcount = SK_ARRAY_COUNT(kQuad);
                    rm.mode   = SkCustomMesh::Mode::kTriangleStrip;

                    SkPaint paint;
                    if (shader) {
                        static constexpr SkColor kColors[] = {SK_ColorTRANSPARENT, SK_ColorWHITE};
                        auto gradient = SkGradientShader::MakeRadial({10, 10},
                                                                     3,
                                                                     kColors,
                                                                     nullptr,
                                                                     2,
                                                                     SkTileMode::kMirror);
                        paint.setShader(std::move(gradient));
                    }
                    paint.setAlpha(alpha);

                    canvas->drawCustomMesh(std::move(rm), blender, paint);

                    canvas->translate(0, 150);
                }
            }
            canvas->restore();
            canvas->translate(150, 0);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new SampleMesh(); )
