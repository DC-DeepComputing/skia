---
name: skia/api/accept-metadata-struct-in-container-writers
description: When designing serialization or writer functions for multi-property metadata containers like EXIF, accept the comprehensive metadata struct rather than single-field parameters
rubric: design
severity: should_fix
---

**Rule.** When designing serialization or writer APIs for container formats that
hold multiple metadata properties (such as EXIF, TIFF, or container image
headers), accept the general metadata struct (such as SkExif::Metadata) rather
than creating dedicated single-property writer functions like
WriteExifWithOrigin. Keep writer signatures symmetric with parser APIs that
populate metadata containers, enabling encoding of all present tags into a
single container block.

**Why.** Container formats like EXIF pack multiple properties into a single
payload block. Writing dedicated functions for individual fields prevents
encoding multiple tags into the same block, bloats the API surface, and breaks
symmetry with reader/parser APIs that operate on metadata structs.

## Examples

Bad:

```cpp
// Flawed: Single-property writer prevents encoding multiple tags into one EXIF block.
sk_sp<SkData> WriteExifWithOrigin(const SkEncodedOrigin origin);
```

Good:

```cpp
// Idiomatic: Accepts the comprehensive metadata struct, matching Parse(Metadata&, ...).
sk_sp<SkData> WriteExif(Metadata& metadata);
```
