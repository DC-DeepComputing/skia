---
name: skia/dont-return-false-for-sentinel-integers
description: When signaling failure or out-of-bounds in functions returning integral indices, line numbers, or offsets with negative sentinels, return the designated sentinel like -1 rather than false
rubric: correctness
severity: should_fix
---

**Rule.** In functions returning an integral index, line number, offset, or
status where -1 (or an invalid sentinel constant) indicates failure or
out-of-bounds input, do not return 'false' on error bailouts; explicitly return
the designated sentinel value (such as -1). Verify that error and early-return
paths match the integral return type rather than implicitly converting boolean
'false' to zero.

**Why.** In C++, boolean literals implicitly convert to integral types, where
'false' evaluates to 0. When a function returns a 0-based index, line number, or
offset, returning 'false' silently yields 0, masquerading as the first element
or valid starting position instead of signaling failure to the caller.

## Examples

Bad:

```cpp
int ParagraphImpl::getLineNumberAtUTF16Offset(size_t codeUnitIndex) {
    this->ensureUTF16Mapping();
    if (codeUnitIndex >= SkToSizeT(fUTF8IndexForUTF16Index.size())) {
        return false; // Implicitly converts to 0, indicating line 0 instead of failure!
    }
    const TextIndex utf8 = fUTF8IndexForUTF16Index[codeUnitIndex];
    return getLineNumberAt(utf8);
}
```

Good:

```cpp
int ParagraphImpl::getLineNumberAtUTF16Offset(size_t codeUnitIndex) {
    this->ensureUTF16Mapping();
    if (codeUnitIndex >= SkToSizeT(fUTF8IndexForUTF16Index.size())) {
        return -1; // Explicitly returns the designated out-of-bounds sentinel
    }
    const TextIndex utf8 = fUTF8IndexForUTF16Index[codeUnitIndex];
    return getLineNumberAt(utf8);
}
```
