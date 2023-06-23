/*
 * Copyright 2023 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "tools/Resources.h"
#include "tools/viewer/ClickHandlerSlide.h"
#include "imgui.h"

#include <iostream>

#if defined(SK_GRAPHITE)

using namespace std;

namespace {

using DP = skgpu::graphite::DrawParams;
using RBR = skgpu::graphite::ResourceBindingRequirements;
using skgpu::graphite::WorkgroupSize;

constexpr uint32_t kSrcDim = 256;

class ComputeImageFilter : public skgpu::graphite::ComputeStep {
public:
    ComputeImageFilter(uint32_t width,
                       uint32_t height,
                       std::string_view name,
                       WorkgroupSize localDispatchSize,
                       SkSpan<const ResourceDesc> resources)
            : ComputeStep(name, localDispatchSize, resources), fWidth(width), fHeight(height){}

    void setKernelDim(uint32_t d) {
        fKernelDim = d;
    }

    void setRou(float r) {
        fRou = r;
    }

protected:
    const uint32_t fWidth;
    const uint32_t fHeight;
    uint32_t fKernelDim = 3;
    float fRou = 1;
};

class GrayscaleFilter : public ComputeImageFilter {
public:
    GrayscaleFilter() : ComputeImageFilter(
        /*width=*/kSrcDim,
        /*height=*/kSrcDim,
        /*name=*/"GrayscalFilter_Compute",
        /*localDispatchSize=*/{32, 32, 1},
        /*resources=*/{
            {
                /*type=*/ResourceType::kTexture,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/0,
            },
            {
                /*type=*/ResourceType::kStorageTexture,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/1,
            }
        }) {}
    ~GrayscaleFilter() override = default;

    std::string computeSkSL(const RBR&, int) const override {
        return R"(
            layout(binding = 0) readonly texture2D src;
            layout(binding = 1) writeonly texture2D dest;

            void main() {
                half4 srcColor = textureRead(src, sk_GlobalInvocationID.xy);
                half albedo = dot(srcColor.rgb, half3(0.3, 0.59, 0.11));
                textureWrite(dest, sk_GlobalInvocationID.xy, half3(albedo).rgb1);
            }
        )";
    }

    WorkgroupSize calculateGlobalDispatchSize(const DP&) const override {
        return WorkgroupSize(kSrcDim / 32, kSrcDim / 32, 1);
    }
};

class BoxBlurFilter : public ComputeImageFilter {
public:
    BoxBlurFilter() : ComputeImageFilter(
        /*width=*/kSrcDim,
        /*height=*/kSrcDim,
        /*name=*/"BoxBlurFilter_Compute",
        /*localDispatchSize=*/{32, 32, 1},
        /*resources=*/{
            {
                /*type=*/ResourceType::kTexture,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/0,
            },
            {
                /*type=*/ResourceType::kStorageTexture,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/1,
            },
            // Uniform buffer:
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kPrivate,
                /*policy=*/ResourcePolicy::kMapped,
                /*slot=*/2,
            }
        }) {}
    ~BoxBlurFilter() override = default;

    std::string computeSkSL(const RBR&, int) const override {
        return R"(
            layout(set = 1, binding = 0) readonly texture2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                uint boxDim;
            };

            void main() {
                half4 srcColor = textureRead(src, sk_GlobalInvocationID.xy);

                uint srcDim = 255;
                int boxWidth = int(boxDim); //hardcoded dimension of box blur's box
                int blurRadius = int((boxWidth - 1) / 2);

                float3 colorSum = float3(0,0,0);
                uint2 neighbourXY = uint2(0,0);

                for(int i=0-blurRadius; i<=blurRadius; ++i){
                    for(int j=0-blurRadius; j<=blurRadius; ++j){
                        neighbourXY = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                        srcColor = textureRead(src, neighbourXY);
                        colorSum = colorSum + srcColor.rbg;
                    }
                }

                float boxSize = float(boxWidth * boxWidth);
                colorSum = colorSum / boxSize;

                textureWrite(dest, sk_GlobalInvocationID.xy, half4(colorSum.rbg1));
            }
        )";
    }
    void prepareUniformBuffer(const DP&,
                            int resourceIndex,
                            const ResourceDesc&,
                            skgpu::graphite::UniformManager* mgr) const override {
        SkASSERT(resourceIndex == 0);
        SkDEBUGCODE(
            const Uniform uniforms[] = {{"boxDim", SkSLType::kUInt}};
            mgr->setExpectedUniforms(uniforms);
        )
        mgr->write(int(fKernelDim));
    }

    WorkgroupSize calculateGlobalDispatchSize(const DP&) const override {
        return WorkgroupSize(kSrcDim / 32, kSrcDim / 32, 1);
    }
};

