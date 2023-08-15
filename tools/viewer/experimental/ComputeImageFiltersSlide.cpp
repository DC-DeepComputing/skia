// #ifdef SK_ENABLE_COMPUTE_IMAGE_FILTER_EXPERIMENTS
/*
 * Copyright 2023 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "imgui.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "tools/Resources.h"
#include "tools/viewer/ClickHandlerSlide.h"

#include <iostream>

#ifdef SK_ENABLE_COMPUTE_IMAGE_FILTER_EXPERIMENTS
#if defined(SK_GRAPHITE)

using namespace std;
namespace {

using skgpu::graphite::DrawParams;
using skgpu::graphite::ResourceBindingRequirements;
using skgpu::graphite::SamplerDesc;
using skgpu::graphite::WorkgroupSize;

class ComputeImageFilter : public skgpu::graphite::ComputeStep {
public:
    ComputeImageFilter(uint32_t width,
                       uint32_t height,
                       std::string_view name,
                       WorkgroupSize localDispatchSize,
                       SkSpan<const ResourceDesc> resources)
            : ComputeStep(name, localDispatchSize, resources)
            , fWidth(width)
            , fHeight(height)
            , fWgDim(localDispatchSize.fWidth) {}

    void setKernelDim(uint32_t d) { fKernelDim = d; }

    void setSigma(float r) { fSigma = r; }

    void updateGaussianKernel() {
        fConvolutionMatrix.clear();
        float twoSigmaSquaredInv = 0.5f / (fSigma * fSigma);

        for (int x = -fKernelDim; x <= 0; x++) {
            for (int y = -fKernelDim; y <= 0; y++) {
                float tmpFactor =
                        (M_1_PI * twoSigmaSquaredInv) * exp((x * x + y * y) * -twoSigmaSquaredInv);
                fConvolutionMatrix.push_back(tmpFactor);
            }
        }
    }

    void updateSeparableGaussianKernel() {
        fSeparableConvolutionMatrix.clear();
        float twoSigmaSquaredInv = 0.5f / (fSigma * fSigma);

        for (int i = -fKernelDim; i <= 0; i++) {
            float tmpFactor = (0.5 * M_2_SQRTPI * sqrtf(twoSigmaSquaredInv)) *
                              exp((i * i) * -twoSigmaSquaredInv);
            fSeparableConvolutionMatrix.push_back(tmpFactor);
        }
    }

    void updateHardwareGaussianKernel() {
        fHardwareConvolutionWeights.clear();
        fHardwareSampleOffsets.clear();

        if (fKernelDim % 2 != 0) {
            fHardwareConvolutionWeights.push_back(fSeparableConvolutionMatrix[0]);
            fHardwareSampleOffsets.push_back(fKernelDim);
        }
        for (int i = fKernelDim % 2 != 0 ? 1 : 0; i < int(fKernelDim); i += 2) {
            // cout<<"here"<<i<<endl;
            float offset1 = fKernelDim - i;
            float offset2 = fKernelDim - i - 1;
            float weight1 = fSeparableConvolutionMatrix[i];
            float weight2 = fSeparableConvolutionMatrix[i + 1];

            float newWeight = weight1 + weight2;
            float newOffset = (offset1 * weight1 + offset2 * weight2) / newWeight;

            fHardwareConvolutionWeights.push_back(newWeight);
            fHardwareSampleOffsets.push_back(newOffset);
        }

        fHardwareConvolutionWeights.push_back(fSeparableConvolutionMatrix[fKernelDim]);
        fHardwareSampleOffsets.push_back(0);

        // cout<<"print separable:"<<endl;
        // for(auto e:fSeparableConvolutionMatrix){
        //     cout<<e<<" ";
        // }
        // cout<<endl<<endl;

        // cout<<"print weights:"<<endl;
        // for(auto e:fHardwareConvolutionWeights){
        //     cout<<e<<" ";
        // }
        // cout<<endl<<endl;

        // cout<<"print offsets:"<<endl;
        // for(auto e:fHardwareSampleOffsets){
        //     cout<<e<<" ";
        // }
        // cout<<endl<<endl<<"--------------"<<endl;

        fmergedWeights.clear();
        fmergedWeights.reserve(fSeparableConvolutionMatrix.size() +
                               fHardwareConvolutionWeights.size() + fHardwareSampleOffsets.size());
        fmergedWeights.insert(fmergedWeights.end(),
                              fSeparableConvolutionMatrix.begin(),
                              fSeparableConvolutionMatrix.end());
        fmergedWeights.insert(fmergedWeights.end(),
                              fHardwareConvolutionWeights.begin(),
                              fHardwareConvolutionWeights.end());
        fmergedWeights.insert(
                fmergedWeights.end(), fHardwareSampleOffsets.begin(), fHardwareSampleOffsets.end());
    }

protected:
    const uint32_t fWidth;
    const uint32_t fHeight;
    const uint32_t fWgDim;

    uint32_t fKernelDim = 1;
    float fSigma = 1;
    std::vector<float> fConvolutionMatrix = {0.0625, 0.125, 0.125, 0.25};
    std::vector<float> fSeparableConvolutionMatrix = {0.241971, 0.398942};
    std::vector<float> fHardwareConvolutionWeights = {0.241971, 0.398942};
    std::vector<float> fHardwareSampleOffsets = {1, 0};

    std::vector<float> fmergedWeights = {0.241971, 0.398942, 0.241971, 0.398942, 1, 0};
};

class GrayscaleFilter : public ComputeImageFilter {
public:
    GrayscaleFilter(uint32_t srcWidth, uint32_t srcHeight, uint32_t wgDim)
            : ComputeImageFilter(
                      /*width=*/srcWidth,
                      /*height=*/srcHeight,
                      /*name=*/"GrayscalFilter_Compute",
                      /*localDispatchSize=*/{wgDim, wgDim, 1},
                      /*resources=*/
                      {{
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
                       }}) {}
    ~GrayscaleFilter() override = default;

    std::string computeSkSL(const ResourceBindingRequirements&, int) const override {
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

    WorkgroupSize calculateGlobalDispatchSize(const DrawParams&) const override {
        return WorkgroupSize(ceil(fWidth / float(fWgDim)), ceil(fHeight / float(fWgDim)), 1);
    }
};

