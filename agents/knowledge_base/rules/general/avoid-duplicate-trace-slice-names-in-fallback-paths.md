---
name: skia/avoid-duplicate-trace-slice-names-in-fallback-paths
description: When instrumenting fallback or retry paths with TRACE_EVENT, avoid emitting duplicate slice names; wrap the operation in an outer scope with distinct nested slices
rubric: testing
severity: should_fix
---

**Rule.** Do not emit consecutive TRACE_EVENT slices with identical names when
an operation falls back, retries, or takes a secondary compilation path.
Instead, wrap the overall logical operation in an outer trace slice (e.g., using
TRACE_EVENT0 or TRACE_FUNC) and record a distinct, descriptive nested slice
(such as '...-CompileRequired' or '...-Fallback') for the secondary attempt.

**Why.** Emitting duplicate sequential trace event names for an initial failed
or partial attempt and its subsequent retry doubles event counts and distorts
average duration metrics on performance dashboards. Wrapping the full operation
in an outer scope accurately captures end-to-end latency, while distinct nested
slices clearly attribute the overhead of the fallback path in trace viewers.

## Examples

Bad:

```cpp
{
    TRACE_EVENT0_ALWAYS("skia.shaders", "CreateGraphicsPipeline");
    GR_VK_CALL_RESULT(gpu, err, CreateGraphicsPipelines(...));
}
if (err == VK_PIPELINE_COMPILE_REQUIRED) {
    {
        TRACE_EVENT0_ALWAYS("skia.shaders", "CreateGraphicsPipeline");
        GR_VK_CALL_RESULT(gpu, err, CreateGraphicsPipelines(...));
    }
}
```

Good:

```cpp
{
    TRACE_EVENT0_ALWAYS("skia.shaders", "CreateGraphicsPipeline");
    if (gpu->vkCaps().supportsPipelineCreationCacheControl()) {
        GR_VK_CALL_RESULT_NOCHECK(gpu, err, CreateGraphicsPipelines(...));
    }
    if (err == VK_PIPELINE_COMPILE_REQUIRED) {
        TRACE_EVENT0_ALWAYS("skia.shaders", "CreateGraphicsPipeline-CompileRequired");
        GR_VK_CALL_RESULT(gpu, err, CreateGraphicsPipelines(...));
    }
}
```
