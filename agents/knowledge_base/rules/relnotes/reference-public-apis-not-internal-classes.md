---
name: skia/relnotes/reference-public-apis-not-internal-classes
description: When authoring release notes in relnotes/, describe changes in terms of client-facing capabilities and public APIs rather than internal mechanisms or private classes
rubric: readability
severity: should_fix
---

**Rule.** When authoring release notes (`relnotes/*.md` or `RELEASE_NOTES.md`),
describe additions, improvements, and deprecations in terms of client-facing
capabilities and public APIs (such as `Sk<Format>Decoder` in `include/` or
priority-based log filtering) rather than private internal implementation
classes (such as `Sk<Format>Codec` in `src/`) or internal technical mechanisms
(such as logging macros). Verify that all referenced types and entry points
exist in public headers.

**Why.** Release note snippets under `relnotes/` are aggregated into
`RELEASE_NOTES.md` during milestone releases and published for external clients
and embedders. Referencing private types or internal implementation details
(such as macros or internal class moves) confuses downstream developers who
cannot access those symbols and care about user-visible capabilities and public
APIs.

## Examples

Bad:

```cpp
<!-- relnotes/sk_png_hdr.md -->
Add HDR metadata support to `SkPngCodec`, `SkPngEncoder`, `SkPngRustCodec`, and `SkPngRustEncoder`.

<!-- relnotes/logging.md -->
Logging macros are now supported at a core Skia level. This update includes moving LoggingPriority out of graphite and into core.
```

Good:

```cpp
<!-- relnotes/sk_png_hdr.md -->
Add HDR metadata support to `SkPngDecoder`, `SkPngRustDecoder`, and `SkPngEncoder`.

<!-- relnotes/logging.md -->
Priority-based log filtering is now supported at a core Skia level. This update includes moving LoggingPriority out of Graphite and into core Skia.
```
