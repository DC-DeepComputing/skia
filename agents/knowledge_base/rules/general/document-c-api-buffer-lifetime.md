---
name: skia/document-c-api-buffer-lifetime
description: When passing locally-scoped or stack buffers to C library APIs or structs requiring non-const pointers, document that internal copying makes immediate deallocation safe
rubric: readability
severity: should_fix
---

**Rule.** When passing stack-allocated or locally-scoped buffers to C library
APIs whose parameters or structs require non-const pointers (such as
png_unknown_chunk.data in png_set_unknown_chunks), document with an explanatory
comment that the library copies the buffer internally. Confirm that the
non-const pointer is not retained or mutated so reviewers and maintainers know
immediate deallocation is safe.

**Why.** Many C library APIs use non-const pointer types even when they only
read and copy input data. Passing locally-scoped buffers or casting away const
to non-const C structs creates apparent use-after-free and mutation hazards
unless comments confirm internal copy semantics and safe immediate deallocation.

## Examples

Bad:

```cpp
skhdr::ContentLightLevelInformation clli;
if (options.fHdrMetadata.getContentLightLevelInformation(&clli)) {
    auto data = clli.serializePngChunk();
    png_unknown_chunk chunk = {
        .name = {'c', 'L', 'L', 'I', 0},
        .data = reinterpret_cast<png_byte*>(data->writable_data()),
        .size = data->size(),
        .location = PNG_HAVE_IHDR,
    };
    png_set_unknown_chunks(fPngPtr, fInfoPtr, &chunk, 1);
}
```

Good:

```cpp
// png_unknown_chunk.data is non-const, but png_set_unknown_chunks copies
// the buffer internally, so immediate deallocation after the call is safe.
skhdr::ContentLightLevelInformation clli;
if (options.fHdrMetadata.getContentLightLevelInformation(&clli)) {
    auto data = clli.serializePngChunk();
    png_unknown_chunk chunk = {
        .name = {'c', 'L', 'L', 'I', 0},
        .data = reinterpret_cast<png_byte*>(data->writable_data()),
        .size = data->size(),
        .location = PNG_HAVE_IHDR,
    };
    png_set_unknown_chunks(fPngPtr, fInfoPtr, &chunk, 1);
}
```
