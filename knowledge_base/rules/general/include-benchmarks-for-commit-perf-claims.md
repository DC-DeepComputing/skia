---
name: skia/include-benchmarks-for-commit-perf-claims
description: When claiming performance improvements in commit messages, substantiate speedup claims with before-and-after benchmark numbers and include corresponding microbenchmarks under bench/
rubric: readability
severity: should_fix
---

**Rule.** When claiming a performance optimization, speedup (e.g., 'faster'), or
algorithmic improvement in a commit message, substantiate the claim with
before/after benchmark measurements (e.g., nanobench numbers). In addition,
include the corresponding benchmark implementations under `bench/` (and register
them in `gn/bench.gni`) using `DEF_BENCH` and nanobench conventions so results
are verifiable and monitored for regressions on Skia Perf.

**Why.** Performance claims must be backed by empirical evidence so reviewers
and maintainers can evaluate whether the optimization achieves its intended
speedup and justifies any added complexity. Including the underlying benchmark
implementations in the repository ensures performance gains are reproducible and
allows continuous integration on Skia Perf (perf.skia.org) to guard against
regressions.

## Examples

Bad:

```cpp
// Unsubstantiated performance claim in commit message without benchmark numbers:
Offer (much) faster version of mapping 1 point

- new benchmark for single-point mappers
- add linked single-point mappers (w/ and w/o perspective check)

// Quoting benchmark numbers in commit message but omitting benchmark source from CL:
Optimize PathIterator traversal by caching contour endpoints.

nanobench results:
  min     median  config        bench
  2.53µs  2.54µs  nonrendering  pathiter_pathiter
  3.09µs  3.09µs  nonrendering  pathiter_olditer
// (CL modifies src/core/SkPath.cpp but adds no benchmark code under bench/)
```

Good:

```cpp
// Commit message substantiates speedup claim with before/after numbers:
Offer faster version of mapping 1 point

- new benchmark for single-point mappers
- add linked single-point mappers (w/ and w/o perspective check)

Benchmark results (nanobench on Linux):
mappt_p_identity: 12.3ns -> 4.1ns (~3x faster)
mappt_p_affine:   24.5ns -> 8.2ns (~3x faster)

// Corresponding benchmark implementation is included in bench/ and gn/bench.gni:
class PathIterBench : public Benchmark {
    const char* onGetName() override { return "pathiter_pathiter"; }
    void onDraw(int loops, SkCanvas*) override { /* ... */ }
};
DEF_BENCH(return new PathIterBench();)
```
