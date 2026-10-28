---
name: skia/unconditional-signatures-for-logging-metadata
description: When adding optional logging, tracing, or diagnostics, keep signatures, fields, and call sites unconditional by encapsulating preprocessor guards inside helper bodies
rubric: readability
severity: should_fix
---

**Rule.** Keep interface signatures, class fields, and call sites unconditional
when adding optional logging, tracing, or debugging instrumentation. Do not wrap
method parameters or struct/class fields in #if preprocessor guards when
overhead is negligible. Furthermore, encapsulate conditional compilation guards
(#if / #ifdef) inside helper function bodies or provide empty inline stubs
rather than wrapping helper definitions and call sites in preprocessor checks.

**Why.** Guarding method parameters and object fields with preprocessor blocks
clutters declarations and causes divergent API surfaces across build
configurations. Similarly, scattering #if checks across call sites creates
boilerplate; encapsulating guards inside helper bodies allows unconditional
calls that the compiler optimizes away when disabled.

## Examples

Bad:

```cpp
// In header/interface:
sk_sp<GraphicsPipeline> findGraphicsPipeline(
    const UniqueKey& key
#if defined(SK_PIPELINE_LIFETIME_LOGGING)
    , SkEnumBitMask<PipelineCreationFlags> flags = PipelineCreationFlags::kNone
#endif
);

// In implementation and call site:
#if SK_HISTOGRAMS_ENABLED
void log_pipeline_creation(const AsyncPipelineCreationBase* apcb) { ... }
#endif
#if SK_HISTOGRAMS_ENABLED
log_pipeline_creation(asyncCreationPtr);
#endif
```

Good:

```cpp
// In header/interface:
sk_sp<GraphicsPipeline> findGraphicsPipeline(
    const UniqueKey& key,
    SkEnumBitMask<PipelineCreationFlags> flags = PipelineCreationFlags::kNone);

// In implementation and call site:
void log_pipeline_creation(const AsyncPipelineCreationBase* apcb) {
#if SK_HISTOGRAMS_ENABLED
    ...
#endif
}
log_pipeline_creation(asyncCreationPtr);
```