class BoxBlurFilter : public ComputeImageFilter {
public:
    BoxBlurFilter(uint32_t srcWidth, uint32_t srcHeight, uint32_t wgDim)
            : ComputeImageFilter(
                      /*width=*/srcWidth,
                      /*height=*/srcHeight,
                      /*name=*/"BoxBlurFilter_Compute",
                      /*localDispatchSize=*/{wgDim, wgDim, 1},
                      /*resources=*/
                      {{
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
                       }}) {}
    ~BoxBlurFilter() override = default;

    std::string computeSkSL(const ResourceBindingRequirements&, int) const override {
        return R"(
            layout(set = 1, binding = 0) readonly texture2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                uint srcDim; // Dimension of image - 1
                int blurRadius;
            };

            void main() {
                half4 srcColor;
                uint2 neighbourCoord;
                float3 colorSum = float3(0);

                for (int i = -blurRadius; i <= blurRadius; ++i){
                    for (int j = -blurRadius; j <= blurRadius; ++j){
                        neighbourCoord = clamp(sk_GlobalInvocationID.xy + uint2(i, j),
                                               uint2(0),
                                               uint2(srcDim));
                        srcColor = textureRead(src, neighbourCoord);
                        colorSum += srcColor.rgb;
                    }
                }

                int boxWidth = (blurRadius * 2) + 1;
                float boxSize = float(boxWidth * boxWidth);
                colorSum = colorSum / boxSize;

                textureWrite(dest, sk_GlobalInvocationID.xy, half4(colorSum.rgb1));
            }
        )";
    }
    void prepareUniformBuffer(const DrawParams&,
                              int resourceIndex,
                              const ResourceDesc&,
                              skgpu::graphite::UniformManager* mgr) const override {
        SkASSERT(resourceIndex == 2);
        SkDEBUGCODE(const skgpu::graphite::Uniform uniforms[] = {{"srcDim", SkSLType::kInt},
                                                                 {"blurRadius", SkSLType::kInt}};
                    mgr->setExpectedUniforms(uniforms);) mgr->write(int(fWidth - 1));
        mgr->write(int(fKernelDim));
    }

    WorkgroupSize calculateGlobalDispatchSize(const DrawParams&) const override {
        return WorkgroupSize(ceil(fWidth / float(fWgDim)), ceil(fHeight / float(fWgDim)), 1);
    }
};

class SlowALUGaussianBlurFilter : public ComputeImageFilter {
public:
    SlowALUGaussianBlurFilter(uint32_t srcWidth, uint32_t srcHeight, uint32_t wgDim)
            : ComputeImageFilter(
                      /*width=*/srcWidth,
                      /*height=*/srcHeight,
                      /*name=*/"SlowALUGaussianBlurFilter_Compute",
                      /*localDispatchSize=*/{wgDim, wgDim, 1},
                      /*resources=*/
                      {{
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
                       }}) {}
    ~SlowALUGaussianBlurFilter() override = default;

    std::string computeSkSL(const ResourceBindingRequirements&, int) const override {
        return R"(
            layout(set = 1, binding = 0) readonly texture2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                uint srcDim; // Dimension of image - 1
                int blurRadius;
                float TwoSigmaSquaredInverse;
            };

            void main() {
                half4 srcColor;
                uint2 neighbourCoord;
                float3 colorSum = float3(0);

                float convolutionFactor;
                float piInverse = 0.31830988618;

                for (int i = -blurRadius; i <= blurRadius; ++i){
                    for (int j = -blurRadius; j <= blurRadius; ++j){
                        neighbourCoord = clamp(sk_GlobalInvocationID.xy + uint2(i, j),
                                               uint2(0),
                                               uint2(srcDim));
                        srcColor = textureRead(src, neighbourCoord);

                        convolutionFactor = (piInverse * TwoSigmaSquaredInverse) *
                                            exp(-1 * (float(i * i) + float(j * j)) *
                                                TwoSigmaSquaredInverse);
                        colorSum = colorSum + srcColor.rgb * convolutionFactor;
                    }
                }

                textureWrite(dest, sk_GlobalInvocationID.xy, half4(colorSum.rgb1));
            }
        )";
    }
    void prepareUniformBuffer(const DrawParams&,
                              int resourceIndex,
                              const ResourceDesc&,
                              skgpu::graphite::UniformManager* mgr) const override {
        SkASSERT(resourceIndex == 2);
        SkDEBUGCODE(const skgpu::graphite::Uniform uniforms[] = {{"srcDim", SkSLType::kInt},
                                                                 {"blurRadius", SkSLType::kInt},
                                                                 {"TwoSigmaSquaredInverse",
                                                                  SkSLType::kFloat}};
                    mgr->setExpectedUniforms(uniforms);) mgr->write(int(fWidth - 1));
        mgr->write(int(fKernelDim));
        mgr->write(1 / (2 * fSigma * fSigma));
    }

    WorkgroupSize calculateGlobalDispatchSize(const DrawParams&) const override {
        return WorkgroupSize(ceil(fWidth / float(fWgDim)), ceil(fHeight / float(fWgDim)), 1);
    }
};

class SlowMemGaussianBlurFilter : public ComputeImageFilter {
public:
    SlowMemGaussianBlurFilter(uint32_t srcWidth, uint32_t srcHeight, uint32_t wgDim)
            : ComputeImageFilter(
                      /*width=*/srcWidth,
                      /*height=*/srcHeight,
                      /*name=*/"SlowMemGaussianBlurFilter_Compute",
                      /*localDispatchSize=*/{wgDim, wgDim, 1},
                      /*resources=*/
                      {{
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
                       }}) {}
    ~SlowMemGaussianBlurFilter() override = default;

