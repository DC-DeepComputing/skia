---
name: skia/name-template-constants-uppercamelcase-without-k-prefix
description: When declaring non-type template parameters or template constants, name them in UpperCamelCase without a leading 'k' rather than lowerCamelCase or kCamelCase
rubric: readability
severity: nit
---

**Rule.** Name non-type template parameters (template constants) using
UpperCamelCase (PascalCase) without a 'k' prefix (e.g., `AppendVertices`, not
`appendVerts` or `kAppendVerts`). Distinguish template parameters from runtime
function arguments (which use lowerCamelCase) and static or constexpr constants
(which use a leading 'k').

**Why.** In Skia coding style, all template parameters use UpperCamelCase to
distinguish them from runtime function arguments. Unlike constexpr variables or
enums, template constants do not take a 'k' prefix.

## Examples

Bad:

```cpp
template <bool appendVerts>
void reserve(unsigned int count);

template <int kTileWidth, bool kShouldSort>
void processTiles();
```

Good:

```cpp
template <bool AppendVerts>
void reserve(unsigned int count);

template <int TileWidth, bool ShouldSort>
void processTiles();
```
