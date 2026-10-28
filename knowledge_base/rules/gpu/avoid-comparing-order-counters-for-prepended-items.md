---
name: skia/gpu/avoid-comparing-order-counters-for-prepended-items
description: When determining the latest execution order across draw containers, do not compare monotonically increasing allocation counters for items prepended to a list head
rubric: correctness
severity: should_fix
---

**Rule.** When determining the latest draw or execution order across containers
(such as Graphite's draw layers or clip stacks), do not compare monotonically
increasing allocation order counters between elements within containers where
items are prepended to the head (such as via addToHead). Prepending elements
reverses their execution sequence relative to creation order, so higher
allocation counters indicate earlier rather than later execution; compare outer
container or layer orders instead.

**Why.** Monotonically increasing creation counters only reflect forward
sequence when items are appended to a queue. When draws or bindings are
prepended to the head of a list, elements allocated later execute earlier in the
pipeline traversal, causing intra-container counter comparisons to invert the
intended execution order.

## Examples

Bad:

```cpp
// Bad: Comparing intra-layer wrapper counters when items were prepended to head
if (insertion.layer->fOrder > latestInsertion.layer->fOrder) {
    latestInsertion = insertion;
} else if (insertion.layer->fOrder == latestInsertion.layer->fOrder) {
    if (insertion.wrapper->fOrder > latestInsertion.wrapper->fOrder) {
        latestInsertion = insertion;
    }
}
```

Good:

```cpp
// Good: Compare only layer order, avoiding intra-layer counter comparisons
bool operator>(const Insertion& other) const {
    SkASSERT(*this);
    if (!other.fLayer) {
        return true;
    }
    return fLayer->fOrder > other.fLayer->fOrder;
}

if (insertion > latestInsertion) {
    latestInsertion = insertion;
}
```