    std::string computeSkSL(const ResourceBindingRequirements&, int) const override {
        return R"(
            layout(set = 1, binding = 0) readonly texture2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                uint srcDim; // Dimension of image - 1
                int blurRadius;
            };
            layout(set = 0, binding = 1) readonly buffer inputBlock
            {
                float kernel_weights[];
            };

            void main() {
                half4 srcColor;
                uint2 neighbourCoord;
                float3 colorSum = float3(0);

                float convolutionFactor;

                for (int i = -blurRadius; i <= blurRadius; ++i){
                    for (int j = -blurRadius; j <= blurRadius; ++j){
                        neighbourCoord = clamp(sk_GlobalInvocationID.xy + uint2(i, j),
                                               uint2(0),
                                               uint2(srcDim));
                        srcColor = textureRead(src, neighbourCoord);

                        convolutionFactor = kernel_weights[uint(blurRadius - abs(i)) *
                                                           uint(blurRadius+1) +
                                                           uint(blurRadius - abs(j))];
                        colorSum = colorSum + srcColor.rgb * convolutionFactor;
                    }
                }

                textureWrite(dest, sk_GlobalInvocationID.xy, half4(colorSum.rgb1));
            }
        )";
    }
    void prepareUniformBuffer(const DrawParams&,
                              int resourceIndex,
                              const ResourceDesc&,
                              skgpu::graphite::UniformManager* mgr) const override {
        SkASSERT(resourceIndex == 2);
        SkDEBUGCODE(const skgpu::graphite::Uniform uniforms[] = {{"srcDim", SkSLType::kInt},
                                                                 {"blurRadius", SkSLType::kInt}};
                    mgr->setExpectedUniforms(uniforms);) mgr->write(int(fWidth - 1));
        mgr->write(int(fKernelDim));
    }

    size_t calculateBufferSize(const skgpu::graphite::DrawParams&,
                               int resourceIndex,
                               const ResourceDesc& r) const override {
        SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);
        return sizeof(float) * (fConvolutionMatrix.size());
    }

    void prepareStorageBuffer(const skgpu::graphite::DrawParams&,
                              int ssboIndex,
                              int resourceIndex,
                              const ResourceDesc& r,
                              void* buffer,
                              size_t bufferSize) const override {
        SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);

        memcpy(buffer, fConvolutionMatrix.data(), bufferSize);

        size_t dataCount = sizeof(float) * (fConvolutionMatrix.size());
        SkASSERT(bufferSize == dataCount);
        SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
    }

    WorkgroupSize calculateGlobalDispatchSize(const DrawParams&) const override {
        return WorkgroupSize(ceil(fWidth / float(fWgDim)), ceil(fHeight / float(fWgDim)), 1);
    }
};

class SeparableGaussianBlurFilter : public ComputeImageFilter {
public:
    SeparableGaussianBlurFilter(uint32_t srcWidth, uint32_t srcHeight, uint32_t wgDim)
            : ComputeImageFilter(
                      /*width=*/srcWidth,
                      /*height=*/srcHeight,
                      /*name=*/"SeparableGaussianBlurFilter_Compute",
                      /*localDispatchSize=*/{wgDim, wgDim, 1},
                      /*resources=*/
                      {{
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
                       }}) {}
    ~SeparableGaussianBlurFilter() override = default;

    std::string computeSkSL(const ResourceBindingRequirements&, int) const override {
        return R"(
            layout(set = 1, binding = 0) readonly texture2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                uint wgDim;
                uint srcDim; // Dimension of image - 1
                int blurRadius;
            };
            layout(set = 0, binding = 1) readonly buffer inputBlock
            {
                float kernel_weights[];
            };

            workgroup float3 intermediate[1024];

            void main() {
                half4 srcColor;
                uint2 neighbourCoord;
                float3 colorSum = float3(0);

                float convolutionFactor;

                uint wgId = sk_WorkgroupID.y;
                uint offset = uint(-blurRadius + int(wgId) * (int(wgDim) - 2 * blurRadius));
                uint2 pixelLoc = uint2(sk_GlobalInvocationID.x, sk_LocalInvocationID.y + offset);

                for (int i = -blurRadius; i <= blurRadius; ++i){
                    neighbourCoord = clamp(pixelLoc + uint2(i, 0), uint2(0), uint2(srcDim));
                    srcColor = textureRead(src, neighbourCoord);

                    convolutionFactor = kernel_weights[uint(blurRadius - abs(i))];
                    colorSum = colorSum + srcColor.rgb * convolutionFactor;
                }

                intermediate[sk_LocalInvocationID.x * wgDim + sk_LocalInvocationID.y] = colorSum;
                workgroupBarrier();

                if (uint(blurRadius) <= sk_LocalInvocationID.y &&
                    sk_LocalInvocationID.y <= uint(31 - blurRadius)){
                    colorSum = float3(0);
                    float3 intermediateColor;

                    for (int i = -blurRadius; i <= blurRadius; ++i){
                        neighbourCoord = clamp(sk_LocalInvocationID.xy + uint2(0, i),
                                               uint2(0),
                                               uint2(31));
                        intermediateColor = intermediate[neighbourCoord.x * wgDim + neighbourCoord.y];

                        convolutionFactor = kernel_weights[uint(blurRadius - abs(i))];
                        colorSum = colorSum + intermediateColor * convolutionFactor;
                    }

                    textureWrite(dest, pixelLoc, half4(colorSum.rgb1));
                }
            }
        )";
    }
    void prepareUniformBuffer(const DrawParams&,
                              int resourceIndex,
                              const ResourceDesc&,
                              skgpu::graphite::UniformManager* mgr) const override {
        SkASSERT(resourceIndex == 2);
        SkDEBUGCODE(const skgpu::graphite::Uniform uniforms[] = {{"wgDim", SkSLType::kInt},
                                                                 {"srcDim", SkSLType::kInt},
                                                                 {"blurRadius", SkSLType::kInt}};
                    mgr->setExpectedUniforms(uniforms);) mgr->write(int(fWgDim));
        mgr->write(int(fWidth - 1));
        mgr->write(int(fKernelDim));
    }

    size_t calculateBufferSize(const skgpu::graphite::DrawParams&,
                               int resourceIndex,
                               const ResourceDesc& r) const override {
        SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);
        return sizeof(float) * (fKernelDim + 1);
    }

    void prepareStorageBuffer(const skgpu::graphite::DrawParams&,
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

    WorkgroupSize calculateGlobalDispatchSize(const DrawParams&) const override {
        float hSize = ceilf(float(fHeight) / float(fWgDim - 2 * fKernelDim));
        return WorkgroupSize(ceil(fWidth / float(fWgDim)), hSize, 1);
    }
};

