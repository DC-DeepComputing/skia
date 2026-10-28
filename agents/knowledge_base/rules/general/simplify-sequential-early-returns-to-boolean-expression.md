---
name: skia/simplify-sequential-early-returns-to-boolean-expression
description: When returning boolean status from condition checks without intermediate side effects, return boolean expressions directly or chain short-circuiting logical operators instead of branching
rubric: readability
severity: nit
---

**Rule.** When returning a boolean value from condition checks without
intermediate side effects, return the boolean expression directly (`return
cond;`) or chain short-circuiting logical expressions (`return a && b;` or
`return a || b;`) instead of writing redundant `if (cond) return true; return
false;` branches or repetitive sequential `if (!cond) return false;` checks.

**Why.** Directly returning or chaining boolean expressions eliminates redundant
conditional branching and nesting depth, simplifies control flow, and adheres to
idiomatic C++ while preserving identical short-circuit evaluation semantics.

## Examples

Bad:

```cpp
// Redundant single-condition branch:
if (globalCache->isResourceTracked(resource)) {
    return true;
}
return false;

// Redundant sequential early returns:
if (!visitor(fSrcProxy.get())) {
    return false;
}
if (!visitor(fDstProxy.get())) {
    return false;
}
return true;
```

Good:

```cpp
// Return boolean expression directly:
return globalCache->isResourceTracked(resource);

// Chain short-circuiting logical expressions directly:
return visitor(fSrcProxy.get()) && visitor(fDstProxy.get());
```
