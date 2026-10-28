---
name: skia/sksl/sksl-omit-builtin-decoration-for-synthetic-fragment-outputs
description: When emitting SPIR-V layout decorations in SkSL codegen, do not emit SpvDecorationBuiltIn for synthetic fragment colors like SK_FRAGCOLOR_BUILTIN and SK_SECONDARYFRAGCOLOR_BUILTIN
rubric: correctness
severity: should_fix
---

**Rule.** In SkSL SPIR-V code generation, do not emit SpvDecorationBuiltIn for
synthetic fragment color built-ins such as SK_FRAGCOLOR_BUILTIN or
SK_SECONDARYFRAGCOLOR_BUILTIN. Ensure fragment color outputs are identified and
decorated via SpvDecorationLocation and SpvDecorationIndex rather than SPIR-V
built-in decorations.

**Why.** SkSL assigns synthetic internal builtin IDs (>= 10000) to represent
fragment output colors and secondary outputs for dual-source blending. Emitting
SpvDecorationBuiltIn with these synthetic IDs produces invalid SPIR-V bytecode
that fails validation in drivers and Vulkan tests.

## Examples

Bad:

```cpp
if (layout.fBuiltin >= 0 && layout.fBuiltin != SK_FRAGCOLOR_BUILTIN) {
    this->writeInstruction(SpvOpDecorate, target, SpvDecorationBuiltIn, layout.fBuiltin,
                           fDecorationBuffer);
}
```

Good:

```cpp
if (layout.fBuiltin >= 0 && (layout.fBuiltin != SK_FRAGCOLOR_BUILTIN &&
                             layout.fBuiltin != SK_SECONDARYFRAGCOLOR_BUILTIN)) {
    this->writeInstruction(SpvOpDecorate, target, SpvDecorationBuiltIn, layout.fBuiltin,
                           fDecorationBuffer);
}
```
