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
                int blurRadius = int(boxDim);

                float3 colorSum = float3(0,0,0);
                uint2 neighbourXY = uint2(0,0);

                for(int i=0-blurRadius; i<=blurRadius; ++i){
                    for(int j=0-blurRadius; j<=blurRadius; ++j){
                        neighbourXY = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                        srcColor = textureRead(src, neighbourXY);
                        colorSum = colorSum + srcColor.rbg;
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
            const skgpu::graphite::Uniform uniforms[] = {{"boxDim", SkSLType::kInt}};
            mgr->setExpectedUniforms(uniforms);
        )
        mgr->write(int(fKernelDim));
    }

    WorkgroupSize calculateGlobalDispatchSize(const DP&) const override {
        return WorkgroupSize(kSrcDim / 32, kSrcDim / 32, 1);
    }
};

class SlowALUGaussianBlurFilter : public ComputeImageFilter {
public:
    SlowALUGaussianBlurFilter() : ComputeImageFilter(
        /*width=*/kSrcDim,
        /*height=*/kSrcDim,
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
                uint boxDim;
                float stddev;
                float stddevp;
            };

            void main() {
                half4 srcColor = textureRead(src, sk_GlobalInvocationID.xy);

                uint srcDim = 255;
                int blurRadius = int(boxDim);

                float3 colorSum = float3(0,0,0);
                uint2 neighbourXY = uint2(0,0);

                float convolutionFactor;
                float pi = 3.1415926535897932384626433832795;
                float rou = stddev;

                float pip = 0.31830988618;
                float roup = stddevp;

                for(int i=0-blurRadius; i<=blurRadius; ++i){
                    for(int j=0-blurRadius; j<=blurRadius; ++j){
                        neighbourXY = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                        srcColor = textureRead(src, neighbourXY);

                        // convolutionFactor = (1/(2*pi*rou*rou)) * exp(-1 * (float(i * i) + float(j * j)) / (2*rou*rou));
                        convolutionFactor = (pip * roup) * exp(-1 * (float(i * i) + float(j * j)) * roup);
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
            const skgpu::graphite::Uniform uniforms[] = {{"boxDim", SkSLType::kInt}, {"rou", SkSLType::kFloat}, {"roup", SkSLType::kFloat}};
            mgr->setExpectedUniforms(uniforms);
        )
        mgr->write(int(fKernelDim));
        mgr->write(fRou);
        mgr->write(1/(2 * fRou * fRou));
    }

    WorkgroupSize calculateGlobalDispatchSize(const DP&) const override {
        return WorkgroupSize(kSrcDim / 32, kSrcDim / 32, 1);
    }
};

class SlowMemGaussianBlurFilter : public ComputeImageFilter {
public:
    SlowMemGaussianBlurFilter() : ComputeImageFilter(
        /*width=*/kSrcDim,
        /*height=*/kSrcDim,
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
                uint boxDim;
                float stddev;
            };
            layout(set = 0, binding = 1) readonly buffer inputBlock
            {
                float in_data[];
            };

            void main() {
                half4 srcColor = textureRead(src, sk_GlobalInvocationID.xy);

                uint srcDim = 255;
                int blurRadius = int(boxDim);
                uint boxWidth = boxDim * 2 + 1;

                float3 colorSum = float3(0,0,0);
                uint2 neighbourXY = uint2(0,0);
                float convolutionFactor;

                // for(int i=0-blurRadius; i<=blurRadius; ++i){ 
                //     for(int j=0-blurRadius; j<=blurRadius; ++j){
                //         neighbourXY = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                //         srcColor = textureRead(src, neighbourXY);

                //         convolutionFactor = in_data[uint(i + blurRadius) * boxWidth + uint(j + blurRadius)];
                //         colorSum = colorSum + srcColor.rgb * convolutionFactor;
                //     }
                // }

                for(int i=0-blurRadius; i<=0; ++i){
                    for(int j=0-blurRadius; j<=0; ++j){
                        neighbourXY = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                        srcColor = textureRead(src, neighbourXY);

                        convolutionFactor = in_data[uint(blurRadius + i) * uint(blurRadius+1) + uint(blurRadius + j)];
                        colorSum = colorSum + srcColor.rgb * convolutionFactor;
                    }
                }

                for(int i=1; i<=blurRadius; ++i){
                    for(int j=0-blurRadius; j<=0; ++j){
                        neighbourXY = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                        srcColor = textureRead(src, neighbourXY);

                        convolutionFactor = in_data[uint(blurRadius - i) * uint(blurRadius+1) + uint(blurRadius + j)];
                        colorSum = colorSum + srcColor.rgb * convolutionFactor;
                    }
                }

                for(int i=0-blurRadius; i<=0; ++i){
                    for(int j=1; j<=blurRadius; ++j){
                        neighbourXY = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                        srcColor = textureRead(src, neighbourXY);

                        convolutionFactor = in_data[uint(blurRadius + i) * uint(blurRadius+1) + uint(blurRadius - j)];
                        colorSum = colorSum + srcColor.rgb * convolutionFactor;
                    }
                }

                for(int i=1; i<=blurRadius; ++i){
                    for(int j=1; j<=blurRadius; ++j){
                        neighbourXY = clamp(sk_GlobalInvocationID.xy + uint2(i, j), uint2(0), uint2(srcDim));
                        srcColor = textureRead(src, neighbourXY);

                        convolutionFactor = in_data[uint(blurRadius - i) * uint(blurRadius+1) + uint(blurRadius - j)];
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
            const skgpu::graphite::Uniform uniforms[] = {{"boxDim", SkSLType::kInt}, {"rou", SkSLType::kFloat}};
            mgr->setExpectedUniforms(uniforms);
        )
        mgr->write(int(fKernelDim));
        mgr->write(fRou);
    }
    
    size_t calculateBufferSize(const  skgpu::graphite::DrawParams&,
                                int resourceIndex,
                                const ResourceDesc& r) const override {
        SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);
        // return sizeof(float) * (pow(fKernelDim*2 + 1, 2));
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

        // int boxWidth = fKernelDim*2 + 1;
        int boxWidth = fKernelDim + 1;
        size_t dataCount = sizeof(float) * (boxWidth * boxWidth);

        memcpy(buffer, fConvolutionMatrix.data(), bufferSize);

        SkASSERT(bufferSize == dataCount);
        SkSpan<float> inData(static_cast<float*>(buffer), dataCount);

        // for(int i=0; i<bufferSize; i++){
        //     cout<<inData[i]<<" ";
        // }
        // cout<<endl<<"------"<<endl;
    }

