---
name: skia/return-matching-element-with-query-result
description: When querying or retrieving container elements across function boundaries, pass resolved elements down to callees and return found items with query results to avoid redundant lookups
rubric: safety
severity: should_fix
---

**Rule.** Avoid redundant container, map, or collection lookups across function
boundaries. When a query or validation helper locates an item in a collection,
return the found element or iterator alongside the query result rather than
discarding it. Conversely, when a caller already retrieved an element or
collection entry, pass the resolved pointer or reference directly down the call
chain rather than passing keys or indices that force callees to re-query the
same data structure.

**Why.** Re-querying containers or repeating search loops across function calls
incurs unnecessary traversal and hashing overhead, especially inside loops or
hot paths. Passing or returning the already-resolved element pointer or
reference eliminates duplicate lookups and keeps retrieval logic centralized.

## Examples

Bad:

```cpp
// Bad: Discarding found item forces caller to repeat search traversal.
BoundsTest result = current->test(drawParams->drawBounds(), key, isStencil);
if (result == BoundsTest::kCompatibleOverlap) {
    targetMatch = current->searchBinding(key);
    break;
}

// Bad: Caller looks up map entry but passes key/index, forcing callee to re-query.
if (Specializations* specs = fSpecializationMap.find(&f.declaration())) {
    for (int i = 0; i < specs->size(); i++) {
        this->writeFunctionInstantiation(f, i, out); // callee re-runs find(&f)
    }
}
```

Good:

```cpp
// Good: Return located element alongside query result.
auto [testResult, match] = current->test(drawParams->drawBounds(), key, isStencil);
targetMatch = match;
if (testResult == BoundsTest::kCompatibleOverlap) {
    break;
}

// Good: Forward resolved element pointer directly down the call chain.
if (const Specializations* specs = fSpecializationMap.find(&f.declaration())) {
    for (int i = 0; i < specs->size(); i++) {
        this->writeFunctionInstantiation(f, i, &specs->at(i), out);
    }
}
```
