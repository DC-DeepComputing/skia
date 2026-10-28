---
name: skia/omit-redundant-rust-loop-and-array-borrows
description: In Rust loops and collections, iterate slices without redundant & patterns and pass owned arrays to Vec::extend instead of extend_from_slice(&...)
rubric: readability
severity: nit
---

**Rule.** In Rust, iterate over slice elements with 'for value in slice' rather
than destructuring with 'for &value in slice' when method calls on the element
auto-dereference. Additionally, when appending owned fixed-size arrays (such as
the return value of .to_le_bytes() or array literals) to a Vec, pass the array
directly to Vec::extend by value rather than calling extend_from_slice(&...).

**Why.** Rust method call syntax automatically dereferences reference receivers
(such as calling .to_le_bytes() on a &u32 reference), rendering & destructuring
in slice loops redundant. Furthermore, fixed-size arrays implement IntoIterator,
allowing Vec::extend to consume owned arrays directly without creating
intermediate slice borrows.

## Examples

Bad:

```cpp
for &value in values {
    bytes.extend_from_slice(&value.to_le_bytes());
}
```

Good:

```cpp
for value in values {
    bytes.extend(value.to_le_bytes());
}
```
