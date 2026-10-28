---
name: skia/api/prefer-class-for-non-trivial-types
description: When declaring types with inheritance or private members, use class rather than struct and stage downstream forward-declaration changes using build flags
rubric: readability
severity: should_fix
---

**Rule.** Declare types as 'class' rather than 'struct' when they involve
inheritance, private members, or encapsulated invariants; reserve 'struct' for
simple data-carrying aggregates. When changing an existing public type from
'struct' to 'class' causes forward-declaration mismatches in downstream
embedders like Chromium (-Wmismatched-tags), do not compromise type declarations
by keeping them 'struct'; stage the transition using a temporary build flag.

**Why.** In Skia, 'struct' signals plain passive data structures primarily meant
for direct field access, whereas types with inheritance and member functions
represent encapsulated abstractions that belong in 'class'. Downgrading types to
'struct' to avoid staging forward-declaration updates compromises code design,
whereas temporary build flags safely coordinate the transition across downstream
repositories.

## Examples

Bad:

```cpp
// Compromising type declaration to 'struct' to avoid staging downstream forward-declaration mismatches:
struct SK_API DawnTextureInfo final : public TextureInfo::Data {
public:
    ...
};
```

Good:

```cpp
// Declare non-trivial types as 'class' and stage downstream forward-declaration updates with a build flag:
#if defined(SK_DAWN_TEXTURE_INFO_IS_STRUCT)
struct SK_API DawnTextureInfo final : public TextureInfo::Data {
#else
class SK_API DawnTextureInfo final : public TextureInfo::Data {
#endif
public:
    ...
};
```
