---
name: skia/distinguish-transformed-variables-with-qualifiers
description: When adapting or transforming variables or parameters in the same scope, use descriptive qualifying prefixes rather than subtle abbreviation variations
rubric: readability
severity: nit
---

**Rule.** When adapting, wrapping, or transforming an input parameter or
existing variable in the same scope, do not use subtle abbreviation variations
(such as 'txt' alongside 'text' or 'rect' alongside 'rct') to distinguish them.
Name the transformed variable with a descriptive qualifying prefix or
role-specific identifier (such as 'adjText', 'adjustedText', or 'origText') that
clearly communicates its representation.

**Why.** Subtle abbreviation variations between related variables in the same
scope look visually almost identical, confusing readers about which
representation or wrapper is being referenced at call sites. Explicit qualifying
prefixes or distinct domain names clearly indicate data flow and prevent subtle
mistakes.

## Examples

Bad:

```cpp
Shaper::Result Shaper::Shape(const SkString& text, const TextDesc& desc, const SkRect& box,
                             const sk_sp<SkFontMgr>& fontmgr) {
    // Bad: 'txt' is a subtle abbreviation variation of parameter 'text', obscuring the distinction
    const AdjustedText txt(text, desc);
    return DoShape(txt, desc, box, fontmgr);
}
```

Good:

```cpp
Shaper::Result Shaper::Shape(const SkString& text, const TextDesc& desc, const SkRect& box,
                             const sk_sp<SkFontMgr>& fontmgr) {
    // Good: Descriptive qualifier 'adjText' clearly communicates the transformed representation
    const AdjustedText adjText(text, desc);
    return DoShape(adjText, desc, box, fontmgr);
}
```