    WorkgroupSize calculateGlobalDispatchSize(const DP&) const override {
        return WorkgroupSize(kSrcDim / 32, kSrcDim / 32, 1);
    }
};

class SeparableGaussianBlurFilter : public ComputeImageFilter {
public:
    SeparableGaussianBlurFilter() : ComputeImageFilter(
        /*width=*/kSrcDim,
        /*height=*/kSrcDim,
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
                int boxDim;
            };
            layout(set = 0, binding = 1) readonly buffer inputBlock
            {
                float in_data[];
            };

            workgroup float3 intermediate[1024];

            void main() {
                half4 srcColor = textureRead(src, sk_GlobalInvocationID.xy);

                uint wkgpDim = 31;
                uint srcDim = 255;
                int blurRadius = boxDim;
                uint padSize = uint(boxDim);

                float3 colorSum = float3(0,0,0);
                uint2 neighbourXY = uint2(0,0);
                float convolutionFactor;

                int dstPixelsPerWg = 32 - 2 * blurRadius;
                int wgId = int(sk_WorkgroupID.y);
                int offset = -blurRadius + wgId * (32 - 2 * blurRadius);
                int2 pixelLoc = int2(int(sk_GlobalInvocationID.x), int(sk_LocalInvocationID.y) + offset);

                /*
                for (int i = -blurRadius; i <= blurRadius; ++i) {
                    neighbourXY = uint2(clamp(pixelLoc + int2(i, 0), int2(0), int2(srcDim)));

                    srcColor = textureRead(src, neighbourXY);

                    convolutionFactor = 1.0 / float(2 * blurRadius + 1);//in_data[uint(i + blurRadius)];
                    colorSum = colorSum + srcColor.rgb * convolutionFactor;
                }
                */

                for(int i=0-blurRadius; i<=0; ++i){
                    neighbourXY = uint2(clamp(pixelLoc + int2(i, 0), int2(0), int2(srcDim)));

                    srcColor = textureRead(src, neighbourXY);

                    convolutionFactor = in_data[uint(i + blurRadius)];
                    colorSum = colorSum + srcColor.rgb * convolutionFactor;
                }

                for(int i=1; i<=blurRadius; ++i){ 
                    neighbourXY = uint2(clamp(pixelLoc + int2(i, 0), int2(0), int2(srcDim)));

                    srcColor = textureRead(src, neighbourXY);

                    convolutionFactor = in_data[uint(blurRadius - i)];
                    colorSum = colorSum + srcColor.rgb * convolutionFactor;
                }

                if(padSize <= sk_LocalInvocationID.y && sk_LocalInvocationID.y <= (31 - padSize)) {
                    // colorSum = float3(0, colorSum.g, colorSum.b);
                    textureWrite(dest, uint2(pixelLoc), half4(colorSum.rgb1));
                }
                
                intermediate[sk_LocalInvocationID.x * 32 + sk_LocalInvocationID.y] = colorSum;

                workgroupBarrier();

                if(padSize <= sk_LocalInvocationID.y && sk_LocalInvocationID.y <= (31 - padSize)){
                    colorSum = float3(0,0,0);
                    
                    // radius < 15
                    for(int i=0-blurRadius; i<=0; ++i){
                         neighbourXY = clamp(sk_LocalInvocationID.xy + uint2(0, i), uint2(0), uint2(31));

                         float3 intermediateColor = intermediate[neighbourXY.x * 32 + neighbourXY.y];
                        
                         convolutionFactor = in_data[uint(i + blurRadius)];
                         colorSum = colorSum + intermediateColor * convolutionFactor;
                     }
                    
                     for(int i=1; i<=blurRadius; ++i){
                         neighbourXY = clamp(sk_LocalInvocationID.xy + uint2(0, i), uint2(0), uint2(31));

                         float3 intermediateColor = intermediate[neighbourXY.x * 32 + neighbourXY.y];
                        
                         convolutionFactor = in_data[uint(blurRadius - i)];
                         colorSum = colorSum + intermediateColor * convolutionFactor;
                     }
                    
                    textureWrite(dest, uint2(pixelLoc), half4(colorSum.rgb1));
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
            const skgpu::graphite::Uniform uniforms[] = {{"boxDim", SkSLType::kInt}};
            mgr->setExpectedUniforms(uniforms);
        )
        int padWidth = fKernelDim <= 15 ? fKernelDim : 15; 
        mgr->write(padWidth);
    }
    
