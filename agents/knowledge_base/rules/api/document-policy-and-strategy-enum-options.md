---
name: skia/api/document-policy-and-strategy-enum-options
description: When declaring or documenting enum options or modes, explain each enumerator's behavior, trade-offs, custom approximations, and specification links directly rather than making broad subsystem claims
rubric: readability
severity: should_fix
---

**Rule.** When declaring or extending enums for configuration options,
operational policies, color spaces, or modes, document each enumerator to
clarify its behavior, trade-offs, and decision criteria for callers. For options
implementing non-obvious behaviors, custom approximations, or external standards
(such as CSS or W3C specifications), explain how the option behaves and include
a deep link to the relevant specification. Phrase comments to describe the
specific effect of selecting that variant rather than making standalone
statements that sound like general subsystem facts or intrinsic limitations.

**Why.** Bare enumerators leave callers uncertain about behavioral differences,
performance trade-offs, or which external standard and approximation is
implemented. Describing specific variant behaviors and citing specification
links ensures callers choose the intended option, while avoiding sweeping
subsystem statements prevents maintainers from mistaking an option's effect for
an intrinsic platform limitation.

## Examples

Bad:

```cpp
// Bad: bare enumerators without trade-off documentation.
enum class Reserve { kExact, kGrow };

// Bad: bare enumerator implementing an external standard and approximation without explanation or spec link.
enum class ColorSpace : uint8_t {
    kOKLab,
    kOKLabGamutMap,
};

// Bad: comment sounds like an intrinsic subsystem fact rather than option behavior.
enum class Cropping {
    kNone,    // Image filters don't use a crop rect.
    kContent, // Use the content bounding box for cropping.
};
```

Good:

```cpp
// Good: documents behavior and allocation trade-offs of each option.
enum class Reserve {
    kExact, // Reserves exact storage needed (never overallocates).
    kGrow,  // Allows buffer overallocation based on growth policy.
};

// Good: explains custom approximation and includes specification link.
enum class ColorSpace : uint8_t {
    kOKLab,
    // Same as kOKLab, but applies a simplified version of CSS gamut mapping
    // (https://www.w3.org/TR/css-color-4/#css-gamut-mapping) into Rec2020.
    kOKLabGamutMap,
};

// Good: describes the effect of selecting each variant.
enum class Cropping {
    kNone,    // Doesn't use a crop rect.
    kContent, // Uses the content bounding box as the crop rect.
};
```
