---
name: skia/example/validate-newly-allocated-object-null-check
description: When validating fallible resource allocations in sequential setup code, verify that null checks test the newly created object rather than repeating prior checks
rubric: correctness
severity: should_fix
---

**Rule.** When performing sequential resource allocations or object creations
(such as surfaces, images, or animations), verify that each subsequent null
check tests the newly created object variable rather than repeating a check on
an earlier variable. Ensure failure branches correctly guard the new resource
against null pointer dereferences.

**Why.** Copy-paste errors that re-test a previously validated variable leave
newly allocated resources unchecked. This renders the error-handling branch dead
code and leads to null pointer dereferences when allocation fails.

## Examples

Bad:

```cpp
auto animation = skottie::Animation::MakeFromFile(argv[1]);
if (!animation) {
    return 1;
}
auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(surfaceSize, nullptr));
if (!animation) {  // Bug: repeats earlier check; surface is unchecked.
    std::printf("Cannot allocate surface\n");
    return 1;
}
```

Good:

```cpp
auto animation = skottie::Animation::MakeFromFile(argv[1]);
if (!animation) {
    return 1;
}
auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(surfaceSize, nullptr));
if (!surface) {  // Validates newly created resource.
    std::printf("Cannot allocate surface\n");
    return 1;
}
```
