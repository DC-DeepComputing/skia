---
name: skia/gpu/log-unsupported-backend-features
description: When handling unsupported backend features or invalid execution branches, log a descriptive error with SKIA_LOG_E rather than bare SkASSERT(false)
rubric: readability
severity: should_fix
---

**Rule.** When handling unsupported backend features, unhandled command types,
or invalid execution branches in GPU code, log a descriptive error message using
subsystem logging macros (`SKIA_LOG_E` or `SKGPU_LOG_E`) rather than relying on
a bare `SkASSERT(false)`. Explicitly state the backend component and the
unsupported feature or operation in the log message.

**Why.** `SkASSERT(false)` provides no descriptive context and is compiled out
completely in release builds, silently ignoring invalid states. Using an
explicit error logging macro ensures an informative, actionable message appears
in production and test logs when unsupported operations occur.

## Examples

Bad:

```cpp
case DrawPassCommands::Type::kAddBarrier: {
    // Graphite/Metal does not support barriers.
    SkASSERT(false);
    break;
}
```

Good:

```cpp
case DrawPassCommands::Type::kAddBarrier: {
    SKIA_LOG_E("MtlCommandBuffer does not support the addition of barriers.");
    break;
}
```