    size_t calculateBufferSize(const  skgpu::graphite::DrawParams&,
                                int resourceIndex,
                                const ResourceDesc& r) const override {
        SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);
        // return sizeof(float) * (pow(fKernelDim*2 + 1, 2));
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

        int boxWidth = fKernelDim + 1;
        size_t dataCount = sizeof(float) * (boxWidth);

        memcpy(buffer, fSeparableConvolutionMatrix.data(), bufferSize);

        SkASSERT(bufferSize == dataCount);
        SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
    }

    WorkgroupSize calculateGlobalDispatchSize(const DP&) const override {
        // return WorkgroupSize(kSrcDim / 32, kSrcDim / 32, 1);

        int padWidth = fKernelDim <= 15 ? fKernelDim : 15; 
        float hSize = ceilf(float(kSrcDim) / float(32 - 2*padWidth)); //128
        // float hSize = 256;
        // cout<<hSize<<endl<<"---"<<endl;

        return WorkgroupSize(kSrcDim / 32, hSize, 1);
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
        const char* captions[] = {"Source", "Seperable gaussian", "Slow box blur", "Slow ALU gaussian blur", "Slow mem gaussian blur", "Grayscale"};
        uint32_t dim = 256;
        uint32_t margin = 10;
        for (unsigned int i = 0; i < std::size(captions); i++) {
            if (i == 0) {
                canvas->drawImage(
                        image, margin + i * (dim + margin), margin, SkSamplingOptions(), &paint);
            } 
            else if(i == 5 && fIsDisplayGrayscale) {
                device->drawComputeFilteredImage(
                        image, margin + i * (dim + margin), margin, &fGrayscale);
            }
            else if(i == 2 && fIsDisplaySlowBox){
                device->drawComputeFilteredImage(
                        image, margin + i * (dim + margin), margin, &fBlur);
            }
            else if(i == 3 && fIsDisplaySlowALUGaussian){
                device->drawComputeFilteredImage(
                        image, margin + i * (dim + margin), margin, &fGaussian);
            }            
            else if(i == 4 && fIsDisplaySlowMemGaussian){
                device->drawComputeFilteredImage(
                        image, margin + i * (dim + margin), margin, &fMemGaussian);
            }
            else if(i == 1 && fIsDisplaySeparableGaussian){
                device->drawComputeFilteredImage(
                        image, margin + i * (dim + margin), margin, &fSeparableGaussian);
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

        float blurBox = fblurBox;
        if (ImGui::SliderFloat("Radius", &blurBox, 1, 255)) {
            fblurBox = int(blurBox); 
            fBlur.setKernelDim(fblurBox);
            fGaussian.setKernelDim(fblurBox);
            fMemGaussian.setKernelDim(fblurBox);
            fSeparableGaussian.setKernelDim(fblurBox);
            if(fIsDisplaySlowMemGaussian) fMemGaussian.updateConvolMatrix();
            if(fIsDisplaySeparableGaussian) fSeparableGaussian.updateSeparableConvolMatrix();
        }
        float rou = fRou;
        if (ImGui::SliderFloat("Rou", &rou, 1, 50)) {
            fRou = rou;
            fGaussian.setRou(fRou);
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
    GrayscaleFilter fGrayscale;
    BoxBlurFilter fBlur;
    SlowALUGaussianBlurFilter fGaussian;
    SlowMemGaussianBlurFilter fMemGaussian;
    SeparableGaussianBlurFilter fSeparableGaussian;

    bool fIsDisplayGrayscale;
    bool fIsDisplaySlowBox;
    bool fIsDisplaySlowALUGaussian;
    bool fIsDisplaySlowMemGaussian;
    bool fIsDisplaySeparableGaussian = true;

    int fblurBox = 1;
    float fRou = 1;
};

}  // namespace

DEF_SLIDE( return new ComputeImageFiltersSlide; )

#endif  // defined(SK_GRAPHITE)
