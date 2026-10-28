---
name: skia/api/keep-descriptor-and-info-classes-immutable
description: When extending immutable descriptor or info classes like SkEncodedInfo, supply new fields via Make() factory methods rather than mutating setters
rubric: readability
severity: should_fix
---

**Rule.** Do not add mutating setter methods to descriptor or info classes (such
as SkEncodedInfo or SkImageInfo) designed as immutable value types. Supply any
new or modified fields through constructor parameters, static Make() factory
functions, or functional transformation helpers (such as makeWH()) instead.

**Why.** Descriptor and info classes represent snapshot metadata and are
designed as immutable value objects across Skia. Introducing mutating setters
breaks this immutability invariant, invites subtle race conditions or unexpected
state mutations across shared references, and creates inconsistent class
interfaces.

## Examples

Bad:

```cpp
class SkEncodedInfo {
public:
    void setHdrMetadata(const skhdr::Metadata& hdrMetadata) {
        fHdrMetadata = hdrMetadata;
    }
    const skhdr::Metadata& getHdrMetadata() const { return fHdrMetadata; }
};
```

Good:

```cpp
class SkEncodedInfo {
public:
    static SkEncodedInfo Make(int width, int height, Color color, Alpha alpha,
                              uint8_t bitsPerComponent,
                              std::unique_ptr<SkCodecs::ColorProfile> profile,
                              const skhdr::Metadata& hdrMetadata);
    const skhdr::Metadata& getHdrMetadata() const { return fHdrMetadata; }
};
```
