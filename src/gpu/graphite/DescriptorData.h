/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DescriptorTypes_DEFINED
#define skgpu_graphite_DescriptorTypes_DEFINED

#include "include/private/base/SkTArray.h"
#include "src/base/SkEnumBitMask.h"
#include "src/gpu/graphite/Sampler.h"

namespace skgpu::graphite {

/**
 * Types of descriptors supported within graphite
*/
enum class DescriptorType : uint8_t {
    kInvalid = 0,
    kUniformBuffer,
    kTextureSampler,
    kTexture,
    kCombinedTextureSampler,
    kStorageBuffer,
    kInputAttachment,

    kLast = kInputAttachment,
};
static constexpr int kDescriptorTypeCount = (int)(DescriptorType::kLast) + 1;

enum class PipelineStageFlags : uint8_t {
    kInvalid = 0,
    kVertexShader = 0b001,
    kFragmentShader = 0b010,
    kCompute = 0b100,
};
SK_MAKE_BITMASK_OPS(PipelineStageFlags);

struct DescriptorData {
    DescriptorData(DescriptorType type,
                   uint32_t count,
                   int bindingIdx,
                   SkEnumBitMask<PipelineStageFlags> stageFlags,
                   const Sampler* immutableSampler = nullptr)
            : fType (type)
            , fCount (count)
            , fBindingIndex (bindingIdx)
            , fPipelineStageFlags(stageFlags)
            , fImmutableSampler(immutableSampler) {}

    // Default ctor that initializes attributes to invalid values
    DescriptorData()
            : fType()
            , fCount(0)
            , fBindingIndex(-1)
            , fPipelineStageFlags(PipelineStageFlags::kInvalid)
            , fImmutableSampler(nullptr) {}

    bool isValid() const {
        return (fType != DescriptorType::kInvalid &&
                fCount > 0 &&
                fBindingIndex >= 0 &&
                fPipelineStageFlags != PipelineStageFlags::kInvalid);
    }

    DescriptorType fType;
    uint32_t fCount;
    int fBindingIndex;
    SkEnumBitMask<PipelineStageFlags> fPipelineStageFlags;
    const Sampler* fImmutableSampler;
};

};  // namespace skgpu::graphite

#endif // skgpu_graphite_DescriptorTypes_DEFINED
