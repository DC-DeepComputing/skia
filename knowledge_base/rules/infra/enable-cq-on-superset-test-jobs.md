---
name: skia/infra/enable-cq-on-superset-test-jobs
description: When adding a comprehensive CI test job in infra/bots/jobs.json that supersedes narrower feature jobs, enable cq_config on the superset job and remove it from subset jobs
rubric: testing
severity: should_fix
---

**Rule.** When adding a comprehensive or superset CI test job in
infra/bots/jobs.json that subsumes narrower feature-specific jobs, enable Commit
Queue execution on the superset job (e.g., "cq_config": {}) and remove
"cq_config" from the redundant subset jobs. Retain the subset job definitions
for manual or post-submit runs if needed, but do not execute both on CQ.

**Why.** Enabling "cq_config" on the superset job ensures complete pre-submit
coverage across all features of the subsystem. Removing CQ execution from
redundant subset jobs prevents duplicate testing, conserving CI bot capacity and
avoiding unnecessary increases in CQ cycle times.

## Examples

Bad:

```cpp
  {
    "name": "Test-Ubuntu24.04-Clang-GCE-CPU-AVX2-x86_64-Debug-All-RustALL"
  },
  {
    "name": "Test-Ubuntu24.04-Clang-GCE-CPU-AVX2-x86_64-Debug-All-RustPNG",
    "cq_config": {}
  }
```

Good:

```cpp
  {
    "name": "Test-Ubuntu24.04-Clang-GCE-CPU-AVX2-x86_64-Debug-All-RustALL",
    "cq_config": {}
  },
  {
    "name": "Test-Ubuntu24.04-Clang-GCE-CPU-AVX2-x86_64-Debug-All-RustPNG"
  }
```
