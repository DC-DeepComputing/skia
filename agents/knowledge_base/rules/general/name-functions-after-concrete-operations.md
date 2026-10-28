---
name: skia/name-functions-after-concrete-operations
description: When naming functions or methods performing multiple steps, describe their concrete operations and side effects rather than using vague verbs like update or process
rubric: readability
severity: should_fix
---

**Rule.** Name functions and methods explicitly after their concrete operations
and side effects rather than using vague or generic verbs (such as 'update...',
'process...', or 'handle...') that obscure what the function actually does. If a
function performs multiple core tasks or state mutations, reflect those primary
responsibilities directly in the name (e.g., 'applySpacingAndBuildClusterTable'
instead of 'updateShapingInfo').

**Why.** Generic verbs like 'update' or 'process' conceal distinct side effects,
prerequisites, and mutations behind an ambiguous label. Explicitly naming the
core operations clarifies execution order and component contracts at call sites
without forcing readers to inspect the function implementation.

## Examples

Bad:

```cpp
// Overly broad name that hides distinct operations and side effects
void ParagraphImpl::updateShapingInfo() {
    this->applySpacing();
    this->buildClusterTable();
}
```

Good:

```cpp
// Explicit name reflecting the concrete operations performed
void ParagraphImpl::applySpacingAndBuildClusterTable() {
    this->applySpacing();
    this->buildClusterTable();
}
```
