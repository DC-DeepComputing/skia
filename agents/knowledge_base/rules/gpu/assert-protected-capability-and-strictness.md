---
name: skia/gpu/assert-protected-capability-and-strictness
description: When validating protected resource allocation in GPU backends, assert both that protected resources require capability support and strict protectedness requires protected allocations
rubric: correctness
severity: should_fix
---

**Rule.** When asserting protected resource validity in GPU backend
implementations, verify both directional invariants with SkASSERT: assert that
protected resources require backend capability support (`isProtected ==
Protected::kNo || caps.supportsProtectedContent()`) and that strict
protectedness configurations forbid non-protected resources
(`!caps.strictProtectedness() || isProtected == Protected::kYes`). Do not omit
either condition when refactoring capability guards.

**Why.** Decoupling strict protectedness checks from general protected content
support without asserting both conditions allows protected resources to be
created on backends that lack protected memory support when strict mode is
disabled, or allows unprotected resources when strict protectedness is enabled.

## Examples

Bad:

```cpp
// Bad: Only asserting strict protectedness, missing general capability check
if (this->glCaps().strictProtectedness()) {
    SkASSERT(isProtected == skgpu::Protected::kYes);
}
```

Good:

```cpp
// Good: Independently assert both capability support and strictness constraints
SkASSERT(isProtected == skgpu::Protected::kNo || this->glCaps().supportsProtectedContent());
SkASSERT(!this->glCaps().strictProtectedness() || isProtected == skgpu::Protected::kYes);
```
