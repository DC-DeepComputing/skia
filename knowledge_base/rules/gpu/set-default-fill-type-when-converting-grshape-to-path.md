---
name: skia/gpu/set-default-fill-type-when-converting-grshape-to-path
description: When converting GrShape or geometry containers with non-standard fill defaults to SkPath, explicitly set kDefaultFillType rather than relying on SkPath's default winding fill
rubric: correctness
severity: nit
---

**Rule.** When converting or materializing an SkPath from a GrShape (such as in
GrShape::asPath), explicitly set the path's fill type to
GrShape::kDefaultFillType. Do not omit setFillType or assume default-constructed
SkPath instances share GrShape's default fill convention.

**Why.** Default-constructed SkPath instances initialize with
SkPathFillType::kWinding (SkPathFillType::kDefault), whereas GrShape defines
kDefaultFillType as SkPathFillType::kEvenOdd. Omitting explicit fill type
configuration causes converted non-path shapes to render with winding fill rules
rather than GrShape's even-odd convention.

## Examples

Bad:

```cpp
SkPath GrShape::asPath() const {
    SkPathBuilder builder;
    // WRONG: Omitting setFillType leaves builder with SkPath's default kWinding fill
    if (fInverted) {
        builder.toggleInverseFillType();
    }
    return builder.detach();
}
```

Good:

```cpp
SkPath GrShape::asPath() const {
    SkPathBuilder builder;
    builder.setFillType(kDefaultFillType);
    if (fInverted) {
        builder.toggleInverseFillType();
    }
    return builder.detach();
}
```
