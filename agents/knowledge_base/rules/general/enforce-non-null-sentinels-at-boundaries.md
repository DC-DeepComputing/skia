---
name: skia/enforce-non-null-sentinels-at-boundaries
description: When classes support default or empty fallback states, normalize null pointers to empty sentinels in constructors and setters rather than exposing fallback accessors
rubric: design
severity: should_fix
---

**Rule.** When a class member supports a default or empty representation (such
as an empty sentinel like SkTypeface::MakeEmpty() or a null object), normalize
null inputs to the sentinel upfront inside constructors and setters. Do not
retain a nullable member and introduce fallback accessors (such as
getOrDefault() or refOrDefault()) or repeat ternary null checks across method
use sites.

**Why.** Enforcing the non-null invariant at class boundaries ensures internal
fields are never null throughout the object's lifetime. This eliminates
redundant defensive null branching, allows public and private getters to
guarantee non-null returns without fallback wrapper methods, and keeps method
implementations simple and direct.

## Examples

Bad:

```cpp
sk_sp<SkTypeface> SkFont::refTypefaceOrDefault() const {
    return fTypeface ? fTypeface : SkTypeface::MakeEmpty();
}

SkGlyphID SkFont::unicharToGlyph(SkUnichar uni) const {
    return this->refTypefaceOrDefault()->unicharToGlyph(uni);
}
```

Good:

```cpp
void SkFont::setTypeface(sk_sp<SkTypeface> tf) {
    fTypeface = tf ? std::move(tf) : SkTypeface::MakeEmpty();
}

SkGlyphID SkFont::unicharToGlyph(SkUnichar uni) const {
    return this->getTypeface()->unicharToGlyph(uni);
}
```