class HardwareHorizontalGaussianBlurFilter : public ComputeImageFilter {
public:
    HardwareHorizontalGaussianBlurFilter(uint32_t srcWidth, uint32_t srcHeight, uint32_t wgDim)
            : ComputeImageFilter(
                      /*width=*/srcWidth,
                      /*height=*/srcHeight,
                      /*name=*/"HardwareHorizontalGaussianBlurFilter_Compute",
                      /*localDispatchSize=*/{wgDim, wgDim, 1},
                      /*resources=*/
                      {{
                               /*type=*/ResourceType::kTexture,
                               /*flow=*/DataFlow::kShared,
                               /*policy=*/ResourcePolicy::kNone,
                               /*slot=*/0,
                       },
                       {
                               /*type=*/ResourceType::kSampler,
                               /*flow=*/DataFlow::kPrivate,
                               /*policy=*/ResourcePolicy::kNone,
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
                       }}) {}
    ~HardwareHorizontalGaussianBlurFilter() override = default;

    std::string computeSkSL(const ResourceBindingRequirements&, int) const override {
        return R"(
            layout(set = 1, binding = 0) sampler2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                float srcDimInv;
                uint srcDim; // Dimension of image - 1
                uint wgDim;
                int blurRadius;
                int shortcutDataCount;
            };
            layout(set = 0, binding = 1) readonly buffer inputBlock
            {
                float kernel_weights[];
            };

            workgroup float3 intermediate[1024];

            void main() {
                // uint2 dstCoord = sk_GlobalInvocationID.xy;
                half4 srcColor;
                half2 neighbourCoord;
                float3 colorSum = float3(0);

                float2 unormCoord;

                float convolutionFactor;


                uint wgId = sk_WorkgroupID.y;
                uint offset = uint(-blurRadius + int(wgId) * (int(wgDim) - 2 * blurRadius));
                uint2 pixelLoc = uint2(sk_GlobalInvocationID.x, sk_LocalInvocationID.y + offset);

                // for (int i = -blurRadius/2; i <= blurRadius/2; ++i){
                //     //neighbourCoord = clamp(half2(pixelLoc) + half2(0.6*float(i*abs(i)), 0), half2(0), half2(srcDim));
                //     neighbourCoord = clamp(half2(pixelLoc) + half2(float(2*i)+1.5, 0), half2(0), half2(srcDim));
                //     unormCoord = float2(neighbourCoord) * srcDimInv;
                //     srcColor = sampleLod(src, unormCoord, 0);

                //     convolutionFactor = kernel_weights[uint(blurRadius - abs(i))];
                //     colorSum = colorSum + srcColor.rgb * convolutionFactor;
                // }

                float tmp = 1;
                int weightsOffset = blurRadius + 1;
                int positionOffset = blurRadius + 1 + shortcutDataCount + 1;

                for (int i = -shortcutDataCount; i <= shortcutDataCount; ++i){
                    if(i<0){
                        tmp = -1;
                    }
                    else{
                        tmp = 1;
                    }
                    neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * kernel_weights[positionOffset + shortcutDataCount - abs(i)], 0), half2(0), half2(srcDim));
                    // neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * kernel_weights[positionOffset + shortcutDataCount - abs(i)], float(2*i)+1.5), half2(0), half2(srcDim));
                    unormCoord = float2(neighbourCoord) * srcDimInv;
                    srcColor = sampleLod(src, unormCoord, 0);

