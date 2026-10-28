---
name: skia/api/avoid-retaining-skfontarguments
description: When persisting, caching, comparing, or hashing font configurations, copy borrowed argument data into an owning structure rather than retaining or specializing std::hash for SkFontArguments
rubric: safety
severity: should_fix
---

**Rule.** Do not store or retain SkFontArguments in long-lived objects, structs,
or cache keys, and do not specialize std::hash or equality operators for it.
SkFontArguments is a transient, non-owning argument abstraction that borrows
pointers to variation coordinates and palette overrides from caller buffers;
copy the coordinates and overrides into an owning data structure (such as
FontArguments, SkFontDescriptor, or std::vector) and define equality and hashing
on that owning type instead.

**Why.** SkFontArguments does not copy or manage the lifetime of its variation
coordinates or palette overrides. Storing SkFontArguments across function
boundaries, in persistent structs, or defining std::hash and equality on it
encourages using it as a dictionary or cache key, risking dangling pointer
dereferences and use-after-free bugs when caller-owned buffers are deallocated.

## Examples

Bad:

```cpp
// Storing non-owning SkFontArguments in persistent keys or specializing std::hash for it
struct FamilyKey {
    std::vector<SkString> fFamilyNames;
    SkFontStyle fFontStyle;
    std::optional<SkFontArguments> fFontArgs;  // Holds borrowed pointers to coordinates/overrides
};

namespace std {
template<> struct hash<SkFontArguments> {
    size_t operator()(const SkFontArguments& args) const;
};
}
```

Good:

```cpp
// Storing owned data in a dedicated struct and defining hashing/equality on the owning type
struct FontArguments {
    int fCollectionIndex = 0;
    std::vector<SkFontArguments::VariationPosition::Coordinate> fCoordinates;
    int fPaletteIndex = 0;
    std::vector<SkFontArguments::Palette::Override> fPaletteOverrides;
    friend bool operator==(const FontArguments& a, const FontArguments& b);
};

namespace std {
template<> struct hash<FontArguments> {
    size_t operator()(const FontArguments& args) const;
};
}

struct FamilyKey {
    std::vector<SkString> fFamilyNames;
    SkFontStyle fFontStyle;
    std::optional<FontArguments> fFontArgs;  // Owns its data safely
};
```
