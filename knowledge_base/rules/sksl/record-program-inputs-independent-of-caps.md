---
name: skia/sksl/record-program-inputs-independent-of-caps
description: When recording SkSL program inputs or interface flags during AST analysis, record shader usage unconditionally rather than gating on target ShaderCaps
rubric: design
severity: should_fix
---

**Rule.** Do not gate SkSL program input or ProgramInterface registration on
target GPU capabilities (ShaderCaps) during AST analysis and transformation
passes. Record what builtins or features the shader uses unconditionally in
ProgramInterface, and enforce target capability requirements during validation
or backend code generation.

**Why.** ProgramInterface flags reflect what the shader program itself uses,
independent of target hardware support. Gating input registration on ShaderCaps
falsely marks features as unused when capabilities are missing, preventing
backend code generators and validation passes from diagnosing unsupported
features and reporting actionable errors.

## Examples

Bad:

```cpp
case SK_LASTFRAGCOLOR_BUILTIN:
    if (context.fCaps->fFBFetchSupport) {
        program.fInterface.fUseLastFragColor = true;
    }
    break;
```

Good:

```cpp
case SK_LASTFRAGCOLOR_BUILTIN:
    program.fInterface.fUseLastFragColor = true;
    break;

// Later in backend code generation:
if (fProgram.fInterface.fUseLastFragColor) {
    if (fCaps.fFBFetchSupport) {
        this->write(", half4 " + std::string(fCaps.fFBFetchColorName) + " [[color(0)]]\n");
    } else {
        fContext.fErrors->error({}, "framebuffer fetch not supported");
    }
}
```
