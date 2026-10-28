---
name: skia/sksl/keep-backend-codegen-data-out-of-ir-structures
description: When adding backend-specific analysis in SkSL, store state in code generators rather than Program, and execute analysis passes during generateCode rather than in constructors
rubric: design
severity: should_fix
---

**Rule.** Do not store backend-specific analysis results or transient codegen
data in target-agnostic IR structures such as SkSL::Program. Instead, store
state within the specific backend code generator (such as SPIRVCodeGenerator or
MetalCodeGenerator), keep constructors limited to field initialization, and run
AST analysis passes (such as Analysis::FindFunctionsToSpecialize) during
generateCode() or instruction emission.

**Why.** Target-agnostic IR structures like SkSL::Program are shared across all
backend targets; adding backend-specific data bloats shared structures and
wastes resources. Keeping CodeGenerator constructors trivial separates
instantiation from compilation, avoids premature AST traversals, and collocates
analysis passes with the code generation logic that consumes their output.

## Examples

Bad:

```cpp
// Storing backend-specific analysis in shared IR:
struct Program {
    Analysis::SpecializationInfo fSpecializationInfo;
};

// Or executing AST analysis passes inside the CodeGenerator constructor:
SPIRVCodeGenerator::SPIRVCodeGenerator(const Context* context,
                                       const ShaderCaps* caps,
                                       const Program* program,
                                       OutputStream* out)
        : CodeGenerator(context, caps, program, out) {
    Analysis::FindFunctionsToSpecialize(*program, isSampler, &fSpecializationInfo);
}
```

Good:

```cpp
// Store backend-specific analysis in the target code generator:
class SPIRVCodeGenerator : public CodeGenerator {
    Analysis::SpecializationInfo fSpecializationInfo;
};

// Keep constructor trivial and run analysis passes during code generation:
SPIRVCodeGenerator::SPIRVCodeGenerator(const Context* context,
                                       const ShaderCaps* caps,
                                       const Program* program,
                                       OutputStream* out)
        : CodeGenerator(context, caps, program, out) {}

void SPIRVCodeGenerator::writeInstructions(const Program& program, OutputStream& out) {
    Analysis::FindFunctionsToSpecialize(program, isSampler, &fSpecializationInfo);
    ...
}
```
