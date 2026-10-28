---
name: skia/sksl/wgsl-emit-enables-in-write-enables
description: In SkSL WGSL code generation, emit language feature enables and extension directives inside writeEnables rather than inlining them in generateCode
rubric: design
severity: should_fix
---

**Rule.** In SkSL WGSL code generation, emit WGSL language enable, extension,
and diagnostic directives within WGSLCodeGenerator::writeEnables() rather than
inlining them directly in generateCode(). Keep shader preamble and extension
declarations consolidated in writeEnables() where the header output stream is
targeted.

**Why.** Inlining extension enable directives directly in generateCode()
scatters preamble statements across general code generation and bypasses the
dedicated header output stream. Consolidating language directives in
writeEnables() guarantees that all WGSL extensions and diagnostic directives are
emitted upfront in the shader header before pipeline declarations.

## Examples

Bad:

```cpp
bool WGSLCodeGenerator::generateCode() {
    this->preprocessProgram();

    if (fProgram.fInterface.fOutputSecondaryColor) {
        if (fContext.fCaps->fDualSourceBlendingSupport) {
            this->writeLine("enable chromium_internal_dual_source_blending;");
        } else {
            fContext.fErrors->error({}, "dual-src blending not supported");
        }
    }
```

Good:

```cpp
void WGSLCodeGenerator::writeEnables() {
    this->writeLine("diagnostic(off, derivative_uniformity);");
    if (fRequirements.fPixelLocalExtension) {
        this->writeLine("enable chromium_experimental_pixel_local;");
    }
    if (fProgram.fInterface.fOutputSecondaryColor) {
        if (fContext.fCaps->fDualSourceBlendingSupport) {
            this->writeLine("enable chromium_internal_dual_source_blending;");
        } else {
            fContext.fErrors->error({}, "dual-src blending not supported");
        }
    }
}
```
