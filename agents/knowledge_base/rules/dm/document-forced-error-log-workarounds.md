---
name: skia/dm/document-forced-error-log-workarounds
description: When using an error log priority or direct SkLog call to force-print non-error output, document the workaround and cite the tracking issue
rubric: readability
severity: should_fix
---

**Rule.** When using an error log priority (such as SkLogPriority::kError) or
calling SkLog directly instead of standard logging macros to force-print
non-error output or bypass macro formatting, document the rationale in an inline
comment. Explain why the unusual log level or unformatted call is required and
reference a tracking issue or bug URL for the underlying limitation.

**Why.** Skia's logging system defaults to filtering messages below kError in
release builds and prepends subsystem prefixes via SKIA_LOG. Using error levels
or bypassing standard macros for informational CLI output appears anomalous;
documenting the workaround and linking the tracking issue clarifies developer
intent and prevents inadvertent reverts or refactoring.

## Examples

Bad:

```cpp
SkLog(SkLogPriority::kError, "Gathered Sources:\n");
for (TaggedSrc& src : *gSrcs) {
    SkLog(SkLogPriority::kError, " - %s %s\n", src.tag.c_str(), src->name().c_str());
}
```

Good:

```cpp
// Using kError to ensure this is always displayed and SkLog (instead of SKIA_LOG)
// to avoid the [skia] prefix. See b/469441457.
SkLog(SkLogPriority::kError, "Gathered Sources:\n");
for (TaggedSrc& src : *gSrcs) {
    SkLog(SkLogPriority::kError, " - %s %s\n", src.tag.c_str(), src->name().c_str());
}
```
