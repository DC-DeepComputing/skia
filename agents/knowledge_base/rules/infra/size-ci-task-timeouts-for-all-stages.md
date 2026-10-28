---
name: skia/infra/size-ci-task-timeouts-for-all-stages
description: When adjusting or configuring CI task execution timeouts in infra/bots, size the timeout to include sufficient headroom for all sequential stages and runtime variance
rubric: correctness
severity: should_fix
---

**Rule.** When defining or adjusting task execution timeouts (such as
b.timeout(...) in task generators or ExecutionTimeout in task specs) for
multi-step CI jobs, size the timeout with sufficient headroom to cover all
sequential stages and runtime variance. Do not budget solely for the longest or
failing phase without accounting for subsequent steps (such as artifact
generation or uploads) and peak load variance.

**Why.** Multi-step CI tasks execute sequential phases (such as building
dependencies followed by generating or isolating artifacts). Setting a tight
timeout that barely accommodates the longest phase risks flaky failures or
recurring timeouts when subsequent steps execute or bot performance varies under
load.

## Examples

Bad:

```cpp
// In infra/bots/gen_tasks_logic/gen_tasks_logic.go
// Chrome build takes ~6 hours; subsequent SKP generation takes 30-40 minutes.
b.timeout(7 * time.Hour) // Insufficient headroom for subsequent steps and runtime variance
```

Good:

```cpp
// In infra/bots/gen_tasks_logic/gen_tasks_logic.go
// Provides sufficient headroom for build, SKP generation, and execution variance.
b.timeout(8 * time.Hour)
```
