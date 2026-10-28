---
name: skia/gpu/gpu-prefer-skgpu-log-over-skdebugf
description: When adding intentional diagnostic, error, or warning logs in Graphite, use structured logging macros (SKGPU_LOG_* or SKIA_LOG_*) rather than SkDebugf or SkDEBUGFAIL
rubric: readability
severity: should_fix
---

**Rule.** In Graphite, emit intentional diagnostic, error, and warning messages
using structured logging macros (SKGPU_LOG_* or SKIA_LOG_*, such as SKGPU_LOG_W
/ SKIA_LOG_W) instead of unstructured SkDebugf or SkDEBUGFAIL. For unexpected or
out-of-sync conditions that have a safe, recoverable fallback, log at the
warning level (SKGPU_LOG_W / SKIA_LOG_W).

**Why.** SKIA_LOG and SKGPU_LOG_* route messages through Skia's
priority-filtered logging system, attach subsystem tags, and allow clients to
intercept diagnostics via SkLogHandler. In contrast, SkDebugf and SkDEBUGFAIL
bypass priority thresholds and log handlers, producing untagged and unfilterable
console output.

## Examples

Bad:

```cpp
if (!fShapeCache.find(key)) {
    SkDebugf("Evicted entry not in cache!");
    return;
}
```

Good:

```cpp
if (!fShapeCache.find(key)) {
    SKGPU_LOG_W("Evicted entry not in cache!");
    return;
}
```
