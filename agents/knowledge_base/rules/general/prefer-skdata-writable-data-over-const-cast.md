---
name: skia/prefer-skdata-writable-data-over-const-cast
description: When obtaining a mutable pointer to an SkData buffer, use data->writable_data() instead of casting away const with const_cast on data->data()
rubric: readability
severity: nit
---

**Rule.** When a non-const pointer to an SkData buffer is required (such as when
passing to legacy or C library APIs), call data->writable_data() instead of
casting away constness via const_cast<void*>(data->data()). Reserve data()
strictly for read-only access.

**Why.** SkData provides writable_data() specifically for cases where mutable
pointer access to the underlying buffer is required. Using const_cast on data()
bypasses SkData's explicit API, obscures call-site intent, and undermines
const-correctness.

## Examples

Bad:

```cpp
// Avoid const_cast on data() when mutable buffer access is required
gmapChunk.data = reinterpret_cast<png_byte*>(const_cast<void*>(gainmapVersion->data()));
```

Good:

```cpp
// Use the dedicated SkData accessor for mutable buffer access
gmapChunk.data = reinterpret_cast<png_byte*>(gainmapVersion->writable_data());
```
