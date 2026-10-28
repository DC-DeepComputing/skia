---
name: skia/tests/document-unusually-large-test-surfaces
description: When allocating raster surfaces or bitmaps in unit tests, keep dimensions as small as possible and document why unusually large dimensions are required
rubric: readability
severity: nit
---

**Rule.** In unit tests, keep raster surfaces, bitmaps, and canvas dimensions as
small as possible to minimize memory footprint and execution time. When a test
genuinely requires unusually large surface dimensions to reproduce an edge case
or numerical instability, add an explanatory comment detailing why the large
size is necessary.

**Why.** Large test surface allocations consume unnecessary memory and slow down
test execution across test suites, especially under sanitizers like ASAN and
MSAN. Documenting the rationale for unusually large dimensions prevents
maintainers from mistakenly shrinking the surface or guessing why the size was
selected.

## Examples

Bad:

```cpp
DEF_TEST(path_walk_edges_concave_large_dx, r) {
    // Bad: Unusually large test surface with no explanation of why dimensions are needed.
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1000, 1000));
    SkCanvas* canvas = surface->getCanvas();
    ...
}
```

Good:

```cpp
DEF_TEST(path_walk_edges_concave_large_dx, r) {
    // The large surface size is necessary to reproduce the bug because we need
    // changes in y to be large enough while changes in x are much greater to
    // capture numerical instability when blitting the edge.
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(900, 700));
    SkCanvas* canvas = surface->getCanvas();
    ...
}
```
