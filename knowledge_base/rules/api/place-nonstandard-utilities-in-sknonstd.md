---
name: skia/api/place-nonstandard-utilities-in-sknonstd
description: When defining non-standard std-like utility functions or type traits, place them in namespace sknonstd in dedicated utility headers rather than the global namespace
rubric: readability
severity: should_fix
---

**Rule.** Place non-standard, std-like utility functions, algorithms, and type
traits inside namespace sknonstd in dedicated utility headers rather than in the
global namespace or in headers providing standard wrapper types. Keep standard
wrapper headers strictly aligned with C++ standard library equivalents, and
avoid polluting the global namespace with bespoke std-like extensions.

**Why.** Skia reserves namespace sknonstd for non-standard utility types and
helpers that feel std-like but are not part of the C++ standard library (as
documented in SkTLogic.h). Adding custom free functions or traits to the global
namespace or directly into standard replacement headers compromises
standard-compatibility, risks naming collisions, and complicates migration to
standard C++ vocabulary types.

## Examples

Bad:

```cpp
// In SkSpan_impl.h (global namespace alongside std::span wrapper)
template <typename T>
T* data_end(SkSpan<T> span) {
    return span.data() + span.size();
}
```

Good:

```cpp
// In a dedicated utility header
namespace sknonstd {
template <typename T>
T* data_end(SkSpan<T> span) {
    return span.data() + span.size();
}
}  // namespace sknonstd
```
