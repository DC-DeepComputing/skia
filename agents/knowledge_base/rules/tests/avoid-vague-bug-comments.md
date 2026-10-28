---
name: skia/tests/avoid-vague-bug-comments
description: When commenting unit tests or code, avoid vague historical bug references (e.g., 'had been buggy'); either explain the concrete invariant tested or omit the comment
rubric: readability
severity: nit
---

**Rule.** Do not leave vague comments referencing historical bugs or regressions
(such as 'had been buggy at one point' or 'used to fail'). Either document the
concrete invariant, root cause, or failure mode being tested (linking a tracking
issue if applicable), or omit the comment and let self-explanatory assertions
and code document expectations.

**Why.** Vague comments referencing past bugs provide no actionable technical
context for future maintainers and become obsolete lore once the bug is
resolved. Documenting the concrete failure mode or invariant clarifies the
test's intent and protects against regressions, while self-explanatory
assertions make redundant narrative comments unnecessary.

## Examples

Bad:

```cpp
// Test copy constructor (which had been buggy at one point)
GrBackendSemaphore copy(sema);
REPORTER_ASSERT(reporter, copy.isInitialized());
```

Good:

```cpp
// Test that copy constructor preserves initialization and backend API.
GrBackendSemaphore copy(sema);
REPORTER_ASSERT(reporter, copy.isInitialized());
REPORTER_ASSERT(reporter, copy.backend() == GrBackendApi::kVulkan);

// Or omit the comment if assertions are self-explanatory:
GrBackendSemaphore copy(sema);
REPORTER_ASSERT(reporter, copy.isInitialized());
```
