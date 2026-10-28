---
name: skia/api/document-sdr-hdr-metadata-semantics
description: When declaring codec HDR metadata accessors or encoder options structs, document that SDR images can carry metadata and describe value-type containers as empty when omitted
rubric: readability
severity: should_fix
---

**Rule.** When declaring or updating HDR metadata accessors or configuration
structs on codecs and encoders (such as getHdrMetadata() or fHdrMetadata),
document both the metadata's scope and representation. On codec accessors,
explicitly document that SDR images can also carry HDR metadata (e.g.,
specifying inverse tone mapping on HDR displays). In encoder options structs
using value-type containers (such as skhdr::Metadata) alongside pointer fields
that use nullptr to signal absence, document that the field is a container for
any metadata to include and is empty when omitting HDR metadata or encoding SDR.

**Why.** Callers and downstream maintainers often assume HDR metadata is
exclusive to HDR images, or expect absent features in options structs to be
represented by nullptr. Explicitly documenting SDR tone-mapping capabilities and
container empty-state semantics prevents incorrect assumptions in decoding
pipelines and clarifies how callers omit or configure metadata.

## Examples

Bad:

```cpp
// Bad: Omits SDR metadata capabilities and container empty semantics
const skhdr::Metadata& getHdrMetadata() const;

struct Options {
    /**
     * HDR metadata to include in the encoded image.
     */
    skhdr::Metadata fHdrMetadata;
    const SkPixmap* fGainmap = nullptr;
};
```

Good:

```cpp
// Good: Clarifies SDR metadata role and container empty-state semantics
/**
 * Return the HDR metadata associated with this image. Note that even SDR images
 * can include HDR metadata (e.g., indicating how to inverse tone map on an HDR display).
 */
const skhdr::Metadata& getHdrMetadata() const;

struct Options {
    /**
     * Container for any HDR metadata to include in the encoded image; empty if omitted.
     */
    skhdr::Metadata fHdrMetadata;
    const SkPixmap* fGainmap = nullptr;
};
```
