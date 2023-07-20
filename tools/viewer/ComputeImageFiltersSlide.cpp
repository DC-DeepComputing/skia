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

// constexpr uint32_t kSrcDim = 512;

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

    void updateConvolMatrix(){
        fConvolutionMatrix.clear();

        for(int x=0-int(fKernelDim); x<=0; x++){
            for(int y=0-int(fKernelDim); y<=0; y++){
                float tmpFactor = (M_1_PI * 1/(2*fRou*fRou)) * exp(-1 * (x*x + y*y) / (2*fRou*fRou));
                fConvolutionMatrix.push_back(tmpFactor);
            }
        }
    }

    void updateSeparableConvolMatrix(){
        fSeparableConvolutionMatrix.clear();

        for(int i=0-int(fKernelDim); i<=0; i++){
            float tmpFactor = (1/ sqrtf(2 * fRou*fRou * M_PI)) * exp(-1 * (i*i) / (2*fRou*fRou));
            fSeparableConvolutionMatrix.push_back(tmpFactor);
        }
    }

protected:
    const uint32_t fWidth;
    const uint32_t fHeight;

    uint32_t fKernelDim = 1;
    float fRou = 1;
    vector<float> fConvolutionMatrix = {0.0625, 0.125, 0.125, 0.25};
    vector<float> fSeparableConvolutionMatrix = {0.241971, 0.398942};
};

class GrayscaleFilter : public ComputeImageFilter {
public:
    GrayscaleFilter(uint32_t srcWidth, uint32_t srcHeight) : ComputeImageFilter(
        /*width=*/srcWidth,
        /*height=*/srcHeight,
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
        return WorkgroupSize(ceil(fWidth / 32.0), ceil(fHeight / 32.0), 1);
    }
};

class BoxBlurFilter : public ComputeImageFilter {
public:
    BoxBlurFilter(uint32_t srcWidth, uint32_t srcHeight) : ComputeImageFilter(
        /*width=*/srcWidth,
        /*height=*/srcHeight,
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
                uint srcDim;
                int blurRadius;
            };

            void main() {
                half4 srcColor;
                uint2 neighbourCoord;
                float3 colorSum = float3(0,0,0);

                for(int i=0-blurRadius; i<=blurRadius; ++i){
                    for(int j=0-blurRadius; j<=blurRadius; ++j){
                        neighbourCoord = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                        srcColor = textureRead(src, neighbourCoord);
                        colorSum += srcColor.rbg;
                    }
                }

                int boxWidth = (blurRadius * 2) + 1;
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
        SkASSERT(resourceIndex == 2);
        SkDEBUGCODE(
            const skgpu::graphite::Uniform uniforms[] = {{"srcDim", SkSLType::kInt}, {"blurRadius", SkSLType::kInt}};
            mgr->setExpectedUniforms(uniforms);
        )
        mgr->write(int(fWidth - 1));
        mgr->write(int(fKernelDim));
    }

    WorkgroupSize calculateGlobalDispatchSize(const DP&) const override {
        return WorkgroupSize(ceil(fWidth / 32.0), ceil(fHeight / 32.0), 1);
    }
};

