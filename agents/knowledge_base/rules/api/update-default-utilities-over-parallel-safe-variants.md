---
name: skia/api/update-default-utilities-over-parallel-safe-variants
description: When addressing numerical overflow or edge cases in utility functions or macros, update the default implementation directly instead of introducing parallel Safe variants
rubric: design
severity: should_fix
---

**Rule.** Do not introduce a parallel Safe... or ..._safe variant of an existing
utility function or macro (such as SkScalarSafeAve) when addressing numerical
overflow or boundary edge cases. Instead, update the default utility
implementation directly so all existing callers benefit from the fix, unless
profiling proves that an unchecked legacy path is strictly required for
performance.

**Why.** Introducing separate 'Safe' variants alongside existing helpers
fragments the API and leaves existing call sites vulnerable to the unhandled
overflow or edge case. Updating the default implementation keeps the interface
unified and ensures consistent correctness across all callers without code
duplication.

## Examples

Bad:

```cpp
// Bad: Introduces a separate Safe macro, leaving existing callers of SkScalarAve vulnerable to overflow
#define SkScalarHalf(a)             ((a) * SK_ScalarHalf)
#define SkScalarAve(a, b)           (((a) + (b)) * SK_ScalarHalf)
#define SkScalarSafeAve(a, b)       (SkScalarHalf(a) + SkScalarHalf(b))
```

Good:

```cpp
// Good: Fixes the default macro directly so all callers are protected from overflow
#define SkScalarHalf(a)             ((a) * SK_ScalarHalf)
#define SkScalarAve(a, b)           (SkScalarHalf(a) + SkScalarHalf(b))
```
