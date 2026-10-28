---
name: skia/infra/prefer-clang-over-msvc-for-perf-bots
description: When defining or updating performance benchmark bots (Perf-*) in infra/bots, use Clang instead of MSVC, reserving MSVC for build and correctness testing
rubric: safety
severity: nit
---

**Rule.** When defining or modifying performance benchmark tasks (Perf-*) in
infra/bots/ (such as jobs.json or tasks.json), configure them to build with
Clang rather than MSVC. Reserve MSVC and other secondary compilers for build and
correctness testing (Test-* or Build-*), and track continuous performance
regressions only on Clang toolchains.

**Why.** Skia's core routines and downstream embedders like Chromium are
explicitly optimized for Clang. MSVC frequently exhibits compiler regressions
and CPU-side overhead even on GPU configurations, which creates noisy and
misleading performance alerts on Skia Perf.

## Examples

Bad:

```cpp
"name": "Perf-Win11-MSVC-Golo-GPU-QuadroP400-x86_64-Release-All-Graphite_Dawn_D3D11"
```

Good:

```cpp
"name": "Perf-Win11-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-Graphite_Dawn_D3D11"
```
