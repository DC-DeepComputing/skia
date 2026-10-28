---
name: skia/api/keep-public-pipeline-operations-generic
description: When declaring public API methods, keep operation names generic rather than exposing internal execution or pipeline distinctions like graphics versus compute
rubric: readability
severity: should_fix
---

**Rule.** Do not expose internal execution or pipeline distinctions (such as
graphics vs. compute pipelines) in public API method names; keep client-facing
operations generic (e.g., `purgePipelines...` instead of
`purgeGraphicsPipelines...`). Name operations after high-level client intent
rather than backend execution mechanisms, even if the current implementation
only applies to one pipeline category.

**Why.** Whether client operations map to graphics or compute pipelines is an
internal implementation detail of Skia's render backends. Exposing execution
categories in public APIs leaks internal implementation details, burdens callers
with backend knowledge, and causes unnecessary API churn if backend execution
strategies evolve.

## Examples

Bad:

```cpp
// In include/gpu/graphite/PrecompileContext.h
class SK_API PrecompileContext {
public:
    void purgeGraphicsPipelinesNotUsedInMs(std::chrono::milliseconds msNotUsed);
};
```

Good:

```cpp
// In include/gpu/graphite/PrecompileContext.h
class SK_API PrecompileContext {
public:
    void purgePipelinesNotUsedInMs(std::chrono::milliseconds msNotUsed);
};
```