                    convolutionFactor = kernel_weights[uint(weightsOffset + shortcutDataCount - abs(i))];
                    colorSum = colorSum + srcColor.rgb * convolutionFactor;
                }


                intermediate[sk_LocalInvocationID.x * wgDim + sk_LocalInvocationID.y] = colorSum;
                workgroupBarrier();

                if (uint(blurRadius) <= sk_LocalInvocationID.y &&
                    sk_LocalInvocationID.y <= uint(31 - blurRadius)){
                    colorSum = float3(0);
                    float3 intermediateColor;
                    uint2 floorNeighbour;

                    for (int i = -blurRadius; i <= blurRadius; ++i){
                        // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 0.6*float(i*abs(i))),
                        //                        half2(0),
                        //                        half2(31));
                        neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, i),
                                               half2(0),
                                               half2(31));
                        floorNeighbour = uint2(floor(neighbourCoord));
                        intermediateColor = intermediate[floorNeighbour.x * wgDim + floorNeighbour.y];

                        convolutionFactor = kernel_weights[uint(blurRadius - abs(i))];
                        colorSum = colorSum + intermediateColor * convolutionFactor;
                    }

                    textureWrite(dest, pixelLoc, half4(colorSum.rgb1));
                }
            }
        )";
    }
    std::tuple<SkISize, SkColorType> calculateTextureParameters(
            const DrawParams&, int index, const ResourceDesc& r) const override {
        SkASSERT(index == 2);
        return {{static_cast<int32_t>(fWgDim), static_cast<int32_t>(fWgDim)},
                kRGBA_8888_SkColorType};
    }

    SamplerDesc calculateSamplerParameters(const DrawParams&,
                                           int index,
                                           const ResourceDesc&) const override {
        SkASSERT(index == 1);
        // Use the repeat tile mode to sample an infinite checkerboard.
        constexpr SkTileMode kTileModes[2] = {SkTileMode::kRepeat, SkTileMode::kRepeat};
        return {SkFilterMode::kLinear, kTileModes};
    }

    void prepareUniformBuffer(const DrawParams&,
                              int resourceIndex,
                              const ResourceDesc&,
                              skgpu::graphite::UniformManager* mgr) const override {
        // SkASSERT(resourceIndex == 0);
        SkDEBUGCODE(
                const skgpu::graphite::Uniform uniforms[] = {{"srcDimInv", SkSLType::kFloat},
                                                             {"srcDim", SkSLType::kInt},
                                                             {"wgDim", SkSLType::kInt},
                                                             {"blurRaidus", SkSLType::kInt},
                                                             {"shortcutDataCount", SkSLType::kInt}};
                mgr->setExpectedUniforms(uniforms);) mgr->write(float(1.0 / fWidth));
        mgr->write(int(fWidth - 1));
        mgr->write(int(fWgDim));
        mgr->write(int(fKernelDim));
        mgr->write(int(fHardwareConvolutionWeights.size() - 1));
    }

    size_t calculateBufferSize(const skgpu::graphite::DrawParams&,
                               int resourceIndex,
                               const ResourceDesc& r) const override {
        // SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);
        return sizeof(float) * (fKernelDim + 1 + fHardwareConvolutionWeights.size() * 2);
    }

    void prepareStorageBuffer(const skgpu::graphite::DrawParams&,
                              int ssboIndex,
                              int resourceIndex,
                              const ResourceDesc& r,
                              void* buffer,
                              size_t bufferSize) const override {
        // SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);

        memcpy(buffer, fmergedWeights.data(), bufferSize);

        int boxWidth = fKernelDim + 1;
        size_t dataCount = sizeof(float) * (boxWidth + fHardwareConvolutionWeights.size() * 2);
        SkASSERT(bufferSize == dataCount);
        SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
    }

    WorkgroupSize calculateGlobalDispatchSize(const DrawParams&) const override {
        float hSize = ceilf(float(fHeight) / float(fWgDim - 2 * fKernelDim));
        return WorkgroupSize(ceil(fWidth / float(fWgDim)), hSize, 1);
    }
};

class ExtendRadiusGaussianBlurFilter : public ComputeImageFilter {
public:
    const uint32_t specialDim = 16;

    ExtendRadiusGaussianBlurFilter(uint32_t srcWidth, uint32_t srcHeight, uint32_t wgDim)
            : ComputeImageFilter(
                      /*width=*/srcWidth,
                      /*height=*/srcHeight,
                      /*name=*/"ExtendRadiusGaussianBlurFilter_Compute",
                      /*localDispatchSize=*/{16, 16, 1},
                      /*resources=*/
                      {{
                               /*type=*/ResourceType::kTexture,
                               /*flow=*/DataFlow::kShared,
                               /*policy=*/ResourcePolicy::kNone,
                               /*slot=*/0,
                       },
                       {
                               /*type=*/ResourceType::kSampler,
                               /*flow=*/DataFlow::kPrivate,
                               /*policy=*/ResourcePolicy::kNone,
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
                       }}) {}
    ~ExtendRadiusGaussianBlurFilter() override = default;

    std::string computeSkSL(const ResourceBindingRequirements&, int) const override {
        return R"(
            layout(set = 1, binding = 0) sampler2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                float srcDimInv;
                uint srcDim; // Dimension of image - 1
                uint wgDim;
                int blurRadius;
                int shortcutDataCount;
            };
            layout(set = 0, binding = 1) readonly buffer inputBlock
            {
                float kernel_weights[];
            };

            workgroup float3 intermediate[512];

