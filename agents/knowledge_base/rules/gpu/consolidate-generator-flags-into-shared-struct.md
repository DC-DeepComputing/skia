---
name: skia/gpu/consolidate-generator-flags-into-shared-struct
description: When passing context state to internal helpers, pass aggregate objects directly without decomposing members or passing loose flags, encapsulating setup in constructors
rubric: design
severity: should_fix
---

**Rule.** When passing intermediate context, pipeline, or generator state to
internal helper functions and macros, pass aggregate context objects (such as
KeyContext or SharedGeneratorData) directly rather than decomposing them into
individual member arguments across call sites or passing loose configuration
flags alongside them. Additionally, encapsulate the derivation and
initialization of helper struct state within the struct's constructor or a
dedicated factory method rather than inlining multi-step setup in outer caller
functions.

**Why.** Decomposing context objects into individual accessors or passing loose
flags alongside shared structs clutters signatures, causes repetitive call-site
boilerplate, and forces widespread churn when helper requirements change.
Encapsulating state derivation within constructors preserves helper invariants
and keeps outer callers focused on orchestration.

## Examples

Bad:

```cpp
// 1. Passing loose flags alongside a shared context struct:
result->generateFragmentSkSL(caps, dict, step, paintID, sharedData, useUniformStorageBufferFS);

// 2. Decomposing aggregate context into individual member arguments across call sites:
#define BEGIN_WRITE_UNIFORMS(gatherer, dict, snippetID) ...
BEGIN_WRITE_UNIFORMS(keyContext.pipelineDataGatherer(), keyContext.dict(), snippetID);

// 3. Inlining intermediate struct setup in outer caller instead of constructor:
SharedGeneratorData sharedData{rootNodes, collect_lifted_expressions(rootNodes), ...};
```

Good:

```cpp
// 1. Intermediate flags consolidated into shared context struct:
result->generateFragmentSkSL(caps, dict, step, paintID, sharedData);

// 2. Pass aggregate context directly to helper macros and functions:
#define BEGIN_WRITE_UNIFORMS(keyContext, snippetID) ScopedUniformWriter scope{keyContext, snippetID};
BEGIN_WRITE_UNIFORMS(keyContext, snippetID);

// 3. Encapsulate state derivation in struct constructor:
SharedGeneratorData sharedData(caps, dict, step, paintID);
```
