---
name: skia/gpu/hoist-shared-shader-variable-declarations
description: When generating shader stage functions, hoist declarations of shared variables like SSBO indices to stage initialization instead of defining them conditionally across emission blocks
rubric: design
severity: should_fix
---

**Rule.** In shader code generators, hoist the declaration and initialization of
shared local variables (such as SSBO or uniform buffer indices) to the beginning
of the shader stage rather than lazily defining them across multiple conditional
emission blocks. Ensure downstream snippet generators and varying handlers can
unconditionally reference the shared identifier without tracking emission state
flags.

**Why.** Lazily defining shared variables across separate conditional blocks
forces the generator to maintain tricky boolean flags (like isDefined) and
duplicate emission strings across alternative branches. Initializing shared
variables once at stage entry simplifies code emission logic and prevents
reference errors in downstream snippet generators.

## Examples

Bad:

```cpp
bool uniformSsboIndexDefined = false;
if (sharedData.useUniformStorageBufferVS && sharedData.hasStepUniforms) {
    SkSL::String::appendf(&mainBody, "uint %s = %s;\n", this->uniformSsboIndex(),
                          RenderStep::ssboIndicesAttribute());
    mainBody += emit_uniforms_from_storage_buffer(this->uniformSsboIndex(), step->uniforms());
}
...
if (sharedData.useUniformStorageBufferVS && sharedData.hasLiftedPaintUniforms &&
    !sharedData.hasStepUniforms) {
    SkSL::String::appendf(&mainBody, "uint %s = %s;\n", this->uniformSsboIndex(),
                          RenderStep::ssboIndicesAttribute());
    uniformSsboIndexDefined = true;
}
```

Good:

```cpp
if (sharedData.useUniformStorageBufferVS) {
    SkSL::String::appendf(&mainBody, "uint %s = %s;\n", this->uniformSsboIndex(),
                          RenderStep::ssboIndicesAttribute());
    if (sharedData.hasStepUniforms) {
        mainBody += emit_uniforms_from_storage_buffer(this->uniformSsboIndex(), step->uniforms());
    }
}
...
if (sharedData.hasLiftedPaintUniforms) {
    // Downstream expressions can reference this->uniformSsboIndex() directly.
}
```
