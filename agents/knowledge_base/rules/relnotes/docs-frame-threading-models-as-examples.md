---
name: skia/relnotes/docs-frame-threading-models-as-examples
description: When describing multi-threading architectures or thread allocations in documentation or release notes, frame configurations as illustrative examples rather than canonical requirements
rubric: readability
severity: nit
---

**Rule.** When documenting multi-threading setups, thread allocations, or
concurrency architectures in release notes or architectural documentation,
describe specific thread counts and role distributions as illustrative examples
(e.g., 'As an example of a possible threading model, one could employ...')
rather than prescribing them as the canonical or standard Skia threading model.

**Why.** Skia is an embeddable graphics library designed to be flexible and
integrate into diverse host threading architectures without mandating fixed
thread counts or dedicated roles. Describing a specific multi-threaded
configuration as 'the general threading model' misleads clients and embedders
into believing that exact structure is required or officially standard.

## Examples

Bad:

```cpp
The general threading model in Skia, for Compilation and Precompilation, involves 4 threads: 
2 for creating Recordings (<r1> and <r2>), 1 for precompiling (<p1>) and the main thread - which 
owns the Context and submits Recordings.
```

Good:

```cpp
As an example of a possible Compilation/Precompilation threading model, one could employ 4 threads:

2 for creating Recordings (<r1> and <r2>)
1 for precompiling (<p1>)
and the main thread - which owns the Context and submits Recordings.
```
