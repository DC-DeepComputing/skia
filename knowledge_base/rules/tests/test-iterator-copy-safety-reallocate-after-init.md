---
name: skia/tests/test-iterator-copy-safety-reallocate-after-init
description: When testing that an iterator or view safely copies underlying container storage, trigger container mutation or reallocation after initializing the iterator
rubric: testing
severity: should_fix
---

**Rule.** In unit tests verifying that an iterator, view, or consumer holds an
independent snapshot of container storage (such as testing copy-on-write or
use-after-free resilience under sanitizers), trigger container mutations or
reallocations (e.g. reserve(), incReserve(), or push_back()) after constructing
the iterator. Mutating or reserving capacity before initializing the consumer
leaves the source container buffer static during iteration, defeating the test's
intent.

**Why.** If a container is reallocated or mutated before the iterator is
constructed, the iterator merely binds to or copies the already-relocated
buffer, failing to verify whether the iterator remains valid and independent
when the underlying source container relocates during its lifetime.

## Examples

Bad:

```cpp
// Container is reallocated before creating the iterator; iteration
// does not verify resilience against source buffer relocation.
path.incReserve(100);
SkContourMeasureIter iter(path, /*forceClosed=*/false);
while (iter.next()) {
}
```

Good:

```cpp
// Iterator is created first, then container is reallocated
// to verify the iterator remains valid with its own copy.
SkContourMeasureIter iter(path, /*forceClosed=*/false);
path.incReserve(100);
while (iter.next()) {
}
```
