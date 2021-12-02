
/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkCustomMesh.h"
#include "include/effects/SkGradientShader.h"

#include <memory>

namespace skiagm {
class CustomMeshGM : public skiagm::GpuGM {
public:
    CustomMeshGM() {}

protected:
    using Attr    = SkCustomMeshSpecification::Attribute;
    using Varying = SkCustomMeshSpecification::Varying;

    SkISize onISize() override { return {800, 800}; }

    void onOnceBeforeDraw() override {
        {
            static const Attr kAttributes[]{
                    {Attr::Type::kUByte4_unorm, SkString{"brag"}, 0},
                    {Attr::Type::kFloat4,       SkString{"xuyv"}, 4},
            };
            static const Varying kVaryings[]{
                    {Varying::Type::kHalf4,  SkString{"color"}},
                    {Varying::Type::kFloat2, SkString{"uv"}   },
            };
            static constexpr char kVS[] = R"(
                    half4 unswizzle_color(half4 color) { return color.garb; }

                    float2 main(in Attributes attributes, out Varyings varyings) {
                        varyings.color = unswizzle_color(attributes.brag);
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
                    sizeof(ColorVertex),
                    SkMakeSpan(kVaryings, SK_ARRAY_COUNT(kVaryings)),
                    SkString(kVS),
                    SkString(kFS));
            if (!spec) {
                SkDebugf("%s\n", error.c_str());
            }
            fSpecWithColor = std::move(spec);
        }
        {
            static const Attr kAttributes[]{
                    {Attr::Type::kFloat4, SkString{"xuyv"}, 0},
            };
            static const Varying kVaryings[]{
                    {Varying::Type::kFloat2, SkString{"vux2"}},
            };
            static constexpr char kVS[] = R"(
                    float2 main(in Attributes a, out Varyings v) {
                        v.vux2 = 2*a.xuyv.wy;
                        return a.xuyv.xz;
                    }
            )";
            static constexpr char kFS[] = R"(
                    float2 helper(in float2 vux2) { return vux2.yx/2; }
                    float2 main(in Varyings varyings) {
                        return helper(varyings.vux2);
                    }
            )";
            auto [spec, error] = SkCustomMeshSpecification::Make(
                    SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                    sizeof(NoColorVertex),
                    SkMakeSpan(kVaryings, SK_ARRAY_COUNT(kVaryings)),
                    SkString(kVS),
                    SkString(kFS));
            if (!spec) {
                SkDebugf("%s\n", error.c_str());
            }
            fSpecWithNoColor = std::move(spec);
        }

        static constexpr SkColor kColors[] = {SK_ColorTRANSPARENT, SK_ColorWHITE};
        fShader = SkGradientShader::MakeRadial({10, 10},
                                               3,
                                               kColors,
                                               nullptr,
                                               2,
                                               SkTileMode::kMirror);
    }

    std::unique_ptr<char[]> makeColorQuad() {
        static constexpr ColorVertex kQuad[]{
                {0x00FFFF00, { 20.f, 00.f,  20.f, 00.f}},
                {0x00FFFFFF, {120.f, 20.f,  20.f, 00.f}},
                {0xFFFF00FF, { 20.f, 00.f, 120.f, 20.f}},
                {0xFFFFFF00, {120.f, 20.f, 120.f, 20.f}},
        };
        auto result = std::make_unique<char[]>(sizeof(kQuad));
        std::memcpy(result.get(), kQuad, sizeof(kQuad));
        return result;
    }

    std::unique_ptr<char[]> makeNoColorQuad() {
        static constexpr NoColorVertex kQuad[]{
                {{ 20.f, 00.f,  20.f, 00.f}},
                {{120.f, 20.f,  20.f, 00.f}},
                {{ 20.f, 00.f, 120.f, 20.f}},
                {{120.f, 20.f, 120.f, 20.f}},
        };
        auto result = std::make_unique<char[]>(sizeof(kQuad));
        std::memcpy(result.get(), kQuad, sizeof(kQuad));
        return result;
    }

    SkString onShortName() override { return SkString("custommesh"); }

    DrawResult onDraw(GrRecordingContext* rc, SkCanvas* canvas, SkString*) override {
        for (const sk_sp<SkBlender>& blender : {SkBlender::Mode(SkBlendMode::kDst),
                                                SkBlender::Mode(SkBlendMode::kSrc),
                                                SkBlender::Mode(SkBlendMode::kSaturation)}) {
            canvas->save();
            for (uint8_t alpha  : {0xFF , 0x40})
            for (bool    colors : {false, true})
            for (bool    shader : {false, true}) {
                auto vb = colors ? this->makeColorQuad() : this->makeNoColorQuad();

                SkCustomMesh rm;
                rm.spec   = colors ? fSpecWithColor : fSpecWithNoColor;
                rm.vb     = std::move(vb);
                rm.vcount = 4;
                rm.mode   = SkCustomMesh::Mode::kTriangleStrip;

                SkPaint paint;
                paint.setColor(SK_ColorGREEN);
                paint.setShader(shader ? fShader : nullptr);
                paint.setAlpha(alpha);

                canvas->drawCustomMesh(std::move(rm), blender, paint);

                canvas->translate(0, 150);
            }
            canvas->restore();
            canvas->translate(150, 0);
        }
        return DrawResult::kOk;
    }

private:
    struct ColorVertex {
        uint32_t brag;
        float    xuyv[4];
    };

    struct NoColorVertex {
        float    xuyv[4];
    };

    sk_sp<SkShader> fShader;

    sk_sp<SkCustomMeshSpecification> fSpecWithColor;
    sk_sp<SkCustomMeshSpecification> fSpecWithNoColor;
};

DEF_GM( return new CustomMeshGM; )

}  // namespace skiagm
