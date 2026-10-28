---
name: skia/modules/name-glyph-text-mappings-clusters
description: When defining text shaping APIs, flags, or wrapper structs mapping glyph indices to text source offsets, name the collection clusters instead of encoding-prefixed names like UTFClusters
rubric: readability
severity: should_fix
---

**Rule.** In text shaping interfaces, wrapper structures, and run buffers, name
identifiers and flag constants representing glyph-to-source-offset mappings
'clusters' (e.g., 'kClusters', 'fClusters') rather than encoding-prefixed names
like 'kUTFClusters'. When exposing dense glyph-index-to-text-offset collections,
document the mapping semantics and indexing units in comments.

**Why.** In text shaping subsystems such as HarfBuzz and SkShaper, 'cluster' is
the established standard term for mapping output glyph indices to input text
source offsets. Prefixing names with 'UTF' is misleading because the array is
indexed by glyph position rather than UTF code units, and diverging from
canonical SkShaper naming impairs readability across text shaping modules.

## Examples

Bad:

```cpp
enum Flags : uint32_t {
    kUTFClusters = 0x04,
};

// Misleading name suggests array is keyed by UTF units
std::vector<size_t> fUTFClusters;
```

Good:

```cpp
enum Flags : uint32_t {
    kClusters = 0x04,
};

// fClusters[i] is an input string index pointing to the start of the UTF sequence
// associated with fGlyphs[i]. The number of entries matches the number of glyphs.
std::vector<size_t> fClusters;
```
