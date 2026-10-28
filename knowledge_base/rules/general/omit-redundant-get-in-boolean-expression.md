---
name: skia/omit-redundant-get-in-boolean-expression
description: When evaluating smart pointers like std::unique_ptr or sk_sp in boolean contexts, omit .get() and test the smart pointer directly
rubric: readability
severity: nit
---

**Rule.** Do not call `.get()` when evaluating a smart pointer (such as
`std::unique_ptr` or `sk_sp`) in a boolean expression, condition, or guard
(e.g., `ptr.get() && ...` or `if (ptr.get())`). Test the smart pointer directly
using its `operator bool()`.

**Why.** Smart pointer types like `std::unique_ptr` and `sk_sp` implement
`explicit operator bool()`, allowing direct contextual conversion to `bool` in
conditional statements, return expressions, and logical operators. Calling
`.get()` to test for nullness in a boolean context is redundant visual clutter
and unidiomatic in C++.

## Examples

Bad:

```cpp
// Redundant .get() in boolean conjunction
return encoder.get() && encoder->encodeRows(src.height());

// Redundant .get() in conditional guard
if (image.get()) {
    process(image.get());
}
```

Good:

```cpp
// Test the smart pointer directly
return encoder && encoder->encodeRows(src.height());

// Test the smart pointer directly
if (image) {
    process(image.get());
}
```
