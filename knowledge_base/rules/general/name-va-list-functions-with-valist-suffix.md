---
name: skia/name-va-list-functions-with-valist-suffix
description: When naming functions or methods that accept a va_list parameter, use a VAList suffix instead of a C-style v prefix
rubric: readability
severity: should_fix
---

**Rule.** Name functions and methods that accept a `va_list` parameter using a
`VAList` suffix (e.g., `SkLogVAList`, `appendVAList`, `printVAList`) rather than
a C standard library-style `v` prefix (e.g., `vSkLog`).

**Why.** A leading 'v' prefix is ambiguous and inconsistent with Skia's naming
conventions. Using the 'VAList' suffix clearly distinguishes `va_list` overloads
from their variadic (`...`) counterparts and maintains uniform, searchable API
naming across the codebase.

## Examples

Bad:

```cpp
void vSkLog(SkLogPriority priority, const char format[], va_list args) SK_PRINTF_LIKE(2, 0);
```

Good:

```cpp
void SkLogVAList(SkLogPriority priority, const char format[], va_list args) SK_PRINTF_LIKE(2, 0);
```
