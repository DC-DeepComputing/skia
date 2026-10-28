---
name: skia/avoid-size-or-count-suffixes-for-loop-counters
description: When declaring loop counters or iteration indices, avoid 'Size' or 'Count' suffixes; reserve them for aggregates and use 'cur<Item>' or '<item>Index'
rubric: readability
severity: nit
---

**Rule.** Do not name loop counters, iteration indices, or cursor variables with
'Size' or 'Count' suffixes (e.g., 'imagesSize' or 'typefacesSize'); reserve
those suffixes for aggregate capacities, collection totals, or upper bounds. For
loop induction and iteration progress variables, use element-oriented names like
'cur<Item>' (e.g., 'curImage', 'curTypeface'), '<item>Index' (e.g.,
'glyphIndex'), or standard single-letter indices ('i', 'j').

**Why.** The suffixes 'Size' and 'Count' conventionally represent total extents,
capacities, or cardinalities. Naming an iteration counter, subscript, or
progress tracker with 'Size' or 'Count' is misleading because it makes an
individual progressing index appear to be an aggregate upper bound or collection
total, obscuring iteration progress and confusing readers when compared against
the actual limit.

## Examples

Bad:

```cpp
// Bad: Naming loop counters with 'Size' makes them look like collection capacities
for (int imagesSize = 0; imagesSize < imagesCount; ++imagesSize) {
    if (!this->mergeGlyphAndImageFromBuffer(buffer)) {
        return false;
    }
}

// Bad: Hoisted iteration cursor suffixed with 'Size' compared against collection count
int typefacesSize = 0;
for (typefacesSize = 0; typefacesSize < typefaceCount; ++typefacesSize) {
    processTypeface(typefacesSize);
}
```

Good:

```cpp
// Good: Element-oriented 'cur<Item>' identifier for loop induction
for (int curImage = 0; curImage < imagesCount; ++curImage) {
    if (!this->mergeGlyphAndImageFromBuffer(buffer)) {
        return false;
    }
}

// Good: Descriptive cursor identifier or index distinguishing progress from total count
int curTypeface = 0;
for (curTypeface = 0; curTypeface < typefaceCount; ++curTypeface) {
    processTypeface(curTypeface);
}
```
