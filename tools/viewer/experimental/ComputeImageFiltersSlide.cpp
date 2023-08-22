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

class HackyGaussianFilter : public ComputeImageFilter {
public:
    HackyGaussianFilter(uint32_t srcWidth, uint32_t srcHeight, uint32_t wgDim)
            : ComputeImageFilter(
                      /*width=*/srcWidth,
                      /*height=*/srcHeight,
                      /*name=*/"HackyGaussianFilter_Compute",
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
                    //    {
                    //            /*type=*/ResourceType::kStorageBuffer,
                    //            /*flow=*/DataFlow::kPrivate,
                    //            /*policy=*/ResourcePolicy::kMapped,
                    //            /*slot=*/2,
                    //    }
                    }) {}
    ~HackyGaussianFilter() override = default;

    std::string computeSkSL(const ResourceBindingRequirements&, int) const override {
        return R"(
            layout(set = 1, binding = 0) sampler2D src;
            layout(set = 1, binding = 1) writeonly texture2D dest;
            layout(set = 0, binding = 0) uniform uniformBlock {
                float srcDimInv;
                uint srcDim; // Dimension of image - 1
                uint wgDim;
                int blurRadius; // 14
                int shortcutDataCount; // 7

                float w0;
                float w1;
                float w2;
                float w3;
                float w4;
                float w5;
                float w6;
                float w7;
                float w8;
                float w9;
                float w10;
                float w11;
                float w12;
                float w13;
                float w14;
                float w15;
                float w16;
                float w17;
                float w18;
                float w19;
                float w20;
                float w21;
                float w22;
                float w23;
                float w24;
                float w25;
                float w26;
                float w27;
                float w28;
                float w29;
                float w30;
            };
            // layout(set = 0, binding = 1) readonly buffer inputBlock
            // {
            //     float kernel_weights[];
            // };

            workgroup float3 intermediate[1024];

            void main() {
                half4 srcColor;
                half2 neighbourCoord;
                float3 colorSum = float3(0);

                float2 unormCoord;
                float convolutionFactor;

                uint wgId = sk_WorkgroupID.y;
                uint offset = uint(-blurRadius + int(wgId) * (int(wgDim) - 2 * blurRadius));
                uint2 pixelLoc = uint2(sk_GlobalInvocationID.x, sk_LocalInvocationID.y + offset);


                float tmp = -1;
                int weightsOffset = 15; // blurRadius + 1;
                int positionOffset = 23;  // blurRadius + 1 + shortcutDataCount + 1;

                // for (int i = -7; i <= 7; ++i){
                //     if(i<0){
                //         tmp = -1;
                //     }
                //     else{
                //         tmp = 1;
                //     }
                //     neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * kernel_weights[positionOffset + shortcutDataCount - abs(i)], 0), half2(0), half2(srcDim));
                //     // neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * srcDimInv, 0), half2(0), half2(srcDim));
                //     unormCoord = float2(neighbourCoord) * srcDimInv;
                //     srcColor = sampleLod(src, unormCoord, 0);

                //     convolutionFactor = kernel_weights[uint(weightsOffset + shortcutDataCount - abs(i))];
                //     colorSum = colorSum + srcColor.rgb * convolutionFactor;
                // }

                // -7
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w23, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w15;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;

                // -6
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w24, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w16;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;

                // -5
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w25, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w17;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;

                // -4
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w26, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w18;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;

                // -3
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w27, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w19;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;

                // -2
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w28, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w20;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;

                // -1
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w29, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w21;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;

                tmp = 1;

                // 0
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w30, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w22;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;


                // 1
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w29, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w21;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;

                // 2
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w28, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w20;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;


                // 3
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w27, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w19;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;


                // 4
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w26, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w18;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;


                // 5
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w25, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w17;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;


                // 6
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w24, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w16;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;


                // 7
                neighbourCoord = clamp(half2(pixelLoc) + half2(tmp * w23, 0), half2(0), half2(srcDim));
                unormCoord = float2(neighbourCoord) * srcDimInv;
                srcColor = sampleLod(src, unormCoord, 0);
                convolutionFactor = w15;
                colorSum = colorSum + srcColor.rgb * convolutionFactor;






                intermediate[sk_LocalInvocationID.x + wgDim * sk_LocalInvocationID.y] = colorSum;
                workgroupBarrier();

                if (uint(blurRadius) <= sk_LocalInvocationID.y &&
                    sk_LocalInvocationID.y <= uint(31 - blurRadius)){

                    textureWrite(dest, pixelLoc, half4(1,0,1,1));

                    colorSum = float3(0);
                    float3 intermediateColor;
                    uint2 floorNeighbour;

                    // for (int i = -14; i <= 14; ++i){
                    //     neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, i),
                    //                            half2(0),
                    //                            half2(31));
                    //     floorNeighbour = uint2(floor(neighbourCoord));
                    //     intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];

                    //     convolutionFactor = kernel_weights[uint(blurRadius - abs(i))];

                    //     colorSum = colorSum + intermediateColor * convolutionFactor;
                    // }




                    // -14
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -14),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w0;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -13
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -13),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w1;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -12
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -12),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w2;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -11
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -11),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w3;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -10
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -10),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w4;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -9
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -9),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w5;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -8
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -8),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w6;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -7
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -7),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w7;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -6
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -6),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w8;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -5
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -5),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w9;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -4
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -4),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w10;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -3
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -3),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w11;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -2
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -2),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w12;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // -1
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, -1),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w13;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // 0
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 0),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w14;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // 1
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 1),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w13;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // 2
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 2),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w12;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // 3
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 3),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w11;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // 4
                    neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 4),
                                               half2(0),
                                               half2(31));
                    floorNeighbour = uint2(floor(neighbourCoord));
                    intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    convolutionFactor = w10;
                    colorSum = colorSum + intermediateColor * convolutionFactor;

                    // 5
                    // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 5),
                    //                            half2(0),
                    //                            half2(31));
                    // floorNeighbour = uint2(floor(neighbourCoord));
                    // intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    // convolutionFactor = w9;
                    // colorSum += intermediateColor * convolutionFactor;

                    // // 6
                    // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 6),
                    //                            half2(0),
                    //                            half2(31));
                    // floorNeighbour = uint2(floor(neighbourCoord));
                    // intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    // convolutionFactor = w8;
                    // colorSum = colorSum + intermediateColor * convolutionFactor;

                    // // 7
                    // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 7),
                    //                            half2(0),
                    //                            half2(31));
                    // floorNeighbour = uint2(floor(neighbourCoord));
                    // intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    // convolutionFactor = w7;
                    // colorSum = colorSum + intermediateColor * convolutionFactor;

                    // // 8
                    // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 8),
                    //                            half2(0),
                    //                            half2(31));
                    // floorNeighbour = uint2(floor(neighbourCoord));
                    // intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    // convolutionFactor = w6;
                    // colorSum = colorSum + intermediateColor * convolutionFactor;

                    // // 9
                    // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 9),
                    //                            half2(0),
                    //                            half2(31));
                    // floorNeighbour = uint2(floor(neighbourCoord));
                    // intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    // convolutionFactor = w5;
                    // colorSum = colorSum + intermediateColor * convolutionFactor;

                    // // 10
                    // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 10),
                    //                            half2(0),
                    //                            half2(31));
                    // floorNeighbour = uint2(floor(neighbourCoord));
                    // intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    // convolutionFactor = w4;
                    // colorSum = colorSum + intermediateColor * convolutionFactor;

                    // // 11
                    // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 11),
                    //                            half2(0),
                    //                            half2(31));
                    // floorNeighbour = uint2(floor(neighbourCoord));
                    // intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    // convolutionFactor = w3;
                    // colorSum = colorSum + intermediateColor * convolutionFactor;

                    // // 12
                    // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 12),
                    //                            half2(0),
                    //                            half2(31));
                    // floorNeighbour = uint2(floor(neighbourCoord));
                    // intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    // convolutionFactor = w2;
                    // colorSum = colorSum + intermediateColor * convolutionFactor;

                    // // 13
                    // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 13),
                    //                            half2(0),
                    //                            half2(31));
                    // floorNeighbour = uint2(floor(neighbourCoord));
                    // intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    // convolutionFactor = w1;
                    // colorSum = colorSum + intermediateColor * convolutionFactor;

                    // // 14
                    // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, 14),
                    //                            half2(0),
                    //                            half2(31));
                    // floorNeighbour = uint2(floor(neighbourCoord));
                    // intermediateColor = intermediate[floorNeighbour.x + wgDim * floorNeighbour.y];
                    // convolutionFactor = w0;
                    // colorSum = colorSum + intermediateColor * convolutionFactor;


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
                                                             {"shortcutDataCount", SkSLType::kInt},

                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},

                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},

                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},

                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},

                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},

                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},
                                                             {"weights", SkSLType::kFloat},

                                                             {"weights", SkSLType::kFloat}};
                mgr->setExpectedUniforms(uniforms);) mgr->write(float(1.0 / fWidth));
        mgr->write(int(fWidth - 1));
        mgr->write(int(fWgDim));
        mgr->write(int(fKernelDim));
        mgr->write(int(fHardwareConvolutionWeights.size() - 1));

        float weights[] = {
                0.000949682, 0.00176521, 0.00313381, 0.00531382, 0.00860595, 0.0133122, 0.0196678,
                0.0277538,   0.0374064,  0.0481534,  0.0592061,  0.0695287,  0.0779864, 0.0835473,
                0.0854876,   0.00271489, 0.00844763, 0.0219181,  0.0474216,  0.0855598, 0.128735,
                0.161534,    0.0854876,  13.3498,    11.371,     9.39264,    7.41474,   5.4372,
                3.45991,     1.48279,    0};

        for(int i=0; i<31; i++){
            mgr->write(weights[i]);
        }
    }

    // size_t calculateBufferSize(const skgpu::graphite::DrawParams&,
    //                            int resourceIndex,
    //                            const ResourceDesc& r) const override {
    //     // SkASSERT(resourceIndex == 3);
    //     SkASSERT(r.fFlow == DataFlow::kPrivate);
    //     return sizeof(float) * (fKernelDim + 1 + fHardwareConvolutionWeights.size() * 2);
    // }

    // void prepareStorageBuffer(const skgpu::graphite::DrawParams&,
    //                           int ssboIndex,
    //                           int resourceIndex,
    //                           const ResourceDesc& r,
    //                           void* buffer,
    //                           size_t bufferSize) const override {
    //     // SkASSERT(resourceIndex == 3);
    //     SkASSERT(r.fFlow == DataFlow::kPrivate);

    //     memcpy(buffer, fmergedWeights.data(), bufferSize);

    //     int boxWidth = fKernelDim + 1;
    //     size_t dataCount = sizeof(float) * (boxWidth + fHardwareConvolutionWeights.size() * 2);
    //     SkASSERT(bufferSize == dataCount);
    //     SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
    //     // for (auto e : fmergedWeights) {
    //     //     cout << e << ", ";
    //     // }
    //     // cout << endl;
    //     // cout << fmergedWeights.size() << endl;
    // }

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
                half4 srcColor;
                half2 neighbourCoord;
                // float3 colorSum = float3(0);

                float3 colorSum1 = float3(0);
                float3 colorSum2 = float3(0);

                float2 unormCoord;
                float convolutionFactor;


                uint wgId = sk_WorkgroupID.y;
                // uint offset = uint(-blurRadius + int(wgId) * (int(wgDim) - 2 * blurRadius));
                // uint2 pixelLoc = uint2(sk_GlobalInvocationID.x, sk_LocalInvocationID.y + offset);

                uint offset = uint(-blurRadius + int(wgId) * (2 * int(wgDim) - 2 * blurRadius));
                uint2 pixelLoc1 = uint2(sk_GlobalInvocationID.x, 2 * sk_LocalInvocationID.y + offset);
                uint2 pixelLoc2 = uint2(sk_GlobalInvocationID.x, 2 * sk_LocalInvocationID.y + 1 + offset);

                float tmp = 1;
                int weightsOffset = blurRadius + 1;
                int positionOffset = blurRadius + 1 + shortcutDataCount + 1;

                for (int i = -shortcutDataCount; i <= shortcutDataCount; ++i){
                    if(i<0){tmp = -1;}
                    else{tmp = 1;}

                    convolutionFactor = kernel_weights[uint(weightsOffset + shortcutDataCount - abs(i))];

                    neighbourCoord = clamp(half2(pixelLoc1) + half2(tmp * kernel_weights[positionOffset + shortcutDataCount - abs(i)], 0), half2(0), half2(srcDim));
                    unormCoord = float2(neighbourCoord) * srcDimInv;
                    srcColor = sampleLod(src, unormCoord, 0);
                    colorSum1 = colorSum1 + srcColor.rgb * convolutionFactor;

                    neighbourCoord = clamp(half2(pixelLoc2) + half2(tmp * kernel_weights[positionOffset + shortcutDataCount - abs(i)], 0), half2(0), half2(srcDim));
                    unormCoord = float2(neighbourCoord) * srcDimInv;
                    srcColor = sampleLod(src, unormCoord, 0);
                    colorSum2 = colorSum2 + srcColor.rgb * convolutionFactor;
                }

                intermediate[sk_LocalInvocationID.x * 2*wgDim + 2 * sk_LocalInvocationID.y] = colorSum1;
                intermediate[sk_LocalInvocationID.x * 2*wgDim + 2 * sk_LocalInvocationID.y + 1] = colorSum2;

                workgroupBarrier();

                if (sk_LocalInvocationID.y <= (wgDim - 1 - uint(blurRadius))){

                    // colorSum = float3(0);

                    pixelLoc1 = pixelLoc1 + uint2(0,blurRadius);
                    pixelLoc2 = pixelLoc2 + uint2(0,blurRadius);

                    colorSum1 = float3(0);
                    colorSum2 = float3(0);
                    float3 intermediateColor;
                    uint2 floorNeighbour;


                    for (int i = -blurRadius; i <= blurRadius; ++i){
                        convolutionFactor = kernel_weights[uint(blurRadius - abs(i))];


                        neighbourCoord = clamp(half2(sk_LocalInvocationID.x, 2 * sk_LocalInvocationID.y + uint(blurRadius)) + half2(0, i),
                                               half2(0),
                                               half2(2*wgDim-1));
                        floorNeighbour = uint2(floor(neighbourCoord));
                        intermediateColor = intermediate[floorNeighbour.x * 2 * wgDim + floorNeighbour.y];
                        colorSum1 = colorSum1 + intermediateColor * convolutionFactor;


                        neighbourCoord = clamp(half2(sk_LocalInvocationID.x, 2 * sk_LocalInvocationID.y + 1 + uint(blurRadius)) + half2(0, i),
                                               half2(0),
                                               half2(2*wgDim-1));
                        floorNeighbour = uint2(floor(neighbourCoord));
                        intermediateColor = intermediate[floorNeighbour.x * 2 * wgDim + floorNeighbour.y];
                        colorSum2 = colorSum2 + intermediateColor * convolutionFactor;


                        // neighbourCoord = clamp(half2(sk_LocalInvocationID.xy) + half2(0, i),
                        //                        half2(0),
                        //                        half2(wgDim-1));
                    }

                    textureWrite(dest, pixelLoc1, half4(colorSum1.rgb1));
                    textureWrite(dest, pixelLoc2, half4(colorSum2.rgb1));
                }
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
        float hSize = ceilf(float(fHeight) / float(2.0 * specialDim - 2 * fKernelDim));
        return WorkgroupSize(ceil(fWidth / float(specialDim)), hSize, 1);
    }
};

