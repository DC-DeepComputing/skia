---
name: skia/document-narrowing-integer-bounds
description: When narrowing integer types or accumulating into fixed-width integers using remote bounds, document the limiting invariant in an inline comment
rubric: readability
severity: should_fix
---

**Rule.** When narrowing integer types or accumulating into fixed-width integers
(such as with SkTo<uint16_t>, SkToU16, or SkToU8) where values are bounded
non-locally, document the upper bound or limiting invariant in an inline
comment. Explain why truncation or overflow cannot occur (e.g., citing a remote
helper's bailout threshold or structural limit) so readers and reviewers know
the cast is safe.

**Why.** Narrowing casts and fixed-width integer accumulations look like
potential truncation or overflow vulnerabilities when the input appears
arbitrarily sized. Documenting the non-local constraint or helper bailout
clarifies why the value is guaranteed to fit within the narrower integer type
and prevents mistaken review comments or unnecessary runtime checks.

## Examples

Bad:

```cpp
int dataKeySize = path_key_from_data_size(fShape.path());
if (dataKeySize >= 0) {
    count += SkTo<uint16_t>(dataKeySize);
}
```

Good:

```cpp
// When >= 0, `dataKeySize` is a small number bounded by
// kMaxKeyFromDataVerbCnt since point count is derived from verb count.
int dataKeySize = path_key_from_data_size(fShape.path());
if (dataKeySize >= 0) {
    count += SkTo<uint16_t>(dataKeySize);
}
```
