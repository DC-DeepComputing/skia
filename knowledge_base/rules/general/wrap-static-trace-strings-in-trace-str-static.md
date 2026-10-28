---
name: skia/wrap-static-trace-strings-in-trace-str-static
description: When passing non-literal strings to TRACE_EVENT macros, wrap dynamic strings in TRACE_STR_COPY and non-literal static strings in TRACE_STR_STATIC
rubric: safety
severity: blocking
---

**Rule.** When passing string event names or string argument values to
TRACE_EVENT* macros, wrap dynamic or short-lived strings (such as std::string or
SkString) in TRACE_STR_COPY(...) to ensure the trace buffer copies the string.
When passing non-literal static string expressions (such as ternary operators
selecting string literals or static string accessors), wrap them in
TRACE_STR_STATIC(...) rather than duplicating code across if/else branches.

**Why.** TRACE_EVENT* macros and tracing backends (such as Perfetto) store raw
const char* pointers by default under the assumption of application lifetime;
omitting TRACE_STR_COPY on dynamic strings causes use-after-free reads or
corrupted trace data when serialized. For static strings, Perfetto enforces
compile-time static lifetime assertions, which TRACE_STR_STATIC satisfies
without duplicating surrounding trace calls.

## Examples

Bad:

```cpp
// Bad: Passing a dynamic string without TRACE_STR_COPY risks use-after-free
TRACE_EVENT_INSTANT1_ALWAYS("skia.shaders",
                            pipeline->fLabel.c_str(),
                            TRACE_EVENT_SCOPE_THREAD,
                            "area", area);

// Bad: Duplicating code to pass literal strings instead of TRACE_STR_STATIC
if (supportsCacheControl) {
    TRACE_EVENT0_ALWAYS("skia.shaders", "CompileAfterMiss");
    CompilePipeline();
} else {
    TRACE_EVENT0_ALWAYS("skia.shaders", "CacheLookupOrCompile");
    CompilePipeline();
}
```

Good:

```cpp
// Good: Wrap dynamic strings in TRACE_STR_COPY to force buffer copying
TRACE_EVENT_INSTANT1_ALWAYS("skia.shaders",
                            TRACE_STR_COPY(pipeline->fLabel.c_str()),
                            TRACE_EVENT_SCOPE_THREAD,
                            "area", area);

// Good: Wrap non-literal static expressions in TRACE_STR_STATIC
TRACE_EVENT0_ALWAYS("skia.shaders",
                    TRACE_STR_STATIC(supportsCacheControl ? "CompileAfterMiss"
                                                          : "CacheLookupOrCompile"));
CompilePipeline();
```
