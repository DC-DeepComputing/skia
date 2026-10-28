---
name: skia/gpu/document-page-locator-semantics
description: When exposing multi-part buffer or atlas locators with separate page index and offset values, document their page-local scoping and indicate bitpacking plans
rubric: readability
severity: should_fix
---

**Rule.** When returning or storing multi-part resource locators (such as
separate page index and intra-page offset values) in multi-page textures or
buffers, document whether offsets are page-local or global across pages. If
bitpacking the page index into the offset integer is planned, document the
layout or record an attributed TODO.

**Why.** Exposing separate page index and offset fields without documentation
creates ambiguity over whether offsets are page-relative or global buffer
offsets, leaving callers and reviewers uncertain why a unified offset was not
used or whether bitpacking is intended.

## Examples

Bad:

```cpp
// Ambiguous whether fAlphaIndex is page-local or global across the atlas.
struct AlphaAllocation {
    uint8_t* fWritePtr   = nullptr;
    int32_t  fAlphaIndex = 0;
    uint16_t fTexPage    = 0;
};
```

Good:

```cpp
// TODO(author): bitpack the texture page into the alpha index?
// Returns a pair of {alphaIndex, texPage} on success, indicating the page-local byte
// offset and atlas page index, respectively.
std::optional<std::pair<int32_t, uint16_t>> finalizeRun();
```
