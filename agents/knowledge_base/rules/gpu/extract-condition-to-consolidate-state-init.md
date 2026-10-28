---
name: skia/gpu/extract-condition-to-consolidate-state-init
description: When a compound condition governs both object or telemetry initialization and subsequent branching, extract it into a local boolean to unify conditionally compiled setup
rubric: readability
severity: should_fix
---

**Rule.** When a compound runtime condition determines whether an object or
telemetry struct is configured and also controls subsequent execution branches,
extract the condition into an explanatory local boolean variable. Initialize all
conditionally compiled fields together in an upfront preprocessor block using
that boolean rather than fragmenting state assignments across nested branch
bodies.

**Why.** Scattering state and telemetry assignments across nested branches and
preprocessor blocks obscures initialization invariants and duplicates
preprocessor boilerplate. Evaluating the condition upfront into a named boolean
clarifies branch intent and unifies struct initialization into a single,
cohesive block.

## Examples

Bad:

```cpp
auto asyncCreation = std::make_unique<AsyncPipelineCreation>();
#if SK_HISTOGRAMS_ENABLED
asyncCreation->fStartTime = skgpu::StdSteadyClock::now();
asyncCreation->fFromPrecompile = forPrecompilation;
#endif

if (caps.useAsyncPipelineCreation() && !forPrecompilation) {
#if SK_HISTOGRAMS_ENABLED
    asyncCreation->fAsynchronous = true;
#endif
    ...
}
```

Good:

```cpp
const bool useAsync = caps.useAsyncPipelineCreation() && !forPrecompilation;

auto asyncCreation = std::make_unique<AsyncPipelineCreation>();
#if SK_HISTOGRAMS_ENABLED
asyncCreation->fStartTime = skgpu::StdSteadyClock::now();
asyncCreation->fFromPrecompile = forPrecompilation;
asyncCreation->fAsynchronous = useAsync;
#endif

if (useAsync) {
    ...
}
```
