---
name: skia/factor-shared-condition-across-branches
description: When conditional branches share a common check or virtual method call, factor the check into an outer condition rather than repeating identical dispatch across branches
rubric: safety
severity: nit
---

**Rule.** When conditional branches share a common condition or method call
(such as `if (A && B) ... else if (A)`), do not repeat the shared check across
branches—especially for virtual methods or queries that incur dynamic dispatch
overhead. Factor the shared check into an enclosing `if` block with nested
branches, or cache the query result in a local variable upfront.

**Why.** Virtual method calls require dynamic dispatch through a vtable and
generally cannot be devirtualized or common-subexpression-eliminated across
branches by the compiler. Factoring the common check into an outer condition
eliminates redundant dynamic dispatch and simplifies control flow.

## Examples

Bad:

```cpp
// Repeating identical virtual check across branches:
if (stream->hasLength() && stream->hasPosition()) {
    size_t remainingBytes = stream->getLength() - stream->getPosition();
    return remainingBytes < len;
} else if (stream->hasLength()) {
    return stream->getLength() < len;
}
```

Good:

```cpp
// Factoring shared condition into an outer condition block:
if (stream->hasLength()) {
    if (stream->hasPosition()) {
        size_t remainingBytes = stream->getLength() - stream->getPosition();
        return remainingBytes < len;
    }
    return stream->getLength() < len;
}
```
