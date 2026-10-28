---
name: skia/sync-declaration-count-comments
description: When writing or updating comments that specify numerical item counts for following declarations, verify the stated count matches the actual items
rubric: readability
severity: should_fix
---

**Rule.** When adding, modifying, or removing declarations introduced by a
comment specifying an explicit numerical count (such as enum values, methods, or
struct fields), verify that the stated count accurately matches the actual
number of subsequent items. Prefer keeping introductory comments descriptive
without hardcoded counts if the list is expected to change frequently, but
ensure any explicit numbers remain synchronized.

**Why.** Hardcoded numerical counts in section comments easily become stale as
items are added or removed during refactoring. Outdated counts mislead readers,
create ambiguity about whether declarations were inadvertently omitted or
misplaced, and erode trust in surrounding documentation.

## Examples

Bad:

```cpp
// The following 6 colortypes are just for reading from - not for rendering to
kR8G8_unorm_SkColorType,
kA16_float_SkColorType,
kR16_float_SkColorType,
kR16G16_float_SkColorType,
kA16_unorm_SkColorType,
kR16_unorm_SkColorType,
kR16G16_unorm_SkColorType,
kR16G16B16A16_unorm_SkColorType,
```

Good:

```cpp
// The following 8 colortypes are just for reading from - not for rendering to
kR8G8_unorm_SkColorType,
kA16_float_SkColorType,
kR16_float_SkColorType,
kR16G16_float_SkColorType,
kA16_unorm_SkColorType,
kR16_unorm_SkColorType,
kR16G16_unorm_SkColorType,
kR16G16B16A16_unorm_SkColorType,
```
