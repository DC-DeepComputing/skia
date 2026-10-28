---
name: skia/sksl/emit-varying-interpolation-on-stage-interfaces-not-color-out
description: In shader backend codegen, emit varying interpolation qualifiers on inter-stage vertex outputs and fragment inputs, never on fragment color outputs
rubric: correctness
severity: should_fix
---

**Rule.** When generating shader pipeline interface attributes or structs, emit
varying interpolation qualifiers (such as flat, noperspective, or
center_no_perspective) exclusively on vertex stage outputs and fragment stage
inputs. Never emit interpolation qualifiers on fragment stage color outputs or
render target attachments. Factor interpolation qualifier emission into a shared
helper used across stage interface generation to keep attribute syntax
consistent.

**Why.** Interpolation qualifiers control how varyings are sampled across
primitives between vertex and fragment stages, which is invalid on fragment
color attachments. Emitting interpolation qualifiers on color outputs generates
malformed shader code that fails driver compilation, while sharing qualifier
emission across stage boundaries prevents syntax mismatches between vertex
outputs and fragment inputs.

## Examples

Bad:

```cpp
// In writeOutputStruct():
} else if (ProgramConfig::IsFragment(fProgram.fConfig->fKind)) {
    this->write(" [[color(" + std::to_string(location) + ")");
    if (var.modifierFlags().isFlat()) {
        this->write(" flat");
    } else if (var.modifierFlags().isNoPerspective()) {
        this->write(" center_no_perspective");
    }
    this->write("]]");
}
```

Good:

```cpp
void MetalCodeGenerator::writeInterpolatedAttributes(const Variable& var) {
    SkASSERT((is_output(var) && ProgramConfig::IsVertex(fProgram.fConfig->fKind)) ||
             (is_input(var) && ProgramConfig::IsFragment(fProgram.fConfig->fKind)));
    this->write(" [[user(locn" + std::to_string(var.layout().fLocation) + ")");
    if (var.modifierFlags().isFlat()) {
        this->write(" flat");
    } else if (var.modifierFlags().isNoPerspective()) {
        this->write(" center_no_perspective");
    }
    this->write("]]");
}
// Called from writeOutputStruct() for vertex outputs and writeInputStruct() for fragment inputs.
```
