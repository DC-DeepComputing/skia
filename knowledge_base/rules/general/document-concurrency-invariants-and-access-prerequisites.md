---
name: skia/document-concurrency-invariants-and-access-prerequisites
description: When documenting concurrency invariants or claims that concurrent threads cannot mutate state, explicitly detail the prerequisite conditions preventing concurrent access
rubric: readability
severity: nit
---

**Rule.** When documenting concurrency invariants, thread-safety assumptions, or
claims that concurrent threads cannot alter an object's state (e.g., that no
other thread can increment reference counts or trigger state transitions),
explicitly document the prerequisite conditions and access rules that guarantee
thread safety. Do not merely assert that concurrent mutation is impossible
without explaining why external threads cannot hold references or acquire
access.

**Why.** Bare assertions that concurrent threads cannot mutate an object or
increment its references leave readers and maintainers unable to verify thread
safety or detect subtle data races. Explaining concrete prerequisite
conditions—such as reference counts having reached zero, single-threaded
ownership boundaries, or exclusion guarantees—makes concurrency invariants
verifiable and prevents accidental breakage during maintenance.

## Examples

Bad:

```cpp
// If true is returned there are no other refs that could trigger a reusable or purgeable state
// change, since only the calling cache thread can increase refs again.
bool isLastExternalRef();
```

Good:

```cpp
// If true is returned there are no other refs that could trigger a reusable or purgeable state
// change. A resource that entered the return queue due to becoming reusable or purgeable only
// happens if that ref count reached zero, so there should be no external ref holder (other than
// the ResourceCache with its separate cache ref). However, unrefReturnQueue() is only called by
// the ResourceCache so it won't be simultaneously handing out usage refs.
bool isLastExternalRef();
```
