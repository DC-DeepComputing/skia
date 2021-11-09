/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphicsPipelineDesc_DEFINED
#define skgpu_GraphicsPipelineDesc_DEFINED

#include "include/core/SkTypes.h"

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "experimental/graphite/src/ContextUtils.h"
#include "include/core/SkSpan.h"
#include "include/private/SkOpts_spi.h"
#include "include/private/SkTArray.h"

namespace skgpu {

// TODO: Temporary enum describing the set of supported vertex shaders. Vertex shader uniforms
// are assumed to be bound to buffer 0, and fragment shader uniforms (for Combination) are bound
// to buffer 1.
enum class TestVertexShader : unsigned {
    // Draws a rectangle from (0,0) to (1,1), from 2 triangles.
    //  - no vertex attributes
    //  - no instance attributes
    //  - no vertex uniforms
    kUnitRectangle,
    // Draws a rectangle from (0,0) to (1,1), from 2 triangles, transformed by a uniform.
    //  - no vertex attributes
    //  - no instance attributes
    //  - [sx,sy,tx,ty] as a single float4 uniform
    kRectangle,
    // Draws a rectangle from (0,0) to (1,1), transformed by instance attributes.
    //  - no vertex attributes
    //  - [tx,ty,sx,sy] float4 instance attribute
    //  - no vertex uniforms
    kInstanceRectangle,
    // Draws transformed triangles, using vertex attributes to store initial position and a uniform
    // to store the transform.
    //  - float2 position vertex attribute
    //  - no instance attributes
    //  - [sx,sy,tx,ty] float4 uniform
    kVertexTris,
};

/**
 * GraphicsPipelineDesc represents the state needed to create a backend specific GraphicsPipeline,
 * minus the target-specific properties that can be inferred from the DrawPass and RenderPassTask.
 */
class GraphicsPipelineDesc {
public:
    GraphicsPipelineDesc();

    // Returns this as a uint32_t array to be used as a key in the pipeline cache.
    SkSpan<const uint32_t> asKey() const {
        return {fKey.data(), fKey.size()};
    }

    bool operator==(const GraphicsPipelineDesc& that) const {
        return this->fKey == that.fKey;
    }

    bool operator!=(const GraphicsPipelineDesc& other) const {
        return !(*this == other);
    }

    // Key describing the color shading tree of the pipeline's program
    Combination combo() const { return fCombination; }
    // Key describing the vertex shader and coverage of the pipeline
    TestVertexShader vertexShader() const { return fVertexShader; }

    void setProgram(TestVertexShader shader, Combination combo);

    /**
     * A common practice is to populate the the vertex/instance's memory using an implicit array of
     * structs. In this case, it is best to assert that:
     *     stride == sizeof(struct)
     */
    size_t vertexStride() const { return fVertexStride; }
    size_t instanceStride() const { return fInstanceStride; }

    struct Hash {
        uint32_t operator()(const GraphicsPipelineDesc& desc) const {
            return SkOpts::hash_fn(desc.fKey.data(), desc.fKey.size() * sizeof(uint32_t), 0);
        }
    };

private:
    // Estimate of max expected key size
    // TODO: flesh this out
    inline static constexpr int kPreAllocSize = 2;

    // TODO: Depending on the nature of the 3 program keys and other state, could the "key" of the
    // GraphicsPipelineDesc just be a char* view of its fields?
    SkSTArray<kPreAllocSize, uint32_t, true> fKey;

    // TODO: this is a bit underspecified. It still needs the rendering technique info, and fixed
    // function settings like stencil settings, depth test, write masks, etc.
    // Additionally, it still needs an entry point to generate the text of the program, but for now
    // the MSL can be inferred directly from the Combination enum and the vertex shader index.
    // Eventually, the program will be specified by a shading key, a geometry key, and a blending
    // key that compactly represent identity and can be queried to produce SkSL if a program must
    // be compiled. These three SkSL fragments are combined in a fixed way.
    Combination      fCombination;
    // TODO: store the rendering technique info from Chris here instead of a hard-coded VS index
    TestVertexShader fVertexShader;

    // Vertex and instance strides are defined by the attribute layout of the vertex shader, and as
    // such do not need to be included in the key. However, it's useful to include this info on the
    // pipeline desc so that attribute writing can be validated easily.
    size_t fVertexStride;
    size_t fInstanceStride;
};

} // namespace skgpu

// Map GraphicsPipelineDesc::Hash's return type to size_t for use with std::unordered_map hashes.
template<>
struct std::hash<skgpu::GraphicsPipelineDesc> {
    size_t operator()(const skgpu::GraphicsPipelineDesc& desc) const noexcept {
        return skgpu::GraphicsPipelineDesc::Hash()(desc);
    }
};

#endif // skgpu_GraphicsPipelineDesc_DEFINED
