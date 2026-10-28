---
name: skia/sksl/keep-sksl-codegen-focused-on-program-ir
description: In src/sksl, do not include GPU or utility dependencies; keep codegen APIs focused on SkSL::Program and delegate compilation orchestration to skgpu::SkSLToBackend
rubric: design
severity: should_fix
---

**Rule.** Do not introduce GPU or utility dependencies (such as
skgpu::ShaderErrorHandler or SkShaderUtils) into src/sksl/. Keep SkSL backend
code generator entry points (such as ToGLSL, ToMetal, ToSPIRV, ToWGSL, and
ToHLSL) focused strictly on converting a compiled SkSL::Program into target
backend source code, and delegate high-level pipeline parsing, compilation
orchestration, and error reporting to skgpu::SkSLToBackend in src/gpu/.

**Why.** SkSL is designed as a standalone, target-agnostic shading language
compiler that must build and operate independently of GPU backends (e.g. for CPU
pipelines and command-line tools like skslc). Introducing higher-level GPU error
handlers, utilities, or orchestration passes into src/sksl/ couples low-level IR
code generation to GPU pipeline abstractions and violates modular layering
boundaries.

## Examples

Bad:

```cpp
// In src/sksl/codegen/SkSLGLSLCodeGenerator.h:
bool ToGLSL(Compiler*,
            const std::string& sksl,
            ProgramKind programKind,
            const ProgramSettings& settings,
            std::string* glsl,
            Program::Interface*,
            skgpu::ShaderErrorHandler* errorHandler);

// In src/sksl/SkSLCompilerUtil.cpp:
#include "include/gpu/ShaderErrorHandler.h"
#include "src/utils/SkShaderUtils.h"

namespace SkSL {
bool ToBackend(Compiler* compiler, ..., skgpu::ShaderErrorHandler* errorHandler) {
    ...
}
}
```

Good:

```cpp
// In src/sksl/codegen/SkSLGLSLCodeGenerator.h:
bool ToGLSL(Program& program, const ShaderCaps* caps, OutputStream& out);
bool ToGLSL(Program& program, const ShaderCaps* caps, std::string* out);

// In src/gpu/SkSLToBackend.h:
#include "include/gpu/ShaderErrorHandler.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/utils/SkShaderUtils.h"

namespace skgpu {
bool SkSLToBackend(const SkSL::ShaderCaps* caps,
                   bool (*toBackend)(SkSL::Program&, const SkSL::ShaderCaps*, std::string*),
                   const char* backendLabel,
                   const std::string& sksl,
                   SkSL::ProgramKind programKind,
                   const SkSL::ProgramSettings& settings,
                   std::string* output,
                   SkSL::ProgramInterface* outInterface,
                   ShaderErrorHandler* errorHandler);
}
```
