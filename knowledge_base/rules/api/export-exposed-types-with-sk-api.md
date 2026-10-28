---
name: skia/api/export-exposed-types-with-sk-api
description: When annotating types, functions, or operators with SK_API, export exposed declarations in include/ but omit SK_API on internal declarations in src/
rubric: readability
severity: should_fix
---

**Rule.** Annotate classes and structs declared in public or exposed headers
(include/** and modules/**/include/**) with SK_API (or SK_SPI for private
headers) whenever they define non-inline member functions or virtual tables
implemented in library source files. Conversely, do not mark internal classes,
methods, or friend operators in src/ with SK_API or SK_SPI, reserving symbol
export macros strictly for Skia's public dynamic library boundary.

**Why.** In shared library builds, public types with out-of-line members must be
exported with SK_API to prevent unresolved external symbol errors for downstream
embedders. Conversely, marking internal classes and helper operators in src/
with SK_API unnecessarily pollutes the dynamic library export table and exposes
private implementation details.

## Examples

Bad:

```cpp
// include/private/SkHdrMetadata.h (exposed type missing SK_API)
struct ContentLightLevelInformation {
    bool parse(const SkData* data);
    sk_sp<SkData> serialize() const;
};

// src/core/SkPathData.h (internal operator marked with SK_API)
friend SK_API bool operator==(const SkPathData& a, const SkPathData& b);
```

Good:

```cpp
// include/private/SkHdrMetadata.h (exposed type exported with SK_API)
struct SK_API ContentLightLevelInformation {
    bool parse(const SkData* data);
    sk_sp<SkData> serialize() const;
};

// src/core/SkPathData.h (internal operator kept unexported)
friend bool operator==(const SkPathData& a, const SkPathData& b);
```
