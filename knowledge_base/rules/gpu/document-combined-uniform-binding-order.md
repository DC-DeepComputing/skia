---
name: skia/gpu/document-combined-uniform-binding-order
description: When emitting combined shader uniform or SSBO declarations that share a buffer binding across decoupled stages, document the required emission order matching data serialization
rubric: readability
severity: should_fix
---

**Rule.** When generating combined shader uniform blocks or SSBO declarations
that share a single buffer binding across multiple subsystems (such as paint and
RenderStep uniforms in Graphite), document the required declaration sequence in
an inline comment. Explain which subsystem's data is gathered first so the
shader layout strictly preserves the serialization order expected by
PipelineDataGatherer.

**Why.** Shader declaration generation in ShaderInfo and data gathering in
DrawList/PipelineDataGatherer are decoupled and lack compile-time schema
validation. Documenting the required field sequence prevents subtle
cross-subsystem layout desynchronization and corrupted uniform reads when shader
generators or gathering loops are updated.

## Examples

Bad:

```cpp
    std::string result = get_uniform_header(set, bufferID);
    for (const ShaderNode* n : nodes) {
        result += get_node_uniforms(&offsetter, n, numPaintUniforms, numUnliftedPaintUniforms, wrotePaintColor);
    }

    if (!stepUniforms.empty()) {
        result += get_uniforms(&offsetter, stepUniforms, -1, /*wrotePaintColor=*/nullptr);
    }
```

Good:

```cpp
    std::string result = get_uniform_header(set, bufferID);
    for (const ShaderNode* n : nodes) {
        result += get_node_uniforms(&offsetter, n, numPaintUniforms, numUnliftedPaintUniforms, wrotePaintColor);
    }

    // Paint and RenderStep uniforms share a binding. When RenderSteps are processed in DrawList,
    // the paint uniforms are always processed before the render step ones, so the emitted uniforms
    // must respect that ordering.
    if (!stepUniforms.empty()) {
        result += get_uniforms(&offsetter, stepUniforms, -1, /*wrotePaintColor=*/nullptr);
    }
```
