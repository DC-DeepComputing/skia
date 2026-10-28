---
name: skia/api/keep-core-font-arguments-backend-agnostic
description: When exposing font styling or configuration options in core font interfaces like SkFontArguments, model capabilities as backend-agnostic properties rather than backend-specific bitmasks
rubric: design
severity: should_fix
---

**Rule.** Do not expose platform- or backend-specific bitmasks or flags (such as
DirectWrite font simulations or FreeType load flags) in core public font
interfaces like SkFontArguments, SkFontDescriptor, or SkTypeface. Instead, model
styling and configuration capabilities (such as synthetic bold, synthetic
oblique, or variation axes) as first-class, backend-agnostic properties using
standard types like std::optional<bool> or dedicated enums. Allow individual
font backends to inspect and map these portable properties to their native
capabilities.

**Why.** SkFontArguments and related core font types are cross-platform
abstractions shared across all typeface backends (DirectWrite, CoreText,
FreeType, Fontations). Leaking backend-specific bitmasks breaks encapsulation
and forces other backends to interpret or ignore foreign flags, whereas portable
properties allow any capable backend to implement them consistently.

## Examples

Bad:

```cpp
// Bad: exposing backend-specific simulation flags in core font arguments
class SkFontArguments {
public:
    SkFontArguments& setSimulationsFlags(int dwSimulationsFlags) {
        fSimulationsFlags = dwSimulationsFlags;
        return *this;
    }
private:
    int fSimulationsFlags = 0;
};
```

Good:

```cpp
// Good: exposing portable styling properties as first-class options
class SkFontArguments {
public:
    SkFontArguments& setSyntheticBold(std::optional<bool> bold) {
        fSyntheticBold = bold;
        return *this;
    }
    SkFontArguments& setSyntheticOblique(std::optional<bool> oblique) {
        fSyntheticOblique = oblique;
        return *this;
    }
private:
    std::optional<bool> fSyntheticBold;
    std::optional<bool> fSyntheticOblique;
};
```
