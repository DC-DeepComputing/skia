---
name: skia/gpu/reuse-intrusive-list-nodes-for-deferred-cleanup
description: When deferring cleanup of objects evicted from an SkTInternalLList, move unlinked nodes into a secondary intrusive list rather than an auxiliary array
rubric: safety
severity: should_fix
---

**Rule.** When deferring destruction or reuse of objects evicted from an
SkTInternalLList, move the unlinked nodes into a secondary SkTInternalLList
rather than storing them in an auxiliary array container or copying handles.
Reuse the object's existing intrusive link pointers once unlinked from the
primary list.

**Why.** Objects managed by an intrusive list (SkTInternalLList) already have
embedded link pointers. Once unlinked from an active cache, chaining the nodes
into a secondary pending-cleanup list requires zero heap allocations, zero
copying of smart-pointer handles, and constant O(1) time.

## Examples

Bad:

```cpp
// Storing evicted resources in an auxiliary array allocates memory:
TrackedResourceArray<sk_sp<Resource>> fPendingBuffers;
...
auto* tail = fBuffersLRU.tail();
fBuffersLRU.remove(tail);
fPendingBuffers.push_back(std::move(tail->buffer()));
delete tail;
```

Good:

```cpp
// Reusing intrusive link pointers in a secondary list with zero allocation:
SkTInternalLList<BufferNode> fPendingBuffers;
...
auto* tail = fBuffersLRU.tail();
fBuffersLRU.remove(tail);
fPendingBuffers.addToHead(tail);
```
