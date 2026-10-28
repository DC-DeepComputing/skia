---
name: skia/document-non-standard-color-channel-mappings
description: When repurposing semantically named structs like SkPMColor4f to store non-standard representations like polar colors, document channel mappings at access sites
rubric: readability
severity: nit
---

**Rule.** When repurposing a struct with semantically named fields (such as
SkPMColor4f or SkColor4f with fR, fG, fB, fA) to hold an alternate
representation (such as polar color space components H, C, L or non-RGB data),
document the non-standard channel mapping where individual fields are assigned
or accessed. Explicitly explain which semantic channels (such as hue stored in
the red channel) correspond to the struct's member fields so field accesses do
not appear as accidental bugs or typos.

**Why.** Named struct fields like fR strongly imply standard RGBA semantics.
When algorithms or color pipelines normalize alternative color spaces into
existing structs (such as storing hue in the first component), accessing
hueColor.fR without an explanatory comment appears to readers and reviewers as
an erroneous channel access or bug.

## Examples

Bad:

```cpp
auto colorWithHueFrom = [](const SkPMColor4f& color, const SkPMColor4f& hueColor) {
    return SkPMColor4f{hueColor.fR, color.fG, color.fB, color.fA};
};
```

Good:

```cpp
auto colorWithHueFrom = [](const SkPMColor4f& color, const SkPMColor4f& hueColor) {
    // If we have any powerless hue, then all colors are already in (some) polar space,
    // and they all store their hue in the red channel.
    return SkPMColor4f{hueColor.fR, color.fG, color.fB, color.fA};
};
```
