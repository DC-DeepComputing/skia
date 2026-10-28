---
name: skia/tests/factor-shared-test-utilities
description: When test helper functions or fixture datasets are shared across multiple unit test files, factor them into dedicated test utility headers
rubric: design
severity: should_fix
---

**Rule.** When test helper functions, fixture generators, or test case datasets
are duplicated or shared across multiple test files, extract them into dedicated
test utility headers and sources (such as `*TestUtils.h` or `*TestCases.h`)
rather than copying them across test files. Include the shared test utility file
across consuming test suites.

**Why.** Duplicating helper functions and fixture datasets across multiple test
files creates maintenance overhead, increases code bloat, and risks behavioral
divergence when fixes or modifications are applied to only one copy. Factoring
shared test logic into dedicated utility files keeps tests DRY and ensures
consistent test setup across suites.

## Examples

Bad:

```cpp
// tests/graphite/sparse_strips/ClipTest.cpp
// Duplicating helper logic and fixture transformations across test files
std::vector<Line> scale_lines(const std::vector<Line>& input, float scale) {
    if (scale == 1.0f) return input;
    std::vector<Line> scaled;
    scaled.reserve(input.size());
    for (const auto& l : input) {
        scaled.push_back({{l.p0.fX * scale, l.p0.fY * scale}, {l.p1.fX * scale, l.p1.fY * scale}});
    }
    return scaled;
}
```

Good:

```cpp
// tests/graphite/sparse_strips/TileTestCases.h
namespace skgpu::graphite::TileTestCases {
struct TileTestCase {
    TileTestCase(const char* name,
                 std::initializer_list<Line> lines,
                 std::initializer_list<Tile> expected,
                 float scale);
    const char* fName;
    SkTDArray<Line> fLines;
    SkTDArray<Tile> fExpected;
};
std::vector<TileTestCase> Get(float scale, uint16_t viewportDim);
}  // namespace skgpu::graphite::TileTestCases
```
