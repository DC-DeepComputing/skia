---
name: skia/api/qualify-performance-claims-in-option-docs
description: When documenting client-facing configuration options or optimization flags in public headers, qualify performance claims conditionally and specify which workloads benefit
rubric: readability
severity: should_fix
---

**Rule.** When documenting client-facing configuration options, options structs,
or optimization flags in public headers, qualify performance claims (e.g., 'may
improve performance') rather than promising unconditional gains. Explicitly
describe the specific workloads, conditions, or internal mechanisms (such as
text rendering, atlasing, or resource reuse) that actually benefit.

**Why.** Unconditional performance claims on client-facing configuration flags
mislead API consumers whose workloads (such as compositors that do not render
text or rely on internal atlasing) see no benefit or might even incur overhead.
Specifying both the conditional nature of the optimization and the workloads
that benefit allows callers to make informed configuration decisions.

## Examples

Bad:

```cpp
// If Recordings are replayed in the order they are recorded, then Graphite can
// make certain assumptions that allow for better performance. Otherwise we have to flush some
// caches at the start of each Recording to ensure that they can be played back properly.
std::optional<bool> fRequireOrderedRecordings;
```

Good:

```cpp
// If Recordings are known to be played back in the order they are recorded, then Graphite
// may be able to make certain assumptions that improve performance. This is often the case
// if the content being drawn triggers the use of internal atlasing in Graphite (e.g. text).
std::optional<bool> fRequireOrderedRecordings;
```
