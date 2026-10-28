---
name: skia/reuse-predicate-helpers-over-inline-checks
description: When evaluating compound type, flag, or state conditions, call existing predicate and classification helpers instead of open-coding inline checks
rubric: readability
severity: nit
---

**Rule.** When checking an entity's category, storage class, type, or state
flags, call existing predicate and classification helper functions rather than
open-coding compound boolean expressions or bitmask checks inline. If
classification logic or invariants are shared across call sites, reuse or
introduce a dedicated semantic predicate helper to encapsulate the checks.

**Why.** Inlining duplicate compound condition and flag checks increases
boilerplate, obscures semantic intent, and risks logic divergence when
classification criteria change. Reusing dedicated predicate helpers keeps
classification logic single-sourced and ensures callers consistently execute any
internal invariants or assertions.

## Examples

Bad:

```cpp
const Variable& var = *decls.varDeclaration().var();
// Bad: open-coding compound modifier and type checks inline
if (var.modifiers().fFlags & Modifiers::kUniform_Flag && !var.type().isOpaque()) {
    this->writeGlobalUniform(var);
}
```

Good:

```cpp
const Variable& var = *decls.varDeclaration().var();
// Good: calling existing classification helper encapsulates predicate logic
if (is_in_global_uniforms(var)) {
    this->writeGlobalUniform(var);
}
```
