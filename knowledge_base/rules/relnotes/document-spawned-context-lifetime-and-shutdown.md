---
name: skia/relnotes/document-spawned-context-lifetime-and-shutdown
description: When documenting spawned helper or worker context lifetimes and threading in headers or release notes, state destruction ordering and shutdown behavior explicitly
rubric: readability
severity: should_fix
---

**Rule.** When documenting API object lifetimes and threading models for spawned
helper or worker contexts (such as worker contexts created from a main context),
state explicitly whether the spawned object can safely outlive its creator and
describe its shutdown or degraded fallback behavior if the parent context is
destroyed first. Avoid vague descriptions like 'manages the lifetime' that leave
destruction ordering and invalidation contracts unspecified.

**Why.** Vague phrasing like 'manages the lifetime' obscures ownership
hierarchies and multithreaded destruction invariants. API clients need explicit
contracts on whether worker or background contexts can safely outlive their
creator and how they behave if the parent is destroyed first, preventing
use-after-free bugs and unexpected shutdown failures.

## Examples

Bad:

```cpp
The PrecompileContext will manage the lifetime of the Context objects needed for precompilation.
```

Good:

```cpp
The PrecompileContext can safely outlive the Context that created it, but it will effectively be shut down at that point.
```
