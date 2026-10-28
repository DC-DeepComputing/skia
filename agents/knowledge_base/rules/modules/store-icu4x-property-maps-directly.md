---
name: skia/modules/store-icu4x-property-maps-directly
description: When checking Unicode code point properties in ICU4X, store ICU4XCodePointMapData8 directly and compare property values instead of caching separate ICU4XCodePointSetData sets
rubric: safety
severity: should_fix
---

**Rule.** When checking whether Unicode code points match specific property
values in ICU4X, store the underlying ICU4XCodePointMapData8 directly and query
property values via map.get(cp) against target constants. Do not derive, cache,
or query multiple separate ICU4XCodePointSetData instances via
get_set_for_value().

**Why.** Deriving and storing separate ICU4XCodePointSetData instances for each
property value increases object layout size and memory overhead, while querying
multiple sets requires sequential contains() operations. Storing the underlying
ICU4XCodePointMapData8 resolves the character's property in a single map lookup
compared against enum or integer constants.

## Examples

Bad:

```cpp
const auto lineBreaks = ICU4XCodePointMapData8::load_line_break(fDataProvider).ok().value();
fMandatoryBreak = lineBreaks.get_set_for_value(/*MandatoryBreak*/6);
fCarriageReturn = lineBreaks.get_set_for_value(/*CarriageReturn*/10);
fLineFeed = lineBreaks.get_set_for_value(/*LineFeed*/17);
fNextLine = lineBreaks.get_set_for_value(/*NextLine*/29);

bool isHardBreak(SkUnichar utf8) override {
    return fMandatoryBreak.contains(utf8) ||
           fCarriageReturn.contains(utf8) ||
           fLineFeed.contains(utf8) ||
           fNextLine.contains(utf8);
}
```

Good:

```cpp
fLineBreaks = ICU4XCodePointMapData8::load_line_break(fDataProvider).ok().value();

bool isHardBreak(SkUnichar utf8) override {
    auto value = fLineBreaks.get(utf8);
    return (value == /*MandatoryBreak*/6) ||
           (value == /*CarriageReturn*/10) ||
           (value == /*LineFeed*/17) ||
           (value == /*NextLine*/29);
}
```