            void main() {
                // half4 srcColor;
                // half2 neighbourCoord;
                // // float3 colorSum = float3(0);

                // float3 colorSum1 = float3(0);
                // float3 colorSum2 = float3(0);

                // float2 unormCoord;
                // float convolutionFactor;


                // uint wgId = sk_WorkgroupID.y;
                // // uint offset = uint(-blurRadius + int(wgId) * (int(wgDim) - 2 * blurRadius));
                // // uint2 pixelLoc = uint2(sk_GlobalInvocationID.x, sk_LocalInvocationID.y + offset);

                // uint offset = uint(-blurRadius + int(wgId) * (2 * int(wgDim) - 2 * blurRadius));
                // uint2 pixelLoc1 = uint2(sk_GlobalInvocationID.x, 2 * sk_LocalInvocationID.y + offset);
                // uint2 pixelLoc2 = uint2(sk_GlobalInvocationID.x, 2 * sk_LocalInvocationID.y + 1 + offset);

                // float tmp = 1;
                // int weightsOffset = blurRadius + 1;
                // int positionOffset = blurRadius + 1 + shortcutDataCount + 1;

                // for (int i = -shortcutDataCount; i <= shortcutDataCount; ++i){
                //     if(i<0){tmp = -1;}
                //     else{tmp = 1;}

                //     convolutionFactor = kernel_weights[uint(weightsOffset + shortcutDataCount - abs(i))];

                //     neighbourCoord = clamp(half2(pixelLoc1) + half2(tmp * kernel_weights[positionOffset + shortcutDataCount - abs(i)], 0), half2(0), half2(srcDim));
                //     unormCoord = float2(neighbourCoord) * srcDimInv;
                //     srcColor = sampleLod(src, unormCoord, 0);
                //     colorSum1 = colorSum1 + srcColor.rgb * convolutionFactor;

                //     neighbourCoord = clamp(half2(pixelLoc2) + half2(tmp * kernel_weights[positionOffset + shortcutDataCount - abs(i)], 0), half2(0), half2(srcDim));
                //     unormCoord = float2(neighbourCoord) * srcDimInv;
                //     srcColor = sampleLod(src, unormCoord, 0);
                //     colorSum2 = colorSum2 + srcColor.rgb * convolutionFactor;
                // }

                // intermediate[sk_LocalInvocationID.x * 2*wgDim + 2 * sk_LocalInvocationID.y] = colorSum1;
                // intermediate[sk_LocalInvocationID.x * 2*wgDim + 2 * sk_LocalInvocationID.y + 1] = colorSum2;

                // workgroupBarrier();

                // if (sk_LocalInvocationID.y <= (wgDim - 1 - uint(blurRadius))){

                //     // colorSum = float3(0);

                //     pixelLoc1 = pixelLoc1 + uint2(0,blurRadius);
                //     pixelLoc2 = pixelLoc2 + uint2(0,blurRadius);

                //     colorSum1 = float3(0);
                //     colorSum2 = float3(0);
                //     float3 intermediateColor;
                //     uint2 floorNeighbour;


                //     for (int i = -blurRadius; i <= blurRadius; ++i){
                //         convolutionFactor = kernel_weights[uint(blurRadius - abs(i))];


                //         neighbourCoord = clamp(half2(sk_LocalInvocationID.x, 2 * sk_LocalInvocationID.y + uint(blurRadius)) + half2(0, i),
                //                                half2(0),
                //                                half2(2*wgDim-1));
                //         floorNeighbour = uint2(floor(neighbourCoord));
                //         intermediateColor = intermediate[floorNeighbour.x * 2 * wgDim + floorNeighbour.y];
                //         colorSum1 = colorSum1 + intermediateColor * convolutionFactor;


                //         neighbourCoord = clamp(half2(sk_LocalInvocationID.x, 2 * sk_LocalInvocationID.y + 1 + uint(blurRadius)) + half2(0, i),
                //                                half2(0),
                //                                half2(2*wgDim-1));
                //         floorNeighbour = uint2(floor(neighbourCoord));
                //         intermediateColor = intermediate[floorNeighbour.x * 2 * wgDim + floorNeighbour.y];
                //         colorSum2 = colorSum2 + intermediateColor * convolutionFactor;


                //         // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, i),
                //         //                        half2(0),
                //         //                        half2(wgDim-1));
                //     }

                //     textureWrite(dest, pixelLoc1, half4(colorSum1.rgb1));
                //     textureWrite(dest, pixelLoc2, half4(colorSum2.rgb1));
                // }
            }
        )";
    }
    std::tuple<SkISize, SkColorType> calculateTextureParameters(
            const DrawParams&, int index, const ResourceDesc& r) const override {
        SkASSERT(index == 2);
        return {{static_cast<int32_t>(specialDim), static_cast<int32_t>(specialDim)},
                kRGBA_8888_SkColorType};
    }

    SamplerDesc calculateSamplerParameters(const DrawParams&,
                                           int index,
                                           const ResourceDesc&) const override {
        SkASSERT(index == 1);
        // Use the repeat tile mode to sample an infinite checkerboard.
        constexpr SkTileMode kTileModes[2] = {SkTileMode::kRepeat, SkTileMode::kRepeat};
        return {SkFilterMode::kLinear, kTileModes};
    }

    void prepareUniformBuffer(const DrawParams&,
                              int resourceIndex,
                              const ResourceDesc&,
                              skgpu::graphite::UniformManager* mgr) const override {
        // SkASSERT(resourceIndex == 0);
        SkDEBUGCODE(
                const skgpu::graphite::Uniform uniforms[] = {{"srcDimInv", SkSLType::kFloat},
                                                             {"srcDim", SkSLType::kInt},
                                                             {"wgDim", SkSLType::kInt},
                                                             {"blurRaidus", SkSLType::kInt},
                                                             {"shortcutDataCount", SkSLType::kInt}};
                mgr->setExpectedUniforms(uniforms);) mgr->write(float(1.0 / fWidth));
        mgr->write(int(fWidth - 1));
        mgr->write(int(specialDim));
        mgr->write(int(fKernelDim));
        mgr->write(int(fHardwareConvolutionWeights.size() - 1));
    }

    size_t calculateBufferSize(const skgpu::graphite::DrawParams&,
                               int resourceIndex,
                               const ResourceDesc& r) const override {
        // SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);
        return sizeof(float) * (fKernelDim + 1 + fHardwareConvolutionWeights.size() * 2);
    }

    void prepareStorageBuffer(const skgpu::graphite::DrawParams&,
                              int ssboIndex,
                              int resourceIndex,
                              const ResourceDesc& r,
                              void* buffer,
                              size_t bufferSize) const override {
        // SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);

        memcpy(buffer, fmergedWeights.data(), bufferSize);

        int boxWidth = fKernelDim + 1;
        size_t dataCount = sizeof(float) * (boxWidth + fHardwareConvolutionWeights.size() * 2);
        SkASSERT(bufferSize == dataCount);
        SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
    }

    WorkgroupSize calculateGlobalDispatchSize(const DrawParams&) const override {
        float hSize = ceilf(float(fHeight) / float(specialDim - 2 * fKernelDim) / 2.0);
        return WorkgroupSize(ceil(fWidth / float(specialDim)), hSize, 1);
    }
};

class HardwareVerticalGaussianBlurFilter : public ComputeImageFilter {
public:
    HardwareVerticalGaussianBlurFilter(uint32_t srcWidth, uint32_t srcHeight, uint32_t wgDim)
            : ComputeImageFilter(
                      /*width=*/srcWidth,
                      /*height=*/srcHeight,
                      /*name=*/"HardwareVerticalGaussianBlurFilter_Compute",
                      /*localDispatchSize=*/{wgDim, wgDim, 1},
                      /*resources=*/
                      {{
                               /*type=*/ResourceType::kTexture,
                               /*flow=*/DataFlow::kShared,
                               /*policy=*/ResourcePolicy::kNone,
                               /*slot=*/0,
                       },
                       {
                               /*type=*/ResourceType::kSampler,
                               /*flow=*/DataFlow::kPrivate,
                               /*policy=*/ResourcePolicy::kNone,
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
                       }}) {}
    ~HardwareVerticalGaussianBlurFilter() override = default;

