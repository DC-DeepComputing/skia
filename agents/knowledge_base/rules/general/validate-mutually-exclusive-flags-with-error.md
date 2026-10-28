---
name: skia/validate-mutually-exclusive-flags-with-error
description: 'When configuring mutually exclusive preprocessor flags or macro options, validate selections with #error compile-time guards instead of silently overriding definitions with #undef'
rubric: readability
severity: should_fix
---

**Rule.** When code requires selecting exactly one option from a set of mutually
exclusive preprocessor configuration flags, validate the configuration at
compile time using #error directives to check that neither or both flags are
defined. Do not use #undef to silently override or reset previous macro
definitions to force an option.

**Why.** Silently overriding conflicting preprocessor flags with #undef masks
broken build configurations and makes macro resolution order-dependent. Explicit
#error checks halt compilation immediately with clear diagnostics whenever
mutually exclusive options collide or required configuration flags are omitted.

## Examples

Bad:

```cpp
#if defined(SK_BLITTER_TRACE_SKVM)
SkBlitterTrace gTrace("VM", false);
#undef STEP_BLITTER_TRACE
#define STEP_BLITTER_TRACE(...) ...
#elif defined(SK_BLITTER_TRACE_RASTER_PIPELINE)
SkBlitterTrace gTrace("RP", false);
#undef STEP_BLITTER_TRACE
#define STEP_BLITTER_TRACE(...) ...
#endif
```

Good:

```cpp
#if !defined(SK_BLITTER_TRACE_SKVM) && !defined(SK_BLITTER_TRACE_RASTER_PIPELINE)
#error "Must define exactly one blitter trace type: SKVM or RASTER_PIPELINE"
#elif defined(SK_BLITTER_TRACE_SKVM) && defined(SK_BLITTER_TRACE_RASTER_PIPELINE)
#error "Cannot define both SK_BLITTER_TRACE_SKVM and SK_BLITTER_TRACE_RASTER_PIPELINE"
#endif
```
