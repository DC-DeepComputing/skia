---
name: skia/bench/factor-shared-benchmark-datasets
description: When adding large datasets or vector geometries to benchmarks under bench/, factor them into shared benchmark utilities instead of duplicating locally
rubric: design
severity: nit
---

**Rule.** Do not embed duplicate copies of large benchmark datasets or complex
vector path geometries within individual benchmark subdirectories. Factor
reusable benchmark assets and test paths into shared utilities under bench/
(such as bench/BenchmarkDataset.h) so multiple benchmarks can evaluate
performance against standardized workloads.

**Why.** Duplicating large vector path descriptions or benchmark datasets in
localized benchmark folders bloats the repository, risks divergence between
performance tests, and complicates maintenance when updating asset definitions.

## Examples

Bad:

```cpp
// bench/graphite/sparse_strips/TigerSvgUnscaled.h
struct Tiger {
    static constexpr const char* kTigerPathStrings[] = {
        "M27.014 90.57s.039.744-.287.736...",
        // ... hundreds of path strings duplicated in local folder ...
    };
};
```

Good:

```cpp
// bench/BenchmarkDataset.h
namespace skgpu::graphite {
    template <>
    struct BenchmarkDatasetInfo<BenchmarkDataset::kTiger> {
        static std::vector<SkPath> GetPaths();
    };
}
// bench/graphite/sparse_strips/FlattenBench.cpp
#include "bench/BenchmarkDataset.h"
fPaths = skgpu::graphite::BenchmarkDatasetInfo<BenchmarkDataset::kTiger>::GetPaths();
```
