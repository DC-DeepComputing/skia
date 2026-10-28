---
name: skia/null-check-stream-and-font-factories
description: When instantiating streams, typefaces, or font proxies via MakeFromFile or MakeFromStream factories, check return values for nullptr before dereferencing them
rubric: correctness
severity: blocking
---

**Rule.** When opening streams or instantiating typefaces, codecs, or font
proxies via factory methods (such as SkStream::MakeFromFile,
SkFontScanner::MakeFromStream, or SkTypeface::MakeFromStream), verify that both
the stream and the resulting object are non-null before dereferencing them.
Handle null results gracefully by skipping the entry or returning early instead
of assuming decoding succeeded.

**Why.** SkStream::MakeFromFile returns nullptr if a file cannot be opened or
does not exist, and stream decoding factories return nullptr when stream
contents do not contain valid font or resource data. Dereferencing returned
pointers without checking causes null pointer dereferences.

## Examples

Bad:

```cpp
auto stream = SkStream::MakeFromFile(pathName.c_str());
auto proxy = scanner->MakeFromStream(std::move(stream), args);
SkString familyName;
proxy->getFamilyName(&familyName);
```

Good:

```cpp
std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(pathName.c_str());
if (!stream) {
    SkDEBUGF("Requested font file %s cannot be opened.\n", pathName.c_str());
    return nullptr;
}
auto proxy = scanner->MakeFromStream(std::move(stream), args);
if (!proxy) {
    SkDEBUGF("Requested font file %s does not contain valid font data.\n", pathName.c_str());
    return nullptr;
}
SkString familyName;
proxy->getFamilyName(&familyName);
```
