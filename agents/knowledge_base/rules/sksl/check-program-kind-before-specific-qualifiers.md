---
name: skia/sksl/check-program-kind-before-specific-qualifiers
description: When validating SkSL AST constructs or declarations, check program kind support for the construct before validating specific qualifiers or attributes
rubric: correctness
severity: should_fix
---

**Rule.** In SkSL IR conversion and validation (such as ModifiersDeclaration or
InterfaceBlock), verify that a construct or declaration is permitted for the
active program kind before validating specific qualifiers or attributes. Reject
unsupported constructs with top-level diagnostics upfront rather than reporting
specialized qualifier errors.

**Why.** If a language construct is forbidden entirely in certain program kinds
(such as runtime effects), validating specific sub-qualifiers first produces
misleading error messages implying the construct is permitted if the specific
qualifier is omitted or changed.

## Examples

Bad:

```cpp
// Local size qualifier checked before verifying whether layout qualifiers are allowed
if ((modifiers.fLayout.fLocalSizeX >= 0 || modifiers.fLayout.fLocalSizeY >= 0 ||
     modifiers.fLayout.fLocalSizeZ >= 0) &&
    !ProgramConfig::IsCompute(kind)) {
    context.fErrors->error(
            modifiers.fPosition,
            "'local size' layout qualifiers are only allowed in a compute program");
    return nullptr;
}
if (!ProgramConfig::IsFragment(kind) && !ProgramConfig::IsVertex(kind) &&
    !ProgramConfig::IsCompute(kind)) {
    context.fErrors->error(modifiers.fPosition,
                           "layout qualifiers are not allowed in this kind of program");
    return nullptr;
}
```

Good:

```cpp
// Check whether layout declarations are supported in this program kind first
if (!ProgramConfig::IsFragment(kind) && !ProgramConfig::IsVertex(kind) &&
    !ProgramConfig::IsCompute(kind)) {
    context.fErrors->error(modifiers.fPosition,
                           "layout qualifiers are not allowed in this kind of program");
    return nullptr;
}

if (modifiers.fLayout.fLocalSizeX >= 0 || modifiers.fLayout.fLocalSizeY >= 0 ||
    modifiers.fLayout.fLocalSizeZ >= 0) {
    if (!ProgramConfig::IsCompute(kind)) {
        context.fErrors->error(
                modifiers.fPosition,
                "'local size' layout qualifiers are only allowed in a compute program");
        return nullptr;
    }
}
```
