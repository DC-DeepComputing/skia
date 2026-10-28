---
name: skia/bail-out-or-fallback-before-optional-dereference
description: When handling an empty std::optional check (!opt), ensure the failure branch early-returns, throws, or provides a fallback before subsequent code dereferences *opt or opt->
rubric: correctness
severity: blocking
---

**Rule.** When checking a fallible operation returning std::optional (such as
matrix.invert() or lookup queries), ensure that handling an empty state (e.g.,
if (!opt)) terminates execution with an early return, throw, or bailout before
subsequent code dereferences the optional (*opt or opt->). If fallback behavior
without bailing out is intended, use opt.value_or(...) or supply default values
rather than allowing control flow to fall through to an unverified dereference.

**Why.** Failing to early-return or bail out after handling an empty optional
allows execution to fall through and unconditionally dereference *opt or opt->,
causing undefined behavior or crashes on std::nullopt in release and debug
builds. Using opt.value_or(...) or returning early makes fallback and
control-flow semantics explicit.

## Examples

Bad:

```cpp
auto inverse = matrix.invert();
if (!inverse) {
    SkPath empty;
    glyph.setPath(alloc, &empty, false, pathModified);
    // Missing return: execution falls through to null dereference below.
}
auto localPath = path.makeTransform(*inverse);
```

Good:

```cpp
auto inverse = matrix.invert();
if (!inverse) {
    SkPath empty;
    glyph.setPath(alloc, &empty, false, pathModified);
    return;
}
auto localPath = path.makeTransform(*inverse);
```
