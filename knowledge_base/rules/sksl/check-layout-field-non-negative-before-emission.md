---
name: skia/sksl/check-layout-field-non-negative-before-emission
description: When emitting SkSL layout attributes or qualifiers in backend code generators, verify layout fields are non-negative to avoid emitting sentinel -1
rubric: correctness
severity: should_fix
---

**Rule.** When generating code for SkSL::Layout fields (such as fIndex,
fLocation, fBinding, fSet, or fOffset), verify that the field value is
non-negative (layout.f<Field> >= 0) before emitting the corresponding attribute
or qualifier. In SkSL IR, integer layout qualifiers default to -1 to represent
unspecified fields; emitting layout attributes without checking non-negativity
causes negative sentinel values to leak into backend shader text or bytecode
(e.g., @blend_src(-1)).

**Why.** SkSL::Layout uses -1 as an in-band sentinel for unset integer layout
attributes. Omitting non-negative guards causes backend code generators (WGSL,
SPIR-V, Metal, GLSL) to emit invalid layout decorators or malformed shader
attributes that fail downstream shader compilation, GPU pipeline creation, or
driver validation.

## Examples

Bad:

```cpp
// Bad: layout.fIndex is not verified to be non-negative, emitting @blend_src(-1) when unset.
if (layout.fLocation == 0 && fProgram.fInterface.fOutputSecondaryColor) {
    this->write("@blend_src(" + std::to_string(layout.fIndex) + ") ");
}
```

Good:

```cpp
// Good: verify layout.fIndex >= 0 before emitting the @blend_src attribute.
if (layout.fLocation == 0 && layout.fIndex >= 0 && fProgram.fInterface.fOutputSecondaryColor) {
    this->write("@blend_src(" + std::to_string(layout.fIndex) + ") ");
}
```
