---
name: skia/document-diagnostic-and-tracking-members
description: When declaring diagnostic, debugging, or state-tracking member variables and helper methods in headers, document what data they record and where it originates
rubric: readability
severity: nit
---

**Rule.** When declaring internal diagnostic, debugging, or state-tracking
member variables and helper methods in class headers—especially when
conditionally compiled under debug macros such as #if defined(SK_DUMP_TASKS),
GPU_TEST_UTILS, or SK_DEBUG—document what data they record, where that data
originates, and how it is consumed. Add concise doc comments clarifying the
source of logged call sites, tokens, or debug metrics and the role of diagnostic
dump helpers.

**Why.** Diagnostic and tracking fields often record non-obvious state like
call-site identifiers, flush counts, or token histories that are populated
across distant call sites. Explanatory comments clarify the purpose and
lifecycle of debug instrumentation, preventing confusion for maintainers
inspecting class layouts or diagnostic dump output.

## Examples

Bad:

```cpp
#if defined(SK_DUMP_TASKS)
    void dumpTasks(TaskList*) const;

    SkTDArray<const char*> fFlushSources;
#endif
```

Good:

```cpp
#if defined(SK_DUMP_TASKS)
    // Traverses and dumps the task list at Recorder::snap().
    void dumpTasks(TaskList*) const;

    // Log of all callers of RecorderPriv::flushTrackedDevices.
    SkTDArray<const char*> fFlushSources;
#endif
```