class DualBlurFilter : public ComputeImageFilter {
public:
    DualBlurFilter(uint32_t srcWidth, uint32_t srcHeight, uint32_t wgDim)
            : ComputeImageFilter(
                      /*width=*/srcWidth,
                      /*height=*/srcHeight,
                      /*name=*/"DualBlurFilter_Compute",
                      /*localDispatchSize=*/{wgDim, wgDim, 1},
                      /*resources=*/
                      {
                              {
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
                              //    {
                              //            /*type=*/ResourceType::kStorageBuffer,
                              //            /*flow=*/DataFlow::kPrivate,
                              //            /*policy=*/ResourcePolicy::kMapped,
                              //            /*slot=*/2,
                              //    }
                      }) {}
    ~DualBlurFilter() override = default;

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
            // layout(set = 0, binding = 1) readonly buffer inputBlock
            // {
            //     float kernel_weights[];
            // };

            workgroup float3 intermediate[1024];

            void main() {
                float3 colorSum = float3(0);
                float2 unormCoord;
                uint2 globalLoc;

                int padding = 2;
                uint wgIdX = sk_WorkgroupID.x;
                uint wgIdY = sk_WorkgroupID.y;
                uint offsetX = uint(-padding + int(wgIdX) * (int(wgDim) - 2 * padding));
                uint offsetY = uint(-padding + int(wgIdY) * (int(wgDim) - 2 * padding));
                uint2 pixelLoc = uint2(sk_LocalInvocationID.x + offsetX, sk_LocalInvocationID.y + offsetY);

                uint threadRange = uint(ceil(float(wgDim) * 0.5));
                if(sk_LocalInvocationID.x < threadRange &&
                   sk_LocalInvocationID.y < threadRange){

                    uint2 globalLoc = pixelLoc + sk_LocalInvocationID.xy;

                    // uint2 globalLoc = uint2(sk_GlobalInvocationID.x + sk_LocalInvocationID.x, sk_GlobalInvocationID.y + sk_LocalInvocationID.y);
                    uint2 pixelLoc2 = globalLoc + uint2(0,1);
                    uint2 pixelLoc3 = globalLoc + uint2(1,0);
                    uint2 pixelLoc4 = globalLoc + uint2(1,1);

                    float2 centralLoc = float2(globalLoc) + float2(0.5, 0.5);

                    unormCoord = clamp((centralLoc * srcDimInv), float2(0), float2(1));
                    colorSum = sampleLod(src, unormCoord, 0).rgb * 4;

                    unormCoord = clamp(((centralLoc + float2(1,1)) * srcDimInv), float2(0), float2(1));
                    colorSum += sampleLod(src, unormCoord, 0).rgb;

                    unormCoord = clamp(((centralLoc + float2(-1,1)) * srcDimInv), float2(0), float2(1));
                    colorSum += sampleLod(src, unormCoord, 0).rgb;

                    unormCoord = clamp(((centralLoc + float2(1,-1)) * srcDimInv), float2(0), float2(1));
                    colorSum += sampleLod(src, unormCoord, 0).rgb;

                    unormCoord = clamp(((centralLoc + float2(-1,-1)) * srcDimInv), float2(0), float2(1));
                    colorSum += sampleLod(src, unormCoord, 0).rgb;

                    colorSum = colorSum * 0.125;

                    textureWrite(dest, globalLoc, half4(colorSum.rgb1));
                    textureWrite(dest, pixelLoc2, half4(colorSum.rgb1));
                    textureWrite(dest, pixelLoc3, half4(colorSum.rgb1));
                    textureWrite(dest, pixelLoc4, half4(colorSum.rgb1));

                    uint2 localLoc = uint2(2*sk_LocalInvocationID.x, 2*sk_LocalInvocationID.y);

                    if(localLoc.x < 32 && localLoc.y < 32){
                        intermediate[localLoc.x * wgDim + localLoc.y] = colorSum;
                    }
                    if(localLoc.x < 31 && localLoc.y < 32){
                        intermediate[(1 + localLoc.x) * wgDim + localLoc.y] = colorSum;
                    }
                    if(localLoc.x < 32 && localLoc.y < 31){
                        intermediate[localLoc.x * wgDim + (1 + localLoc.y)] = colorSum;
                    }
                    if(localLoc.x < 31 && localLoc.y < 31){
                        intermediate[(1 + localLoc.x) * wgDim + (1 + localLoc.y)] = colorSum;
                    }


                    // workgroupBarrier();
                    // textureWrite(dest, globalLoc, half4(intermediate[localLoc.x * wgDim + localLoc.y].rgb1));
                    // textureWrite(dest, pixelLoc2, half4(intermediate[(1 + localLoc.x) * wgDim + localLoc.y].rgb1));
                    // textureWrite(dest, pixelLoc3, half4(intermediate[localLoc.x * wgDim + (1 + localLoc.y)].rgb1));
                    // textureWrite(dest, pixelLoc4, half4(intermediate[(1 + localLoc.x) * wgDim + (1 + localLoc.y)].rgb1));
                }

                workgroupBarrier();


                if (uint(padding) <= sk_LocalInvocationID.y &&
                    sk_LocalInvocationID.y <= uint(31 - padding) &&
                    uint(padding) <= sk_LocalInvocationID.x &&
                    sk_LocalInvocationID.x <= uint(31 - padding)){

                    colorSum = float3(0,0,0);
                    // globalLoc = sk_GlobalInvocationID.xy;
                    globalLoc = pixelLoc;
                    int2 localLoc = int2(sk_LocalInvocationID.xy);
                    uint2 sampleCoord;

                    sampleCoord = uint2(clamp(localLoc + int2(2,0) , int2(0), int2(31)));
                    colorSum += intermediate[sampleCoord.x * wgDim + sampleCoord.y].rgb;

                    sampleCoord = uint2(clamp(localLoc + int2(-2,0) , int2(0), int2(31)));
                    colorSum += intermediate[sampleCoord.x * wgDim + sampleCoord.y].rgb;

                    sampleCoord = uint2(clamp(localLoc + int2(0,2) , int2(0), int2(31)));
                    colorSum += intermediate[sampleCoord.x * wgDim + sampleCoord.y].rgb;

                    sampleCoord = uint2(clamp(localLoc + int2(0,-2) , int2(0), int2(31)));
                    colorSum += intermediate[sampleCoord.x * wgDim + sampleCoord.y].rgb;

                    sampleCoord = uint2(clamp(localLoc + int2(1,1) , int2(0), int2(31)));
                    colorSum += intermediate[sampleCoord.x * wgDim + sampleCoord.y].rgb * 2;

                    sampleCoord = uint2(clamp(localLoc + int2(-1,1) , int2(0), int2(31)));
                    colorSum += intermediate[sampleCoord.x * wgDim + sampleCoord.y].rgb * 2;

                    sampleCoord = uint2(clamp(localLoc + int2(1,-1) , int2(0), int2(31)));
                    colorSum += intermediate[sampleCoord.x * wgDim + sampleCoord.y].rgb * 2;

                    sampleCoord = uint2(clamp(localLoc + int2(-1,-1) , int2(0), int2(31)));
                    colorSum += intermediate[sampleCoord.x * wgDim + sampleCoord.y].rgb * 2;


                    colorSum = colorSum * 0.0833333333;
                    // textureWrite(dest, globalLoc, half4(intermediate[localLoc.x * wgDim + localLoc.y].rgb1));
                    textureWrite(dest, globalLoc, half4(colorSum.rgb1));
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

    WorkgroupSize calculateGlobalDispatchSize(const DrawParams&) const override {
        int padding = 2;
        float hSize = ceilf(float(fHeight) / float(fWgDim - 2 * padding));
        float wSize = ceilf(float(fHeight) / float(fWgDim - 2 * padding));
        return WorkgroupSize(wSize, hSize, 1);
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
            , fDualBlur(fImage->width(), fImage->height(), wgDim)
            , fExtendRadiusGaussian(fImage->width(), fImage->height(), wgDim)
            , fHackyGaussian(fImage->width(), fImage->height(), wgDim) {
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

    void updateAllKernelDim() {
        fBoxBlur.setKernelDim(fKernelDim);
        fALUGaussian.setKernelDim(fKernelDim);
        fMemGaussian.setKernelDim(fKernelDim);
        fSeparableGaussian.setKernelDim(fKernelDim);
        fHardwareHorizontalGaussian.setKernelDim(fKernelDim);
        fDualBlur.setKernelDim(fKernelDim);
        fExtendRadiusGaussian.setKernelDim(fKernelDim);
        fHackyGaussian.setKernelDim(fKernelDim);
    }

    void updateAllSigma() {
        fALUGaussian.setSigma(fSigma);
        fMemGaussian.setSigma(fSigma);
        fSeparableGaussian.setSigma(fSigma);
        fHardwareHorizontalGaussian.setSigma(fSigma);
        fDualBlur.setSigma(fSigma);
        fExtendRadiusGaussian.setSigma(fSigma);
        fHackyGaussian.setSigma(fSigma);
    }

    void updateAllGaussianKernel() {
        if (fIsDisplaySlowMemGaussian) fMemGaussian.updateGaussianKernel();
        if (fIsDisplaySeparableGaussian) fSeparableGaussian.updateSeparableGaussianKernel();
        if (fIsDisplayHardwareHorizontalGaussian)
            fHardwareHorizontalGaussian.updateHardwareGaussianKernel();
        if (fIsDisplayHardwareHorizontalGaussian)
            fHardwareHorizontalGaussian.updateSeparableGaussianKernel();
        if (fIsDisplayDualBlur) fDualBlur.updateSeparableGaussianKernel();
        if (fIsDisplayDualBlur) fDualBlur.updateHardwareGaussianKernel();
        if (fIsDisplayExtendRadiusGaussian) fExtendRadiusGaussian.updateSeparableGaussianKernel();
        if (fIsDisplayExtendRadiusGaussian) fExtendRadiusGaussian.updateHardwareGaussianKernel();
        if (fIsDisplayHackyBlur) fHackyGaussian.updateSeparableGaussianKernel();
        if (fIsDisplayHackyBlur) fHackyGaussian.updateHardwareGaussianKernel();
    }

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
        SkFont font(nullptr, 16);
        SkPaint captionPaint;
        captionPaint.setColor(SK_ColorBLACK);

        uint32_t dim = fImage->width();  // assuming image is square
        uint32_t margin = 10;

        // // First Row
        vector<SkScalar> marginLeft;
        for (int i = 0; i < 4; i++) marginLeft.push_back(margin + i * (dim + margin));
        SkScalar textMargin = 5 + margin + dim;
        SkScalar marginTop = margin;
        const char* captions1[] = {"Source", "Grayscale", "Slow box blur"};

        canvas->drawImage(fImage, marginLeft[0], marginTop, SkSamplingOptions(), &paint);
        if (fIsDisplayGrayscale)
            device->drawComputeFilteredImage(fImage, marginLeft[1], marginTop, &fGrayscale);
        if (fIsDisplaySlowBox)
            device->drawComputeFilteredImage(fImage, marginLeft[2], marginTop, &fBoxBlur);

        for (unsigned int i = 0; i < std::size(captions1); i++) {
            canvas->drawString(
                    captions1[i], marginLeft[i], marginTop + textMargin, font, captionPaint);
        }

        // Second Row
        marginTop = (4 * margin) + dim;
        if (fIsDisplaySeparableGaussian)
            device->drawComputeFilteredImage(fImage, marginLeft[0], marginTop, &fSeparableGaussian);
        if (fIsDisplaySlowALUGaussian)
            device->drawComputeFilteredImage(fImage, marginLeft[1], marginTop, &fALUGaussian);
        if (fIsDisplaySlowMemGaussian)
            device->drawComputeFilteredImage(fImage, marginLeft[2], marginTop, &fMemGaussian);

        const char* captions2[] = {
                "Seperable gaussian", "Slow ALU gaussian blur", "Slow mem gaussian blur"};
        for (unsigned int i = 0; i < std::size(captions2); i++) {
            canvas->drawString(
                    captions2[i], marginLeft[i], marginTop + textMargin, font, captionPaint);
        }

        // Third Row
        marginTop = (8 * margin) + 2 * dim;
        if (fIsDisplayHardwareHorizontalGaussian)
            device->drawComputeFilteredImage(
                    fImage, marginLeft[0], marginTop, &fHardwareHorizontalGaussian);
        if (fIsDisplayExtendRadiusGaussian)
            device->drawComputeFilteredImage(
                    fImage, marginLeft[1], marginTop, &fExtendRadiusGaussian);
        if (fIsDisplayDualBlur)
            device->drawComputeFilteredImage(fImage, marginLeft[2], marginTop, &fDualBlur);

        const char* captions3[] = {
                "Hardware Horizontal gaussian", "Extend Radius gaussian", "Dual blur"};
        for (unsigned int i = 0; i < std::size(captions3); i++) {
            canvas->drawString(
                    captions3[i], marginLeft[i], marginTop + textMargin, font, captionPaint);
        }

        // Forth Row
        marginTop = (12 * margin) + 3 * dim;
        if (fIsDisplayHackyBlur)
            device->drawComputeFilteredImage(fImage, marginLeft[0], marginTop, &fHackyGaussian);

        const char* captions4[] = {"Hacky gaussian"};
        for (unsigned int i = 0; i < std::size(captions4); i++) {
            canvas->drawString(
                    captions4[i], marginLeft[i], marginTop + textMargin, font, captionPaint);
        }

        ImGui::Begin("Blur slider", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Checkbox("Greyscale", &fIsDisplayGrayscale);
        ImGui::Checkbox("Slow Box Blur", &fIsDisplaySlowBox);
        ImGui::Checkbox("Slow ALU Gaussian Blur", &fIsDisplaySlowALUGaussian);
        ImGui::Checkbox("Slow Mem Gaussian Blur", &fIsDisplaySlowMemGaussian);
        ImGui::Checkbox("Separable Gaussian Blur", &fIsDisplaySeparableGaussian);
        ImGui::Checkbox("Hardware Horizontal Gaussian Blur", &fIsDisplayHardwareHorizontalGaussian);
        ImGui::Checkbox("Extend Radius Gaussian Blur", &fIsDisplayExtendRadiusGaussian);
        ImGui::Checkbox("Dual Blur", &fIsDisplayDualBlur);
        ImGui::Checkbox("Hacky Blur", &fIsDisplayHackyBlur);

        float blurBox = fKernelDim;
        float maxRadius = fIsDisplaySeparableGaussian || fIsDisplayHardwareHorizontalGaussian ||
                                          fIsDisplayExtendRadiusGaussian
                                  ? 15
                                  : 64;
        if (ImGui::SliderFloat("Radius", &blurBox, 1, 15)) {
            fKernelDim = int(blurBox);
            fSigma = fmax(fKernelDim * 0.333333333, 1);

            updateAllKernelDim();
            updateAllSigma();
            updateAllGaussianKernel();
        }

        float sigma = fSigma;
        if (ImGui::SliderFloat("Sigma", &sigma, 1, std::max(fKernelDim / 3 * 2, 2))) {
            fSigma = sigma;
            updateAllSigma();
            updateAllGaussianKernel();
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
    DualBlurFilter fDualBlur;
    ExtendRadiusGaussianBlurFilter fExtendRadiusGaussian;
    HackyGaussianFilter fHackyGaussian;

    bool fIsDisplayGrayscale = 0;
    bool fIsDisplaySlowBox = 0;
    bool fIsDisplaySlowALUGaussian = 0;
    bool fIsDisplaySlowMemGaussian = 0;
    bool fIsDisplaySeparableGaussian = 0;
    bool fIsDisplayHardwareHorizontalGaussian = 0;
    bool fIsDisplayExtendRadiusGaussian = 0;
    bool fIsDisplayDualBlur = 0;
    bool fIsDisplayHackyBlur = 1;

    int fKernelDim = 1;
    float fSigma = 1;
};

}  // namespace

DEF_SLIDE(std::string imageName = "mandrill_256.png"; std::string imagePath = "images/" + imageName;
          return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName);)

DEF_SLIDE(std::string imageName = "mandrill_512.png"; std::string imagePath = "images/" + imageName;
          return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName);)

DEF_SLIDE(std::string imageName = "mandrill_1600.png";
          std::string imagePath = "images/" + imageName;
          return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName);)

// DEF_SLIDE(std::string imageName = "puppy_999.png"; std::string imagePath = "images/" + imageName;
//           return new ComputeImageFiltersSlide(GetResourceAsImage(imagePath.c_str()), imageName);)

#endif  // defined(SK_GRAPHITE)
#endif  // SK_ENABLE_COMPUTE_IMAGE_FILTER_EXPERIMENTS
