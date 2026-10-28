---
name: skia/avoid-string-allocation-in-trace-loops
description: When recording trace events inside loops or hot paths, avoid dynamic string allocations and emit shared context in an enclosing trace scope instead
rubric: safety
severity: should_fix
---

**Rule.** Avoid dynamic string allocations, formatting, or concatenation (such
as std::string addition or SkStringPrintf) inside loops or per-element trace
events. Restructure trace calls hierarchically by emitting shared contextual
labels in an enclosing trace scope (e.g., TRACE_EVENT0), or precompute and cache
static string descriptors before the loop.

**Why.** Dynamic string allocations and heap formatting inside frequent or
looped trace events introduce significant CPU overhead that artificially
inflates execution times and distorts profiling results. Emitting shared context
once in an outer trace scope keeps inner trace points lightweight while
preserving full diagnostic context.

## Examples

Bad:

```cpp
for (int i = 0; i < pipelines.size(); ++i) {
    TRACE_EVENT_INSTANT1_ALWAYS(
            "skia.shaders",
            TRACE_STR_COPY((desc.toLabel() + " + " + pipelines[i]->label()).c_str()),
            TRACE_EVENT_SCOPE_THREAD,
            "area", drawAreas[i]);
}
```

Good:

```cpp
TRACE_EVENT0_ALWAYS("skia.shaders", TRACE_STR_COPY(desc.toLabel().c_str()));
for (int i = 0; i < pipelines.size(); ++i) {
    TRACE_EVENT_INSTANT1_ALWAYS(
            "skia.shaders",
            TRACE_STR_COPY(pipelines[i]->label().c_str()),
            TRACE_EVENT_SCOPE_THREAD,
            "area", drawAreas[i]);
}
```
