---
name: skia/api/name-parameters-by-semantic-intent
description: When declaring public API parameters or configuration options, name them after caller-facing semantic intent or effect rather than internal implementation mechanisms
rubric: readability
severity: should_fix
---

**Rule.** Name public API parameters and configuration options after their
caller-facing semantic effect or intent rather than internal implementation
mechanisms (e.g., `leak_tracer` or `leakTracer` instead of `install_atexit`).
Describe what outcome or capability the caller requests or configures, keeping
the public contract decoupled from private runtime hooks, data structures, or
helper libraries.

**Why.** Internal mechanisms (such as registering `atexit()` handlers, using
specific thread pools, or invoking internal dispatchers) are implementation
details that may evolve or be replaced. Naming parameters after internal
mechanics leaks private architecture into public contracts and confuses callers
who care about functional behavior rather than underlying machinery.

## Examples

Bad:

```cpp
// Leaks internal implementation mechanism (atexit handler registration):
class SK_API SkEventTracer {
public:
    static bool SetInstance(SkEventTracer* tracer, bool install_atexit = true);
};
```

Good:

```cpp
// Names parameter after caller-facing semantic effect:
class SK_API SkEventTracer {
public:
    static bool SetInstance(SkEventTracer* tracer, bool leak_tracer = false);
};
```
