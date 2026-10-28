---
name: skia/gpu/assign-layer-search-binding-result
description: When querying compatible binding lists with searchBinding, assign the returned pointer to the target variable rather than discarding the expression statement
rubric: correctness
severity: blocking
---

**Rule.** When querying compatible binding lists or layers using search methods
such as Layer::searchBinding, assign the returned BindingList* pointer to the
destination variable rather than invoking the search as a discarded expression
statement. Verify that subsequent insertion and batching operations receive the
located match instead of falling back to creating redundant bindings.

**Why.** searchBinding is a pure query method that traverses the binding list
without mutating state. Discarding its return value leaves the destination
pointer null, causing subsequent logic to treat compatible bindings as absent
and miss batching opportunities.

## Examples

Bad:

```cpp
BindingList* targetMatch = nullptr;
if (start.fList->fNext) {
    start.fLayer->searchBinding</*kForwards=*/true>(key, start.fList->fNext);
}
```

Good:

```cpp
BindingList* targetMatch = nullptr;
if (start.fList->fNext) {
    targetMatch = start.fLayer->searchBinding</*kForwards=*/true>(key, start.fList->fNext);
}
```
