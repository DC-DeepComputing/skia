---
name: skia/sksl/report-compiler-errors-via-error-reporter
description: In SkSL passes and backend codegen, report unsupported features or malformed input via ErrorReporter instead of SkASSERT or SK_ABORT, reserving SkDEBUGFAIL for impossible frontend-validated states
rubric: correctness
severity: should_fix
---

**Rule.** In SkSL analysis, optimization, and backend codegen passes, report
unsupported language features, unhandled operators, or unexpected AST states
reachable by valid or malformed shader input via ErrorReporter (such as
fErrors->error) instead of crashing or asserting with SkASSERT, SK_ABORT, or
SkUNREACHABLE. Conversely, for conditions guaranteed impossible by frontend
semantic analysis and type checking, do not emit user-facing compiler errors;
use SkDEBUGFAIL or SkDEBUGFAILF (with safe bailout) to flag internal compiler
invariant bugs in debug builds.

**Why.** Valid shaders and fuzzers must not crash debug builds or trigger
assertions/aborts on unsupported features, nor silently fall through in release
builds where SkASSERT is compiled out. Gracefully reporting errors via
ErrorReporter provides actionable compiler diagnostics. Conversely, impossible
frontend-validated states represent internal compiler bugs rather than user
errors; reporting them via ErrorReporter misattributes bugs, while SkDEBUGFAIL
alerts developers in debug builds without crashing production builds.

## Examples

Bad:

```cpp
// Bad: asserting or aborting on unsupported features or reachable input:
Operator op = p.getOperator();
SkASSERT(op.kind() != Operator::Kind::PLUSPLUS);
// or:
SK_ABORT("Specialization requires a uniform or parameter variable");

// Bad: emitting user-facing errors for impossible backend IR states:
if (atomicPtrId == NA) {
    fContext.fErrors->error(c.fPosition, "invalid atomic pointer argument to atomic operation");
    return NA;
}
```

Good:

```cpp
// Good: report unsupported features or reachable errors via ErrorReporter:
Operator op = p.getOperator();
if (op.kind() == Operator::Kind::PLUSPLUS) {
    fContext.fErrors->error(p.fPosition, "prefix '++' not yet supported in WGSL backend");
    return;
}
// or:
fContext.fErrors->error(call.fPosition, "Specialization requires a uniform or parameter variable");

// Good: use SkDEBUGFAIL for impossible internal compiler states:
if (atomicPtrId == NA) {
    SkDEBUGFAILF("atomic intrinsic expected a pointer argument: %s",
                 arguments[0]->description().c_str());
    return NA;
}
```
