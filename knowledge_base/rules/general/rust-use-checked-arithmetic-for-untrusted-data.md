---
name: skia/rust-use-checked-arithmetic-for-untrusted-data
description: When computing buffer offsets, slice ranges, or payload lengths from untrusted binary data in Rust, use checked arithmetic instead of raw arithmetic operators
rubric: safety
severity: should_fix
---

**Rule.** When computing buffer offsets, slice ranges, or payload lengths from
untrusted binary input in Rust, use checked arithmetic (`checked_add`,
`checked_sub`, `checked_mul`) rather than raw arithmetic operators (`+`, `-`,
`*`) or saturating operations. Return early with an error or fallback value if
any checked computation returns `None`.

**Why.** Untrusted binary streams (such as image decoders or metadata parsers)
can supply crafted or corrupt dimensions and offsets. In Rust, raw arithmetic
operators can wrap in release mode or panic in debug mode upon overflow, leading
to out-of-bounds panics or truncated slice ranges. Checked arithmetic ensures
invalid offsets fail gracefully without panicking.

## Examples

Bad:

```cpp
let param_start = seg.offset as usize + 2 /* marker */ + 2 /* length field */;
let param_len = (seg.parameter_length as usize).saturating_sub(2);
let end = param_start + param_len;
if end <= raw_data.len() {
    &raw_data[param_start..end]
} else {
    &[]
}
```

Good:

```cpp
let Some(param_start) = (seg.offset as usize)
    .checked_add(2 /* marker */)
    .and_then(|offset| offset.checked_add(2 /* length field */))
else {
    return &[];
};
let Some(param_len) = (seg.parameter_length as usize).checked_sub(2) else {
    return &[];
};
let Some(end) = param_start.checked_add(param_len) else {
    return &[];
};
if param_start <= raw_data.len() && end <= raw_data.len() {
    &raw_data[param_start..end]
} else {
    &[]
}
```
