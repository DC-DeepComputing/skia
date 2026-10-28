---
name: skia/sksl/bail-out-on-unsupported-ast-in-optimization-passes
description: In SkSL analysis and optimization passes, do not assert AST expression shapes with SkASSERT; check node kinds explicitly and bail out or skip unsupported expressions
rubric: correctness
severity: blocking
---

**Rule.** In SkSL analysis, optimization, and transformation passes, do not use
SkASSERT to assume that AST expressions conform to the specific subexpression
patterns or node types supported by the pass. Explicitly verify expression
shapes and child node types at runtime (such as checking with .is<T>()), and
gracefully skip the optimization (e.g., via continue or returning false) when
encountering valid but unhandled AST constructs.

**Why.** Valid shaders can construct complex expression ASTs, such as nested
struct field accesses (a.b.c) where a FieldAccess base is another FieldAccess
rather than a VariableReference. Asserting that AST expressions match an
optimization's expected pattern crashes debug builds on valid user programs and
causes undefined downcasts in release builds, whereas graceful fallbacks safely
preserve correct unoptimized execution.

## Examples

Bad:

```cpp
} else if (arg.is<FieldAccess>()) {
    const std::unique_ptr<Expression>& accessBase = arg.as<FieldAccess>().base();
    SkASSERT(accessBase->is<VariableReference>());
    var = accessBase->as<VariableReference>().variable();
}
```

Good:

```cpp
} else if (arg.is<FieldAccess>() &&
           arg.as<FieldAccess>().base()->is<VariableReference>()) {
    argBase = arg.as<FieldAccess>().base()->as<VariableReference>().variable();
} else {
    continue;  // Skip unsupported expression gracefully
}
```
