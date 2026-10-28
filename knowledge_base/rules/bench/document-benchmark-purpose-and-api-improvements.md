---
name: skia/bench/document-benchmark-purpose-and-api-improvements
description: When adding or modifying microbenchmarks under bench/ that evaluate container or builder performance tradeoffs, document the measured scenario and note potential API enhancements or optimization knobs
rubric: readability
severity: nit
---

**Rule.** When introducing or modifying microbenchmarks under `bench/` that
evaluate container or builder performance tradeoffs, document the scenario being
measured in a comment above the benchmark class. Include breadcrumbs or TODO
notes identifying potential API enhancements or optimization knobs (such as
exposing explicit storage reservation or allocation growth controls on helper
methods).

**Why.** Microbenchmarks often expose API design tradeoffs or missing
optimization parameters discovered during performance analysis. Documenting what
the benchmark isolates and recording potential follow-up API improvements
provides crucial context for future maintainers investigating regressions or
refining those APIs.

## Examples

Bad:

```cpp
// Lacks explanation of what usage scenario is being measured or follow-up opportunities
class PathBuilderManyShapes final : public Benchmark {
public:
    PathBuilderManyShapes(const char* suffix, Proc proc) : fSuffix(suffix), fProc(proc) {}
    ...
};
```

Good:

```cpp
// Measures SkPathBuilder perf when appending a large number of simple shapes (addRect etc).
// Since internally we know how to reserve exactly or with growth, might consider exposing
// that option in these helpers: addRect, addOval, etc.
class PathBuilderManyShapes final : public Benchmark {
public:
    PathBuilderManyShapes(const char* suffix, Proc proc) : fSuffix(suffix), fProc(proc) {}
    ...
};
```
