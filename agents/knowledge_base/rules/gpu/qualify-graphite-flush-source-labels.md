---
name: skia/gpu/qualify-graphite-flush-source-labels
description: When specifying diagnostic flush source labels in Graphite task dumping, qualify the label string with the enclosing ClassName::methodName to prevent ambiguous dump output
rubric: readability
severity: should_fix
---

**Rule.** When specifying diagnostic flush source labels in Graphite (such as
arguments to flushTrackedDevices(SK_DUMP_TASKS_CODE(...))), qualify the label
string with the enclosing class and method name (e.g., ClassName::methodName or
ClassName::methodName: Description). Do not pass unqualified descriptions or
bare operation names without the initiating class and method prefix.

**Why.** Graphite task dumping prints flush tokens alongside their flush source
strings to help trace GPU command scheduling. Unqualified labels are ambiguous
and inconsistent across sibling classes like Device and Recorder, making it
difficult to identify which component and method initiated the flush.

## Examples

Bad:

```cpp
this->priv().flushTrackedDevices(SK_DUMP_TASKS_CODE("Update Backend Texture"));
```

Good:

```cpp
this->priv().flushTrackedDevices(
        SK_DUMP_TASKS_CODE("Recorder::updateBackendTexture: Update Backend Texture"));
```
