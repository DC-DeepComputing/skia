---
name: skia/sksl/no-codegen-asserts-on-frontend-valid-constructs
description: When handling SkSL AST or layout qualifiers, enforce validation in front-end error checking rather than asserting or defensively checking in codegen backends
rubric: design
severity: should_fix
---

**Rule.** Enforce SkSL language constraints and layout qualifier validation
(such as non-zero workgroup local sizes or variable declaration attributes) in
front-end IR conversion and error checking (such as ModifiersDeclaration or
VarDeclaration::ErrorCheck) via ErrorReporter. Do not rely on backend code
generators (WGSL, SPIR-V, Metal, GLSL) to validate qualifiers, and never assert
with SkASSERT on AST or layout constructs permitted by the front end; guard
against backend-specific capability constraints safely.

**Why.** SkSL has multiple backend code generators that consume the same
front-end IR. Validating layout qualifiers and language rules in the front end
ensures uniform compiler error diagnostics across all backends without redundant
checks. Asserting on constructs permitted by the front end causes fuzzers and
invalid shaders to crash debug builds instead of producing clean compiler
errors.

## Examples

Bad:

```cpp
// In WGSLCodeGenerator.cpp:
// 1. Asserting on unvalidated front-end layout qualifiers:
SkASSERT(fLocalSizeX > 0);
this->write("@compute @workgroup_size(" + std::to_string(fLocalSizeX));

// 2. Asserting on constructs permitted by the front end:
SkASSERT(layout.fIndex <= 0 ||
         (fProgram.fInterface.fOutputSecondaryColor && layout.fLocation == 0));
```

Good:

```cpp
// In SkSLModifiersDeclaration.cpp (front-end IR validation):
if (modifiers.fLayout.fLocalSizeX == 0 ||
    modifiers.fLayout.fLocalSizeY == 0 ||
    modifiers.fLayout.fLocalSizeZ == 0) {
    context.fErrors->error(modifiers.fPosition, "local size qualifiers cannot be zero");
    return nullptr;
}

// In WGSLCodeGenerator.cpp (guarding backend capabilities safely):
if (layout.fLocation == 0 && layout.fIndex >= 0 &&
    fContext.fCaps->fDualSourceBlendingSupport &&
    fProgram.fInterface.fOutputSecondaryColor) {
    this->write("@index(" + std::to_string(layout.fIndex) + ") ");
}
```
