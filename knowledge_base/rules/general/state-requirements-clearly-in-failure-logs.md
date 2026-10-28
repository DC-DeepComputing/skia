---
name: skia/state-requirements-clearly-in-failure-logs
description: When writing failure or diagnostic log messages, explain preconditions in clear prose rather than cryptic tags, and differentiate messages across method variants
rubric: readability
severity: should_fix
---

**Rule.** When logging failure, error, or diagnostic messages (such as via
SkDebugf, SKIA_LOG_E, or SKGPU_LOG_E), explain requirements and failure reasons
in clear, descriptive prose rather than appending cryptic parameter identifiers,
variable names, or shorthand tags in parentheses. Additionally, when logging
from overloaded or specialized function variants (such as compressed versus
uncompressed paths), differentiate the error messages so diagnostics clearly
identify which variant failed.

**Why.** Appending raw parameter names or shorthand flags in parentheses is
ambiguous, while identical log messages across method variants obscure which
execution path or data format failed. Phrasing preconditions in clear prose and
distinguishing variant operations provides immediately actionable context for
diagnosing issues without cross-referencing call sites.

## Examples

Bad:

```cpp
// Sibling pattern 1: Cryptic parameter tags in parentheses
SkDebugf("%s failed due not finding a valid backend format for %" PRIu32
         " (requireKnownFormat)", __func__, bufferDesc.format);

// Sibling pattern 2: Undifferentiated message in variant method
// In Recorder::updateCompressedBackendTexture
if (!uploadSource.isValid()) {
    SKGPU_LOG_E("Recorder::updateBackendTexture: Could not create UploadSource");
    return false;
}
```

Good:

```cpp
// Sibling pattern 1: Clear prose explaining requirement
SkDebugf("%s requires a known format, but failed due not finding a valid backend format for"
         " %" PRIu32 ".", __func__, bufferDesc.format);

// Sibling pattern 2: Differentiated message identifying variant
// In Recorder::updateCompressedBackendTexture
if (!uploadSource.isValid()) {
    SKGPU_LOG_E("Recorder::updateBackendTexture: Could not create compressed UploadSource");
    return false;
}
```
