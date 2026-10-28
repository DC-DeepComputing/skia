---
name: skia/document-deferred-error-handling-invariants
description: When executing intermediate operations with pre-initialized or dummy data under deferred error checking, document the safety invariant and discard boundary in inline comments
rubric: readability
severity: should_fix
---

**Rule.** When code relies on deferred or centralized error handling and allows
intermediate operations to execute with dummy, stale, or pre-initialized scratch
values before a failure is checked, document the safety invariant in an inline
comment. Explicitly explain why intermediate operations on unverified data
cannot cause undefined behavior or crash hazards, and identify the transaction
boundary, loop header, or bailout point that detects failure and discards
intermediate results.

**Why.** Unchecked execution following fallible operations looks like a missing
error check or data corruption bug to reviewers and static analyzers.
Documenting the safety invariant and discard mechanism clarifies why
intermediate dummy values are benign and prevents maintainers from introducing
unnecessary checks or refactoring working loops.

## Examples

Bad:

```cpp
// Pre-initialized to avoid MSAN warnings.
SkPoint points[3] = {{0, 0}, {0, 0}, {0, 0}};
...
case 'M':
    data = find_points(data, points, 1, relative, &c);
    path.moveTo(points[0]);
```

Good:

```cpp
// Pre-initialize to avoid MSAN warnings if find_points fails.
SkPoint points[3] = {{0, 0}, {0, 0}, {0, 0}};
...
case 'M':
    data = find_points(data, points, 1, relative, &c);
    // find_points might have failed, so points[0] may be stale or dummy.
    // However, data is set to nullptr on error and checked at the top of
    // the loop, discarding this entire local path before committing.
    path.moveTo(points[0]);
```
