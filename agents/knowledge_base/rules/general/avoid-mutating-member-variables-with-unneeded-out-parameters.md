---
name: skia/avoid-mutating-member-variables-with-unneeded-out-parameters
description: When invoking functions with unneeded out-parameters, declare dedicated local dummy variables instead of passing instance member variables that inadvertently mutate object state
rubric: readability
severity: should_fix
---

**Rule.** Do not pass instance member variables to receive out-parameters that
are not needed in the current calling context. When an API requires a mandatory
out-parameter that is unneeded, declare a dedicated local dummy variable (such
as unused<ParamName>) or pass nullptr if supported, rather than reusing member
fields.

**Why.** Reusing an existing instance member variable to satisfy an unneeded
out-parameter overwrites object state with unwanted side effects, creates
misleading data-flow coupling between separate execution phases, and can mask
logic bugs if execution order changes. Declaring a dedicated local variable
makes it explicit that the output is discarded and keeps object state clean.

## Examples

Bad:

```cpp
// Member variable fHasPaintUniforms is passed to receive an unneeded out-parameter:
sksl += emit_paint_params_uniforms(bindingReqs.fUniformsSetIdx,
                                   bindingReqs.fPaintParamsBufferBinding,
                                   bindingReqs.fUniformBufferLayout,
                                   fRootNodes,
                                   &fHasPaintUniforms,
                                   &unusedWrotePaintColor);
```

Good:

```cpp
// Dedicated local dummy variable absorbs the unneeded out-parameter:
bool unusedHasPaintUniforms = false;
bool unusedWrotePaintColor = false;
sksl += emit_paint_params_uniforms(bindingReqs.fUniformsSetIdx,
                                   bindingReqs.fPaintParamsBufferBinding,
                                   bindingReqs.fUniformBufferLayout,
                                   fRootNodes,
                                   &unusedHasPaintUniforms,
                                   &unusedWrotePaintColor);
```
