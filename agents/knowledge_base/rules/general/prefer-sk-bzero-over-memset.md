---
name: skia/prefer-sk-bzero-over-memset
description: When clearing or copying memory buffers that may be empty or null, use sk_bzero and sk_careful_memcpy instead of memset, memcpy, or manual size checks
rubric: readability
severity: should_fix
---

**Rule.** Use sk_bzero(ptr, size) instead of std::memset(ptr, 0, size) when
clearing raw memory buffers to zero. Use sk_careful_memcpy(dst, src, len) or
sk_careful_memmove instead of manual if (size > 0) checks around memcpy/memmove
when copying buffers that may have zero length or null pointers.

**Why.** In C and C++, calling memset, memcpy, or memmove with null pointers is
undefined behavior even when the length is zero. Compilers like GCC can exploit
this undefined behavior to optimize away subsequent null checks. Skia provides
sk_bzero, sk_careful_memcpy, and sk_careful_memmove in SkMalloc.h to safely
handle zero-length buffers without ad-hoc size branching.

## Examples

Bad:

```cpp
// Redundant size check and raw memcpy
if (buffer && size > 0) {
    memcpy(buffer, fData->bytes() + fOffset, size);
}

// Undefined behavior if buffer is nullptr or size is 0
std::memset(fBuffer, 0, count * sizeof(*fBuffer));
```

Good:

```cpp
if (buffer) {
    sk_careful_memcpy(buffer, fData->bytes() + fOffset, size);
}

sk_bzero(fBuffer, count * sizeof(*fBuffer));
```