    std::string computeSkSL(const ResourceBindingRequirements&, int) const override {
        return R"(

        )";
    }
    std::tuple<SkISize, SkColorType> calculateTextureParameters(
            const DrawParams&, int index, const ResourceDesc& r) const override {
        SkASSERT(index == 2);
        return {{static_cast<int32_t>(fWgDim), static_cast<int32_t>(fWgDim)},
                kRGBA_8888_SkColorType};
    }

    SamplerDesc calculateSamplerParameters(const DrawParams&,
                                           int index,
                                           const ResourceDesc&) const override {
        SkASSERT(index == 1);
        // Use the repeat tile mode to sample an infinite checkerboard.
        constexpr SkTileMode kTileModes[2] = {SkTileMode::kRepeat, SkTileMode::kRepeat};
        return {SkFilterMode::kLinear, kTileModes};
    }

    void prepareUniformBuffer(const DrawParams&,
                              int resourceIndex,
                              const ResourceDesc&,
                              skgpu::graphite::UniformManager* mgr) const override {
        // SkASSERT(resourceIndex == 0);
        SkDEBUGCODE(
                const skgpu::graphite::Uniform uniforms[] = {{"srcDimInv", SkSLType::kFloat},
                                                             {"srcDim", SkSLType::kInt},
                                                             {"wgDim", SkSLType::kInt},
                                                             {"blurRaidus", SkSLType::kInt},
                                                             {"shortcutDataCount", SkSLType::kInt}};
                mgr->setExpectedUniforms(uniforms);) mgr->write(float(1.0 / fWidth));
        mgr->write(int(fWidth - 1));
        mgr->write(int(fWgDim));
        mgr->write(int(fKernelDim));
        mgr->write(int(fHardwareConvolutionWeights.size() - 1));
    }

    size_t calculateBufferSize(const skgpu::graphite::DrawParams&,
                               int resourceIndex,
                               const ResourceDesc& r) const override {
        // SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);
        return sizeof(float) * (fKernelDim + 1 + fHardwareConvolutionWeights.size() * 2);
    }

    void prepareStorageBuffer(const skgpu::graphite::DrawParams&,
                              int ssboIndex,
                              int resourceIndex,
                              const ResourceDesc& r,
                              void* buffer,
                              size_t bufferSize) const override {
        // SkASSERT(resourceIndex == 3);
        SkASSERT(r.fFlow == DataFlow::kPrivate);

        memcpy(buffer, fmergedWeights.data(), bufferSize);

        int boxWidth = fKernelDim + 1;
        size_t dataCount = sizeof(float) * (boxWidth + fHardwareConvolutionWeights.size() * 2);
        SkASSERT(bufferSize == dataCount);
        SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
    }

    WorkgroupSize calculateGlobalDispatchSize(const DrawParams&) const override {
        float hSize = ceilf(float(fHeight) / float(fWgDim - 2 * fKernelDim));
        return WorkgroupSize(ceil(fWidth / float(fWgDim)), hSize, 1);
    }
};

