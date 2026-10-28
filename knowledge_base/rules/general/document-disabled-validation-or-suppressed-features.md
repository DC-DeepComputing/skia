---
name: skia/document-disabled-validation-or-suppressed-features
description: When configuring codecs, parsers, or external library wrappers to disable strict validation or ignore features, document the compatibility or performance rationale in explanatory comments
rubric: readability
severity: nit
---

**Rule.** When configuring codecs, parsers, or third-party library wrappers to
disable strict specification compliance, bypass validation checks, or ignore
metadata and features (such as disabling strict mode or ignoring Exif/XMP data),
document the technical rationale in an explanatory comment. Explicitly explain
whether the override is an intentional compatibility workaround for real-world
content, a performance optimization, or a temporary omission that requires an
attributed TODO comment.

**Why.** Disabling strict specification validation or ignoring metadata without
explanation makes configurations appear arbitrary, buggy, or incomplete.
Documenting the underlying motivation—such as avoiding I/O stalls on streaming
metadata or accommodating non-compliant files found in the wild—preserves design
intent and prevents maintainers from accidentally re-enabling strictness and
breaking valid workloads.

## Examples

Bad:

```cpp
// Undocumented configuration overrides disabling strictness and metadata
avifDecoder->ignoreXMP = crabbyavif::CRABBY_AVIF_TRUE;
avifDecoder->ignoreExif = crabbyavif::CRABBY_AVIF_TRUE;
avifDecoder->strictFlags = crabbyavif::AVIF_STRICT_DISABLED;
```

Good:

```cpp
// Ignore XMP and Exif to ensure that avifDecoderParse() isn't waiting for
// some tiny Exif payload hiding at the end of a file.
avifDecoder->ignoreXMP = crabbyavif::CRABBY_AVIF_TRUE;
avifDecoder->ignoreExif = crabbyavif::CRABBY_AVIF_TRUE;

// Disable strict mode to allow AVIF files in the wild that are technically
// invalid according to the specification but can be safely decoded.
avifDecoder->strictFlags = crabbyavif::AVIF_STRICT_DISABLED;
```
