---
name: skia/prefer-direct-skcolor4f-member-access
description: When reading or extracting components from SkColor4f or SkRGBA4f, access public fields fR, fG, fB, and fA directly instead of calling .array() or using operator[]
rubric: readability
severity: nit
---

**Rule.** Access individual color components of SkColor4f (and SkRGBA4f)
directly through its public struct members (fR, fG, fB, fA) rather than
converting to an intermediate container via .array() or indexing with
operator[]. Reserve .array() and .vec() for passing components to external APIs
or algorithms that explicitly require std::array or contiguous float buffers.

**Why.** SkColor4f is a lightweight aggregate struct with public float members.
Converting to an intermediate std::array or indexing by integer subscripts
introduces unnecessary indirection and boilerplate when named fields (fR, fG,
fB, fA) provide direct, readable, and self-documenting member access.

## Examples

Bad:

```cpp
auto c4f = SkColor4f::FromColor(c);
auto arr = c4f.array();
ColorValue v({arr[0], arr[1], arr[2], arr[3]});

// Subscript indexing:
float r = c4f[0];
float g = c4f[1];
```

Good:

```cpp
auto c4f = SkColor4f::FromColor(c);
ColorValue v{c4f.fR, c4f.fG, c4f.fB, c4f.fA};

// Direct member access:
float r = c4f.fR;
float g = c4f.fG;
```