class ComputeImageFiltersSlide final : public ClickHandlerSlide {
public:
    ComputeImageFiltersSlide(sk_sp<SkImage> image, std::string imageName, uint32_t wgDim = 32)
            : fImage(std::move(image))
            , fImageName(std::move(imageName))
            , fGrayscale(fImage->width(), fImage->height(), wgDim)
            , fBoxBlur(fImage->width(), fImage->height(), wgDim)
            , fALUGaussian(fImage->width(), fImage->height(), wgDim)
            , fMemGaussian(fImage->width(), fImage->height(), wgDim)
            , fSeparableGaussian(fImage->width(), fImage->height(), wgDim)
            , fHardwareHorizontalGaussian(fImage->width(), fImage->height(), wgDim)
            , fHardwareVerticalGaussian(fImage->width(), fImage->height(), wgDim)
            , fExtendRadiusGaussian(fImage->width(), fImage->height(), wgDim) {
        fName = ("ImageFilters_Compute_" + fImageName).c_str();
    }

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
            canvas->drawString(
                    ("Could not load " + fImageName).c_str(), 10, 30, font, captionPaint);
            return;
        }

        canvas->clear(SK_ColorWHITE);

        SkPaint paint;
        const char* captions[] = {"Source",
                                  "Seperable gaussian",
                                  "Slow box blur",
                                  "Slow ALU gaussian blur",
                                  "Slow mem gaussian blur",
                                  "Grayscale"};
        uint32_t dim = fImage->width();  // assuming image is square
        uint32_t margin = 10;

        // device->drawComputeFilteredImage(
        //         fImage, margin + 0 * (dim + margin), (4 * margin) + dim,
        //         &fHardwareHorizontalGaussian);
        // device->drawComputeFilteredImage(
        //         fImage, margin + 1 * (dim + margin), (4 * margin) + dim,
        //         &fHardwareVerticalGaussian);
        device->drawComputeFilteredImage(
                fImage, margin + 2 * (dim + margin), (4 * margin) + dim, &fExtendRadiusGaussian);

        for (unsigned int i = 0; i < std::size(captions); i++) {
            if (i == 0) {
                canvas->drawImage(
                        fImage, margin + i * (dim + margin), margin, SkSamplingOptions(), &paint);
            } else if (i == 5 && fIsDisplayGrayscale) {
                device->drawComputeFilteredImage(
                        fImage, margin + i * (dim + margin), margin, &fGrayscale);
            } else if (i == 2 && fIsDisplaySlowBox) {
                device->drawComputeFilteredImage(
                        fImage, margin + i * (dim + margin), margin, &fBoxBlur);
            } else if (i == 3 && fIsDisplaySlowALUGaussian) {
                device->drawComputeFilteredImage(
                        fImage, margin + i * (dim + margin), margin, &fALUGaussian);
            } else if (i == 4 && fIsDisplaySlowMemGaussian) {
                device->drawComputeFilteredImage(
                        fImage, margin + i * (dim + margin), margin, &fMemGaussian);
            } else if (i == 1 && fIsDisplaySeparableGaussian) {
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
        if (ImGui::SliderFloat("Radius",
                               &blurBox,
                               1,
                               fIsDisplaySeparableGaussian || fIsDisplayHardwareHorizontalGaussian
                                       ? 15
                                       : 64)) {
            fKernelDim = int(blurBox);
            fSigma = fmax(fKernelDim * 0.333333333, 1);

            fBoxBlur.setKernelDim(fKernelDim);
            fALUGaussian.setKernelDim(fKernelDim);
            fMemGaussian.setKernelDim(fKernelDim);
            fSeparableGaussian.setKernelDim(fKernelDim);
            fHardwareHorizontalGaussian.setKernelDim(fKernelDim);
            fHardwareVerticalGaussian.setKernelDim(fKernelDim);
            fExtendRadiusGaussian.setKernelDim(fKernelDim);
            fALUGaussian.setSigma(fSigma);
            fMemGaussian.setSigma(fSigma);
            fSeparableGaussian.setSigma(fSigma);
            fHardwareHorizontalGaussian.setSigma(fSigma);
            fHardwareVerticalGaussian.setSigma(fSigma);
            fExtendRadiusGaussian.setSigma(fSigma);
            if (fIsDisplaySlowMemGaussian) fMemGaussian.updateGaussianKernel();
            if (fIsDisplaySeparableGaussian) fSeparableGaussian.updateSeparableGaussianKernel();
            if (fIsDisplayHardwareHorizontalGaussian)
                fHardwareHorizontalGaussian.updateHardwareGaussianKernel();
            if (fIsDisplayHardwareHorizontalGaussian)
                fHardwareHorizontalGaussian.updateSeparableGaussianKernel();
            if (fIsDisplayHardwareVerticalGaussian)
                fHardwareVerticalGaussian.updateSeparableGaussianKernel();
            if (fIsDisplayHardwareVerticalGaussian)
                fHardwareVerticalGaussian.updateHardwareGaussianKernel();
            if (fIsDisplayExtendRadiusGaussian)
                fExtendRadiusGaussian.updateSeparableGaussianKernel();
            if (fIsDisplayExtendRadiusGaussian)
                fExtendRadiusGaussian.updateHardwareGaussianKernel();
        }

        float sigma = fSigma;
        if (ImGui::SliderFloat("Sigma", &sigma, 1, std::max(fKernelDim / 3 * 2, 2))) {
            fSigma = sigma;
            fALUGaussian.setSigma(fSigma);
            fMemGaussian.setSigma(fSigma);
            fSeparableGaussian.setSigma(fSigma);
            fHardwareHorizontalGaussian.setSigma(fSigma);
            fHardwareVerticalGaussian.setSigma(fSigma);
            fExtendRadiusGaussian.setSigma(fSigma);
            if (fIsDisplaySlowMemGaussian) fMemGaussian.updateGaussianKernel();
            if (fIsDisplaySeparableGaussian) fSeparableGaussian.updateSeparableGaussianKernel();
            if (fIsDisplayHardwareHorizontalGaussian)
                fHardwareHorizontalGaussian.updateSeparableGaussianKernel();
            if (fIsDisplayHardwareHorizontalGaussian)
                fHardwareHorizontalGaussian.updateHardwareGaussianKernel();
            if (fIsDisplayHardwareVerticalGaussian)
                fHardwareVerticalGaussian.updateSeparableGaussianKernel();
            if (fIsDisplayHardwareVerticalGaussian)
                fHardwareVerticalGaussian.updateHardwareGaussianKernel();
            if (fIsDisplayExtendRadiusGaussian)
                fExtendRadiusGaussian.updateSeparableGaussianKernel();
            if (fIsDisplayExtendRadiusGaussian)
                fExtendRadiusGaussian.updateHardwareGaussianKernel();
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
    HardwareHorizontalGaussianBlurFilter fHardwareHorizontalGaussian;
    HardwareVerticalGaussianBlurFilter fHardwareVerticalGaussian;
    ExtendRadiusGaussianBlurFilter fExtendRadiusGaussian;

    bool fIsDisplayGrayscale;
    bool fIsDisplaySlowBox;
    bool fIsDisplaySlowALUGaussian;
    bool fIsDisplaySlowMemGaussian;
    bool fIsDisplaySeparableGaussian;
    bool fIsDisplayHardwareHorizontalGaussian = 1;
    bool fIsDisplayHardwareVerticalGaussian = 1;
    bool fIsDisplayExtendRadiusGaussian = 1;

    int fKernelDim = 1;
    float fSigma = 1;
};

}  // namespace

DEF_SLIDE(std::string imageName = "mandrill_256.png"; std::string imagePath = "images/" + imageName;
          return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName);)

// DEF_SLIDE(std::string imageName = "mandrill_512.png"; std::string imagePath = "images/" +
// imageName;
//           return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName);)

// DEF_SLIDE(std::string imageName = "mandrill_1600.png";
//           std::string imagePath = "images/" + imageName;
//           return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName);)

// DEF_SLIDE(std::string imageName = "puppy_999.png"; std::string imagePath = "images/" + imageName;
//           return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName);)

#endif  // defined(SK_GRAPHITE)
#endif  // SK_ENABLE_COMPUTE_IMAGE_FILTER_EXPERIMENTS
