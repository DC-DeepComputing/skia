---
name: skia/assign-smart-pointers-directly-not-reset-release
description: When transferring or assigning ownership between smart pointers, use direct or move assignment instead of calling .reset() with other.release()
rubric: readability
severity: nit
---

**Rule.** Prefer direct assignment or move assignment (e.g., `sp = expr;` or `sp
= std::move(other);`) when transferring ownership to a smart pointer (`sk_sp` or
`std::unique_ptr`). Do not unwrap and re-adopt the pointer by calling
`.reset(expr.release())`.

**Why.** Move assignment via `operator=` cleanly transfers ownership and resets
any existing managed object. Calling `.reset(other.release())` is redundant,
verbose, unnecessarily extracts a raw pointer, and bypasses smart pointer type
checks.

## Examples

Bad:

```cpp
// Redundant unwrap and re-adopt via raw pointer:
fData.reset(newData.detachAsData().release());
fSink.reset(otherSink.release());
```

Good:

```cpp
// Direct move assignment cleanly transfers ownership:
fData = newData.detachAsData();
fSink = std::move(otherSink);
```