class SlowALUGaussianBlurFilter : public ComputeImageFilter {
public:
    SlowALUGaussianBlurFilter(uint32_t srcWidth, uint32_t srcHeight) : ComputeImageFilter(
        /*width=*/srcWidth,
        /*height=*/srcHeight,
        /*name=*/"SlowALUGaussianBlurFilter_Compute",
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
    ~SlowALUGaussianBlurFilter() override = default;

    std::string computeSkSL(const RBR&, int) const override {
        return R"(
            layout(set = 1, binding = 0) readonly texture2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                uint srcDim;
                int blurRadius;
                float rouInverse;
            };

            void main() {
                half4 srcColor;
                uint2 neighbourCoord;
                float3 colorSum = float3(0,0,0);

                float convolutionFactor;
                float piInverse = 0.31830988618;

                for(int i=0-blurRadius; i<=blurRadius; ++i){
                    for(int j=0-blurRadius; j<=blurRadius; ++j){
                        neighbourCoord = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                        srcColor = textureRead(src, neighbourCoord);

                        convolutionFactor = (piInverse * rouInverse) * exp(-1 * (float(i * i) + float(j * j)) * rouInverse);
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
        SkASSERT(resourceIndex == 2);
        SkDEBUGCODE(
            const skgpu::graphite::Uniform uniforms[] = {{"srcDim", SkSLType::kInt}, {"blurRadius", SkSLType::kInt}, {"rouInverse", SkSLType::kFloat}};
            mgr->setExpectedUniforms(uniforms);
        )
        mgr->write(int(fWidth - 1));
        mgr->write(int(fKernelDim));
        mgr->write(1/(2 * fRou * fRou));
    }

    WorkgroupSize calculateGlobalDispatchSize(const DP&) const override {
        return WorkgroupSize(ceil(fWidth / 32.0), ceil(fHeight / 32.0), 1);
    }
};

class SlowMemGaussianBlurFilter : public ComputeImageFilter {
public:
    SlowMemGaussianBlurFilter(uint32_t srcWidth, uint32_t srcHeight) : ComputeImageFilter(
        /*width=*/srcWidth,
        /*height=*/srcHeight,
        /*name=*/"SlowMemGaussianBlurFilter_Compute",
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
                /*slot=*/3,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kPrivate,
                /*policy=*/ResourcePolicy::kMapped,
                /*slot=*/2,
            }
        }) {}
    ~SlowMemGaussianBlurFilter() override = default;

    std::string computeSkSL(const RBR&, int) const override {
        return R"(
            layout(set = 1, binding = 0) readonly texture2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                uint srcDim;
                int blurRadius;
            };
            layout(set = 0, binding = 1) readonly buffer inputBlock
            {
                float in_data[];
            };

            void main() {
                half4 srcColor;
                uint2 neighbourCoord;
                float3 colorSum = float3(0,0,0);

                float convolutionFactor;

                for(int i=0-blurRadius; i<=blurRadius; ++i){
                    for(int j=0-blurRadius; j<=blurRadius; ++j){
                        neighbourCoord = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                        srcColor = textureRead(src, neighbourCoord);

                        convolutionFactor = in_data[uint(blurRadius - abs(i)) * uint(blurRadius+1) + uint(blurRadius - abs(j))];
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
        SkASSERT(resourceIndex == 2);
        SkDEBUGCODE(
            const skgpu::graphite::Uniform uniforms[] = {{"srcDim", SkSLType::kInt}, {"blurRadius", SkSLType::kInt}};
            mgr->setExpectedUniforms(uniforms);
        )
        mgr->write(int(fWidth - 1));
        mgr->write(int(fKernelDim));
    }

    size_t calculateBufferSize(const  skgpu::graphite::DrawParams&,
                                int resourceIndex,
                                const ResourceDesc& r) const override {
        SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);
        return sizeof(float) * (pow(fKernelDim+ 1, 2));
    }

    void prepareStorageBuffer(const  skgpu::graphite::DrawParams&,
                                int ssboIndex,
                                int resourceIndex,
                                const ResourceDesc& r,
                                void* buffer,
                                size_t bufferSize) const override {
        SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);

        memcpy(buffer, fConvolutionMatrix.data(), bufferSize);

        int boxWidth = fKernelDim + 1;
        size_t dataCount = sizeof(float) * (boxWidth * boxWidth);
        SkASSERT(bufferSize == dataCount);
        SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
    }

    WorkgroupSize calculateGlobalDispatchSize(const DP&) const override {
        return WorkgroupSize(ceil(fWidth / 32.0), ceil(fHeight / 32.0), 1);
    }
};

class SeparableGaussianBlurFilter : public ComputeImageFilter {
public:
    SeparableGaussianBlurFilter(uint32_t srcWidth, uint32_t srcHeight) : ComputeImageFilter(
        /*width=*/srcWidth,
        /*height=*/srcHeight,
        /*name=*/"SeparableGaussianBlurFilter_Compute",
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
                /*slot=*/3,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kPrivate,
                /*policy=*/ResourcePolicy::kMapped,
                /*slot=*/2,
            }
        }) {}
    ~SeparableGaussianBlurFilter() override = default;

    std::string computeSkSL(const RBR&, int) const override {
        return R"(
            layout(set = 1, binding = 0) readonly texture2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                uint srcDim;
                int blurRadius;
            };
            layout(set = 0, binding = 1) readonly buffer inputBlock
            {
                float in_data[];
            };

            workgroup float3 intermediate[1024];

            void main() {
                half4 srcColor;
                uint2 neighbourCoord;
                float3 colorSum = float3(0,0,0);

                float convolutionFactor;

                uint wgId = sk_WorkgroupID.y;
                uint offset = uint(-blurRadius + int(wgId) * (32 - 2 * blurRadius));
                uint2 pixelLoc = uint2(sk_GlobalInvocationID.x, sk_LocalInvocationID.y + offset);

                for(int i=0-blurRadius; i<=blurRadius; ++i){
                    neighbourCoord = clamp(pixelLoc + uint2(i, 0), uint2(0), uint2(srcDim));
                    srcColor = textureRead(src, neighbourCoord);

                    convolutionFactor = in_data[uint(blurRadius - abs(i))];
                    colorSum = colorSum + srcColor.rgb * convolutionFactor;
                }

                intermediate[sk_LocalInvocationID.x * 32 + sk_LocalInvocationID.y] = colorSum;
                workgroupBarrier();

                if(uint(blurRadius) <= sk_LocalInvocationID.y && sk_LocalInvocationID.y <= uint(31 - blurRadius)){
                    colorSum = float3(0,0,0);
                    float3 intermediateColor;

                    for(int i=0-blurRadius; i<=blurRadius; ++i){
                        neighbourCoord = clamp(sk_LocalInvocationID.xy + uint2(0, i), uint2(0), uint2(31));
                        intermediateColor = intermediate[neighbourCoord.x * 32 + neighbourCoord.y];

                        convolutionFactor = in_data[uint(blurRadius - abs(i))];
                        colorSum = colorSum + intermediateColor * convolutionFactor;
                    }

                    textureWrite(dest, pixelLoc, half4(colorSum.rgb1));
                }
            }
        )";
    }
    void prepareUniformBuffer(const DP&,
                            int resourceIndex,
                            const ResourceDesc&,
                            skgpu::graphite::UniformManager* mgr) const override {
        SkASSERT(resourceIndex == 2);
        SkDEBUGCODE(
            const skgpu::graphite::Uniform uniforms[] = {{"srcDim", SkSLType::kInt}, {"blurRaidus", SkSLType::kInt}};
            mgr->setExpectedUniforms(uniforms);
        )
        mgr->write(int(fWidth - 1));
        mgr->write(int(fKernelDim));
    }

    size_t calculateBufferSize(const  skgpu::graphite::DrawParams&,
                                int resourceIndex,
                                const ResourceDesc& r) const override {
        SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);
        return sizeof(float) * (fKernelDim+ 1);
    }

    void prepareStorageBuffer(const  skgpu::graphite::DrawParams&,
                                int ssboIndex,
                                int resourceIndex,
                                const ResourceDesc& r,
                                void* buffer,
                                size_t bufferSize) const override {
        SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);

        memcpy(buffer, fSeparableConvolutionMatrix.data(), bufferSize);

        int boxWidth = fKernelDim + 1;
        size_t dataCount = sizeof(float) * (boxWidth);
        SkASSERT(bufferSize == dataCount);
        SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
    }

    WorkgroupSize calculateGlobalDispatchSize(const DP&) const override {
        float hSize = ceilf(float(fHeight) / float(32 - 2*fKernelDim));
        return WorkgroupSize(ceil(fWidth / 32.0), hSize, 1);
    }
};

