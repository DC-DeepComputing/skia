/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputeStep_DEFINED
#define skgpu_graphite_ComputeStep_DEFINED

#include "include/core/SkSpan.h"
#include "src/core/SkEnumBitMask.h"

#include <string>
#include <string_view>
#include <vector>

namespace skgpu::graphite {

class DrawParams;
struct ResourceBindingRequirements;

/**
 * A `ComputeStep` represents a compute pass within a wider draw operation. A `ComputeStep`
 * implementation describes an invocation of a compute program and its data binding layout.
 *
 * A `ComputeStep` can perform arbitrary operations on the GPU over various types of data, including
 * geometry and image processing. The data processed by a `ComputeStep` can be inputs (textures or
 * buffers) populated on the CPU, data forwarded to and from other `ComputeStep` invocations (via
 * "slots"), transient storage buffers/textures that are only used within an individual dispatch,
 * geometry attribute (vertex/index/instance) and indirect draw parameters of a subsequent raster
 * pipeline stage, as well as texture outputs.
 *
 * The data flow between sequential `ComputeStep` invocations is described by associating input and
 * output resource "slots" that have matching slot indices across the instances. For example, a
 * buffer assigned to an output slot of a ComputeStep will be forwarded to the input slot with the
 * same slot index in a subsequent ComputeStep:
 *
 *      _______________                                        _______________
 *     |               |                                      |               |
 *     |                ---[Output Slot 0]---[Input Slot 0]---                |
 *     | ComputeStep 1 |                                      | ComputeStep 2 |
 *     |                ---[Output Slot 1]---[Input Slot 1]---                |
 *     |               |                                      |               |
 *      ---------------                                        ---------------
 *
 * Similarly, raster stage geometry buffers that are specified as the output of a ComputeStep can be
 * used to assign the draw buffers of a RenderStep.
 *
 * It is the responsibility of the owning entity (e.g. a RendererProvider) to ensure that a chain of
 * ComputeStep and RenderStep invocations have a compatible resource and data-flow layout.
 */
class ComputeStep {
public:
    enum class DataFlow {
        // A readable Buffer binding that contains CPU-encoded SkPath geometry.
        // If present, a `ComputeStep` must implement `writePathInput`.
        kPathInput,

        // A set of writable Buffer bindings that the `ComputeStep` will write vertex and instance
        // attributes to. If present, these buffers can be used to encode the draw command for a
        // subsequent `RenderStep`.
        kVertexOutput,
        kIndexOutput,
        kInstanceOutput,
        kIndirectOutput,

        // A read-write storage binding that the `ComputeStep` may use for its internal state.
        kTransient,

        // Bindings with a slot number that can be used to forward data between a series of
        // `ComputeStep`s. These DataFlow types are accompanied with a "slot number" that can be
        // used to associate the output slot of one `ComputeStep` with the input slot of a
        // subsequent `ComputeStep`.
        kInputSlot,
        kOutputSlot,
    };

    enum class ResourceType {
        kUniformBuffer,
        kStorageBuffer,

        // TODO(b/238794438): Support sampled and storage texture types.
    };

    struct Resource final {
        ResourceType fType;
        DataFlow fFlow;

        // If true, the resource must have its contents initialized to 0 before pipeline execution.
        bool fClearedBeforeUse = false;

        // This field only has meaning (and must have a non-negative value) if `fFlow` is one of
        // `DataFlow::kInputSlot` or `DataFlow::kOutputSlot`.
        int fSlot = -1;
    };

    virtual ~ComputeStep() = default;

    // Returns a complete SkSL compute program. The returned SkSL must declare all resoure bindings
    // starting at `nextBindingIndex` in the order in which they are enumerated by
    // `ComputeStep::resources()`.
    virtual std::string computeSkSL(const ResourceBindingRequirements&,
                                    int nextBindingIndex) const = 0;

    // Write path inputs to a storage buffer resource. The `DrawParams` parameter is expected to
    // contain a SkPath geometry. The supplied `buffer` parameter references the start address of a
    // mapped storage buffer corresponding to a `Resource` with the `DataFlow::kPathInput` flow
    // type.
    virtual void writePathInput(const DrawParams&, int ssboIndex, void* buffer);

    SkSpan<const Resource> resources() const { return SkSpan(fResources); }

    // Identifier that can be used as part of a unique key for a compute pipeline state object
    // associated with this `ComputeStep`.
    uint32_t uniqueID() const { return fUniqueID; }

    // Returns a debug name for the subclass implementation.
    const char* name() const { return fName.c_str(); }

    bool consumesPath() const { return fFlags & Flags::kConsumesPath; }
    bool outputsGeometry() const { return fFlags & Flags::kOutputsGeometry; }
    bool drivesIndirectDraw() const { return fFlags & Flags::kDrivesIndirectDraw; }

protected:
    ComputeStep(std::string_view name, SkSpan<const Resource> resources);

private:
    enum class Flags : uint8_t {
        kNone               = 0b000,
        kConsumesPath       = 0b001,
        kOutputsGeometry    = 0b010,
        kDrivesIndirectDraw = 0b100,
    };
    SK_DECL_BITMASK_OPS_FRIENDS(Flags);

    // Disallow copy and move
    ComputeStep(const ComputeStep&) = delete;
    ComputeStep(ComputeStep&&)      = delete;

    uint32_t fUniqueID;
    SkEnumBitMask<Flags> fFlags;
    std::string fName;
    std::vector<Resource> fResources;
};
SK_MAKE_BITMASK_OPS(ComputeStep::Flags);

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ComputeStep_DEFINED
