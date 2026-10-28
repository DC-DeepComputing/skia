---
name: skia/gpu/compare-renderstep-id-for-vertex-input-change
description: When checking whether vertex input layout changed between consecutive graphics pipelines in Graphite, compare RenderStep IDs rather than deep-comparing attribute arrays
rubric: safety
severity: should_fix
---

**Rule.** When checking whether vertex input layout has changed between
consecutive graphics pipelines, compare RenderStepID rather than performing deep
comparisons across vertex and instance attribute arrays. If pipelines can also
bind dynamic paint parameter attributes, combine the RenderStepID inequality
check with a check for active paint parameter attributes.

**Why.** In Graphite, vertex and instance attribute configurations are
statically declared per RenderStep and remain invariant for that step's
lifetime. Comparing integer RenderStepID values provides an O(1) check that
avoids redundant iteration and element-wise struct comparison across attribute
descriptors during pipeline transitions.

## Examples

Bad:

```cpp
const bool vertexInputDirty =
        previous == nullptr ||
        !is_same_vertex_input_state(fCachedVertexAttribs,
                                    fCachedInstanceAttribs,
                                    previous->fCachedVertexAttribs,
                                    previous->fCachedInstanceAttribs);
```

Good:

```cpp
const bool vertexInputDirty =
        previous == nullptr ||
        previous->fRenderStepID != fRenderStepID ||
        fHasPaintParamAttributes || previous->fHasPaintParamAttributes;
```
