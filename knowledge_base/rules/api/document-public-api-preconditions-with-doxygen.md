---
name: skia/api/document-public-api-preconditions-with-doxygen
description: When declaring or porting public API methods in include headers, document parameter constraints and ranges (@param), clamping limits, preconditions, and failure/nullptr conditions
rubric: readability
severity: should_fix
---

**Rule.** Document public API methods in public headers (`include/**`,
`modules/**/include/**`) with Doxygen comments (`/** ... */`) specifying
`@param` constraints (such as valid numeric ranges, clamping limits, and
invalid-argument failure conditions), preconditions, and all failure return
semantics (such as enumerating conditions returning `nullptr` or `false`,
including backend context requirements). Place all API documentation, behavioral
nuances, and caveats directly on the public interface header rather than only in
private headers (`include/private/`, `src/`). Additionally, when porting public
methods to companion or builder classes (such as `SkPathBuilder`), copy the doc
comments so public methods remain documented.

**Why.** Public headers in include/ and modules/**/include/ define Skia's
external API surface and are parsed by Doxygen to generate public documentation.
Documenting constraints, valid ranges, clamping behavior, or failure modes
solely in internal headers or omitting them leaves callers unaware of valid
input domains, null fallbacks, and subtle semantics, causing null pointer
dereferences or unexpected runtime failures.

## Examples

Bad:

```cpp
// 1. Missing parameter constraints, valid ranges, and clamping limits:
/** @param desiredScale Output size scale factor. */
bool queryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes&, float desiredScale,
                   SkYUVAPixmapInfo* yuvaPixmapInfo) const;

// 2. Incompletely documenting failure/nullptr conditions (e.g. backend requirements):
/** return nullptr if the requested ColorInfo is not supported, or dimensions are out of range. */
sk_sp<SkImage> makeScaled(skgpu::graphite::Recorder*, const SkImageInfo&) const;

// 3. Documenting behavioral caveats only on private/internal headers:
// In include/codec/SkCodec.h (public header):
/** Return the HDR metadata in the encoded data. */
const skhdr::Metadata& getHdrMetadata() const;

// 4. Porting public methods to a builder without copying doc comments:
// In include/core/SkPathBuilder.h:
bool isEmpty() const { return fVerbs.empty(); }
```

Good:

```cpp
// 1. Documented parameter constraints, valid numeric ranges, and clamping limits:
/**
 *  @param desiredScale Scale factor for yuvaPixmapInfo. A value <= 0 causes failure.
 *                      The scale is clamped at 1 (upscaling is not supported).
 */
bool queryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes&, float desiredScale,
                   SkYUVAPixmapInfo* yuvaPixmapInfo) const;

// 2. Fully documenting all failure/nullptr and backend requirements:
/**
 * return nullptr if the requested ColorInfo is not supported, dimensions are out of range,
 * or if the recorder is null on a Graphite-backed Image.
 */
sk_sp<SkImage> makeScaled(skgpu::graphite::Recorder*, const SkImageInfo&) const;

// 3. Behavioral caveats and nuances documented on the public interface header:
// In include/codec/SkCodec.h (public header):
/** Return HDR metadata. Note that even SDR images can include HDR metadata. */
const skhdr::Metadata& getHdrMetadata() const;

// 4. Copied doc comments when porting methods to a companion or builder class:
// In include/core/SkPathBuilder.h:
/** Returns true if the path contains no verbs. */
bool isEmpty() const { return fVerbs.empty(); }
```
