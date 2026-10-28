---
name: skia/avoid-duplicate-consecutive-statements
description: When authoring or refactoring statements, avoid consecutive duplicate assignments or identical function calls without intervening side effects to eliminate redundant dead operations
rubric: readability
severity: nit
---

**Rule.** Do not write or retain consecutive duplicate statements, assignments,
or identical function calls without intervening side effects or state changes.
When authoring or refactoring logic, verify that duplicate lines introduced by
copy-paste or editor slips are removed so each statement performs distinct work.

**Why.** Consecutive duplicate statements or assignments without intervening
mutations are dead code that add visual noise, waste execution cycles, and often
signal an accidental copy-paste error where a different variable or operation
was intended.

## Examples

Bad:

```cpp
fExceededMaxLines = textWrapper.exceededMaxLines();
fExceededMaxLines = textWrapper.exceededMaxLines();
```

Good:

```cpp
fExceededMaxLines = textWrapper.exceededMaxLines();
```
