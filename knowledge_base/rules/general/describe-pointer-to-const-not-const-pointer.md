---
name: skia/describe-pointer-to-const-not-const-pointer
description: When documenting APIs returning or accepting read-only data, describe the type as 'pointer to const' rather than 'const pointer' to convey pointee immutability
rubric: readability
severity: nit
---

**Rule.** When documenting APIs, functions, or release notes that return or
accept read-only data (such as `const T*`), describe the type as a 'pointer to
const T' rather than a 'const pointer to T'. Ensure documentation accurately
distinguishes between pointee data immutability (low-level const) and pointer
variable immutability (top-level const).

**Why.** In C++, a 'const pointer' (`T* const`) signifies that the pointer
variable itself is immutable while the pointee can still be mutated, whereas a
'pointer to const' (`const T*`) indicates that the pointed-to data is read-only.
Using precise terminology in release notes and API documentation prevents
ambiguity about whether the pointer variable or its underlying data is
immutable.

## Examples

Bad:

```cpp
`SkImage::refEncodedData()` and `SkImageGenerator::refEncodedData()` now returns a const pointer to the SkData to more explicitly signal that this is a read-only view into the data.
```

Good:

```cpp
`SkImage::refEncodedData()` and `SkImageGenerator::refEncodedData()` now returns a pointer to const SkData to more explicitly signal that this is a read-only view into the data.
```
