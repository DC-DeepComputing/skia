---
name: skia/prefer-slice-get-for-fallible-subslices
description: When extracting fallible subslices in Rust, use slice.get(range) with the ? operator instead of manual len() bounds checks and direct indexing
rubric: readability
severity: should_fix
---

**Rule.** When extracting fallible subslices from a slice in Rust, use
slice.get(offset..end) (or slice.get_mut(...)) with the ? operator or pattern
matching instead of manually comparing len() and indexing with
&slice[offset..end]. Rely on slice.get() to safely perform bounds checks and
return an Option without panicking.

**Why.** Direct slice indexing (&slice[range]) panics on out-of-bounds indices,
requiring manual, error-prone bounds checks against slice.len(). Using
slice.get(range) safely returns None on out-of-bounds ranges, replacing verbose
manual comparisons with concise, idiomatic ? error propagation.

## Examples

Bad:

```cpp
let end = offset.checked_add(size)?;
if end > data.len() {
    return None;
}
let subslice = &data[offset..end];
```

Good:

```cpp
let end = offset.checked_add(size)?;
let subslice = data.get(offset..end)?;
```
