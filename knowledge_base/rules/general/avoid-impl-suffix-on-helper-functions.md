---
name: skia/avoid-impl-suffix-on-helper-functions
description: When defining helper functions in C++ source files, avoid appending _impl suffixes; use descriptive names or C++ function overloading instead
rubric: readability
severity: should_fix
---

**Rule.** Do not append _impl suffixes to helper function names in C++ files.
Give helper functions clean, descriptive names, and leverage C++ function
overloading when multiple helper variants share the same intent but accept
different parameter sets.

**Why.** Artificial _impl suffixes clutter identifiers and leak internal
implementation details into function names. Using descriptive names clarifies
each function's purpose at call sites, while function overloading allows related
variants to share a clean, unified name naturally without pseudo-private naming
decorations.

## Examples

Bad:

```cpp
// Artificial _impl suffix clutters helper name
static void assert_color_in_range_impl(skiatest::Reporter* reporter,
                                       const SkPixmap& pixmap,
                                       const std::string& testName,
                                       SkIPoint pt,
                                       SkColor expected_min,
                                       SkColor expected_max) {
    ...
}
```

Good:

```cpp
// Clean descriptive helper name; use function overloading for parameter variants
static void assert_color_in_range(skiatest::Reporter* reporter,
                                  const SkPixmap& pixmap,
                                  const std::string& testName,
                                  SkIPoint pt,
                                  SkColor expected_min,
                                  SkColor expected_max) {
    ...
}
```
