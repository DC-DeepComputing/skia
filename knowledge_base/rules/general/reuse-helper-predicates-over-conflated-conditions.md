---
name: skia/reuse-helper-predicates-over-conflated-conditions
description: When combining conditional checks, reuse existing helper predicates rather than manually inlining and conflating their sub-conditions into compound boolean expressions
rubric: design
severity: should_fix
---

**Rule.** When extending or evaluating conditional checks, reuse existing helper
predicates rather than manually inlining and conflating their sub-conditions
into compound boolean expressions. Do not combine orthogonal flags and
qualification checks into a single factored expression (such as `(A || B) &&
C`), which duplicates classification logic and risks accidentally applying
qualification clauses to unrelated conditions.

**Why.** Conflating distinct conditions or flags into a single compound
expression duplicates qualification logic, risks unintended semantic coupling
(such as inadvertently subjecting orthogonal conditions `A` to qualifier `C`),
and diverges from the canonical helper definition. Calling dedicated helper
predicates preserves original condition semantics and keeps evaluation cohesive.

## Examples

Bad:

```cpp
// !var.type().isOpaque() inadvertently qualifies in/out flags as well:
if ((var.modifiers().fFlags &
     (Modifiers::kIn_Flag | Modifiers::kOut_Flag | Modifiers::kUniform_Flag)) &&
    !var.type().isOpaque()) {
    return;
}
```

Good:

```cpp
// Reuses helper predicate and keeps stage in/out condition separate:
if ((var.modifiers().fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag)) ||
    is_in_global_uniforms(var)) {
    return;
}
```
