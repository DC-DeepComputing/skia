---
name: skia/sksl/pass-shadercaps-to-backend-codegen
description: When declaring backend shader code generation entry points, pass target ShaderCaps explicitly as a parameter rather than relying on Compiler or ambient state
rubric: correctness
severity: should_fix
---

**Rule.** When declaring or refactoring backend shader code generation functions
(such as ToGLSL, ToMetal, ToSPIRV, or ToWGSL), pass target shader capabilities
(const ShaderCaps* caps) as an explicit parameter. Do not rely on a Compiler
instance or ambient global state to supply target capabilities.

**Why.** Backend shader code generation depends on device capabilities,
extension pragmas, precision rules, and driver workarounds. Passing ShaderCaps
explicitly decouples code generators from Compiler instances, ensuring
standalone code generation has direct access to the required target-specific
capability flags.

## Examples

Bad:

```cpp
bool ToGLSL(Compiler* compiler, Program& program, OutputStream& out);
```

Good:

```cpp
bool ToGLSL(Program& program, const ShaderCaps* caps, OutputStream& out);
```