class GaussianBlurFilter : public ComputeImageFilter {
public:
    GaussianBlurFilter() : ComputeImageFilter(
        /*width=*/kSrcDim,
        /*height=*/kSrcDim,
        /*name=*/"GaussianBlurFilter_Compute",
        /*localDispatchSize=*/{32, 32, 1},
        /*resources=*/{
            {
                /*type=*/ResourceType::kTexture,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/0,
            },
            {
                /*type=*/ResourceType::kStorageTexture,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/1,
            },
            // Uniform buffer:
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kPrivate,
                /*policy=*/ResourcePolicy::kMapped,
                /*slot=*/2,
            }
        }) {}
    ~GaussianBlurFilter() override = default;

    std::string computeSkSL(const RBR&, int) const override {
        return R"(
            layout(set = 1, binding = 0) readonly texture2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                uint boxDim;
                float stddev;
            };

            void main() {
                half4 srcColor = textureRead(src, sk_GlobalInvocationID.xy);

                uint srcDim = 255;
                int boxWidth = int(boxDim);
                int blurRadius = int((boxWidth - 1) / 2);

                float3 colorSum = float3(0,0,0);
                uint2 neighbourXY = uint2(0,0);

                float convolutionFactor;
                float pi = 3.1415926535897932384626433832795;
                float rou = stddev;

                for(int i=0-blurRadius; i<=blurRadius; ++i){
                    for(int j=0-blurRadius; j<=blurRadius; ++j){
                        neighbourXY = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                        srcColor = textureRead(src, neighbourXY);

                        convolutionFactor = (1/(2*pi*rou*rou)) * exp(-1 * (float(i * i) + float(j * j)) / (2*rou*rou));
                        colorSum = colorSum + srcColor.rgb * convolutionFactor;
                    }
                }

                textureWrite(dest, sk_GlobalInvocationID.xy, half4(colorSum.rgb1));
            }
        )";
    }
    void prepareUniformBuffer(const DP&,
                            int resourceIndex,
                            const ResourceDesc&,
                            skgpu::graphite::UniformManager* mgr) const override {
        SkASSERT(resourceIndex == 0);
        SkDEBUGCODE(
            const Uniform uniforms[] = {{"boxDim", SkSLType::kUInt}};
            mgr->setExpectedUniforms(uniforms);
        )
        mgr->write(int(fKernelDim));
        mgr->write(fRou);
    }

    WorkgroupSize calculateGlobalDispatchSize(const DP&) const override {
        return WorkgroupSize(kSrcDim / 32, kSrcDim / 32, 1);
    }
};

class ComputeImageFiltersSlide final : public ClickHandlerSlide {
public:
    ComputeImageFiltersSlide() { fName = "ImageFilters_Compute"; }

private:
    class Click : public ClickHandlerSlide::Click {
    public:
        Click(int ptIdx) : fPtIdx(ptIdx) {}

        void doClick(SkPath* path) { SK_UNUSED SkPoint pt = path->getPoint(fPtIdx); }

    private:
        int fPtIdx;
    };

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        skgpu::graphite::Device* device = SkCanvasPriv::TopDevice(canvas)->asGraphiteDevice();
        if (!device) {
            canvas->clear(SK_ColorRED);
            SkFont font(nullptr, 20);
            SkPaint captionPaint;
            captionPaint.setColor(SK_ColorWHITE);
            canvas->drawString("Graphite only", 10, 30, font, captionPaint);
            return;
        }

        auto image = GetResourceAsImage("images/mandrill_256.png");
        // auto image = GetResourceAsImage("images/white.png");
        if (!image) {
            canvas->clear(SK_ColorRED);
            SkFont font(nullptr, 20);
            SkPaint captionPaint;
            captionPaint.setColor(SK_ColorWHITE);
            canvas->drawString("Could not load mandrill_256.png", 10, 30, font, captionPaint);
            return;
        }

        canvas->clear(SK_ColorWHITE);

        SkPaint paint;
        const char* captions[] = {"Source", "Grayscale", "Slow box blur", "Slow gaussian blur"};
        uint32_t dim = 256;
        uint32_t margin = 10;
        for (unsigned int i = 0; i < std::size(captions); i++) {
            if (i == 0) {
                canvas->drawImage(
                        image, margin + i * (dim + margin), margin, SkSamplingOptions(), &paint);
            } else if(i == 1 && fIsDisplayGrayscale) {
                device->drawComputeFilteredImage(
                        image, margin + i * (dim + margin), margin, &fGrayscale);
            }
            else if(i == 2 && fIsDisplaySlowBox){
                device->drawComputeFilteredImage(
                        image, margin + i * (dim + margin), margin, &fBlur);
            }
            else if(i == 3 && fIsDisplaySlowGaussian){
                device->drawComputeFilteredImage(
                        image, margin + i * (dim + margin), margin, &fGaussian);
            }

            SkFont font(nullptr, 16);
            SkPaint captionPaint;
            captionPaint.setColor(SK_ColorBLACK);
            canvas->drawString(captions[i],
                               margin + i * (dim + margin),
                               5 + 2 * margin + dim,
                               font,
                               captionPaint);
        }

        ImGui::Begin("Blur slider", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Checkbox("Greyscale", &fIsDisplayGrayscale);
        ImGui::Checkbox("Slow Box Blur", &fIsDisplaySlowBox);
        ImGui::Checkbox("Slow Gaussian Blur", &fIsDisplaySlowGaussian);

        float blurBox = fblurBox;
        if (ImGui::SliderFloat("Dim", &blurBox, 3, 255)) {
            fblurBox = int(blurBox)%2==0 ? blurBox+1 : blurBox; //making sure it is odd
            fBlur.setKernelDim(fblurBox);
            fGaussian.setKernelDim(fblurBox);
        }
        float rou = fRou;
        if (ImGui::SliderFloat("Rou", &rou, 1, 50)) {
            fRou = rou; //making sure it is odd
            fGaussian.setRou(fRou);
        }
    }

    bool onChar(SkUnichar c) override { return false; }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override {
        return nullptr;
    }

    bool onClick(ClickHandlerSlide::Click* clickBase) override { return false; }

private:
    GrayscaleFilter fGrayscale;
    BoxBlurFilter fBlur;
    GaussianBlurFilter fGaussian;

    bool fIsDisplayGrayscale;
    bool fIsDisplaySlowBox;
    bool fIsDisplaySlowGaussian;

    int fblurBox = 3;
    float fRou = 1;
};

}  // namespace

DEF_SLIDE( return new ComputeImageFiltersSlide; )

#endif  // defined(SK_GRAPHITE)