class ComputeImageFiltersSlide final : public ClickHandlerSlide {
public:
    ComputeImageFiltersSlide(sk_sp<SkImage> image, std::string imageName) :
        fImage(image),
        fImageName(imageName),
        fGrayscale(fImage->width(),fImage->height()),
        fBoxBlur(fImage->width(),fImage->height()),
        fALUGaussian(fImage->width(),fImage->height()),
        fMemGaussian(fImage->width(),fImage->height()),
        fSeparableGaussian(fImage->width(),fImage->height()) { fName = ("ImageFilters_Compute_" + fImageName).c_str(); }

private:
    class Click : public ClickHandlerSlide::Click {
    public:
        Click(int ptIdx) : fPtIdx(ptIdx) {}

        void doClick(SkPath* path) { [[maybe_unused]] SkPoint pt = path->getPoint(fPtIdx); }

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

        if (!fImage) {
            canvas->clear(SK_ColorRED);
            SkFont font(nullptr, 20);
            SkPaint captionPaint;
            captionPaint.setColor(SK_ColorWHITE);
            canvas->drawString(("Could not load " + fImageName).c_str(), 10, 30, font, captionPaint);
            return;
        }

        canvas->clear(SK_ColorWHITE);

        SkPaint paint;
        const char* captions[] = {"Source", "Seperable gaussian", "Slow box blur", "Slow ALU gaussian blur", "Slow mem gaussian blur", "Grayscale"};
        uint32_t dim = fImage->width(); //assuming image is square
        uint32_t margin = 10;
        for (unsigned int i = 0; i < std::size(captions); i++) {
            if (i == 0) {
                canvas->drawImage(
                        fImage, margin + i * (dim + margin), margin, SkSamplingOptions(), &paint);
            }
            else if(i == 5 && fIsDisplayGrayscale) {
                device->drawComputeFilteredImage(
                        fImage, margin + i * (dim + margin), margin, &fGrayscale);
            }
            else if(i == 2 && fIsDisplaySlowBox){
                device->drawComputeFilteredImage(
                        fImage, margin + i * (dim + margin), margin, &fBoxBlur);
            }
            else if(i == 3 && fIsDisplaySlowALUGaussian){
                device->drawComputeFilteredImage(
                        fImage, margin + i * (dim + margin), margin, &fALUGaussian);
            }
            else if(i == 4 && fIsDisplaySlowMemGaussian){
                device->drawComputeFilteredImage(
                        fImage, margin + i * (dim + margin), margin, &fMemGaussian);
            }
            else if(i == 1 && fIsDisplaySeparableGaussian){
                device->drawComputeFilteredImage(
                        fImage, margin + i * (dim + margin), margin, &fSeparableGaussian);
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
        ImGui::Checkbox("Slow ALU Gaussian Blur", &fIsDisplaySlowALUGaussian);
        ImGui::Checkbox("Slow Mem Gaussian Blur", &fIsDisplaySlowMemGaussian);
        ImGui::Checkbox("Separable Gaussian Blur", &fIsDisplaySeparableGaussian);

        float blurBox = fKernelDim;
        if (ImGui::SliderFloat("Radius", &blurBox, 1, fIsDisplaySeparableGaussian? 15:64)) {
            fKernelDim = int(blurBox);

            fBoxBlur.setKernelDim(fKernelDim);
            fALUGaussian.setKernelDim(fKernelDim);
            fMemGaussian.setKernelDim(fKernelDim);
            fSeparableGaussian.setKernelDim(fKernelDim);
            if(fIsDisplaySlowMemGaussian) fMemGaussian.updateConvolMatrix();
            if(fIsDisplaySeparableGaussian) fSeparableGaussian.updateSeparableConvolMatrix();
        }
        float rou = fRou;
        if (ImGui::SliderFloat("Rou", &rou, 1, max(fKernelDim/3*2, 2))) {
            fRou = rou;
            fALUGaussian.setRou(fRou);
            fMemGaussian.setRou(fRou);
            fSeparableGaussian.setRou(fRou);
            if(fIsDisplaySlowMemGaussian) fMemGaussian.updateConvolMatrix();
            if(fIsDisplaySeparableGaussian) fSeparableGaussian.updateSeparableConvolMatrix();
        }
        ImGui::End();

    }

    bool onChar(SkUnichar c) override { return false; }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override {
        return nullptr;
    }

    bool onClick(ClickHandlerSlide::Click* clickBase) override { return false; }

private:
    sk_sp<SkImage> fImage;
    std::string fImageName;

    GrayscaleFilter fGrayscale;
    BoxBlurFilter fBoxBlur;
    SlowALUGaussianBlurFilter fALUGaussian;
    SlowMemGaussianBlurFilter fMemGaussian;
    SeparableGaussianBlurFilter fSeparableGaussian;

    bool fIsDisplayGrayscale;
    bool fIsDisplaySlowBox;
    bool fIsDisplaySlowALUGaussian;
    bool fIsDisplaySlowMemGaussian;
    bool fIsDisplaySeparableGaussian;

    int fKernelDim = 1;
    float fRou = 1;
};

}  // namespace

DEF_SLIDE(
    std::string imageName = "mandrill_256.png";
    std::string imagePath = "images/" + imageName;
    return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName); )

DEF_SLIDE(
    std::string imageName = "mandrill_512.png";
    std::string imagePath = "images/" + imageName;
    return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName); )

DEF_SLIDE(
    std::string imageName = "mandrill_1600.png";
    std::string imagePath = "images/" + imageName;
    return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName); )

DEF_SLIDE(
    std::string imageName = "puppy_999.png";
    std::string imagePath = "images/" + imageName;
    return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName); )

#endif  // defined(SK_GRAPHITE)
