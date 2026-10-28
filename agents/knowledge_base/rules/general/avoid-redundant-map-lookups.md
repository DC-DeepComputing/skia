---
name: skia/avoid-redundant-map-lookups
description: When querying associative containers or maps, capture find() pointers to prevent duplicate lookups, and guard hot-path queries behind cheap active-state checks
rubric: safety
severity: should_fix
---

**Rule.** When querying associative containers or maps (such as
skia_private::THashMap or std::unordered_map):
1. Do not query once to check presence and again to fetch the value. Capture or hoist the pointer returned by find() into a local variable, test it for nullptr, and reuse it directly.
2. On hot paths (such as AST traversals or codegen visitors) where specialized mappings only exist in specific contexts, guard lookups behind a cheap fast-path check (such as a non-null active-context pointer or non-empty flag) rather than querying the container unconditionally.

**Why.** Associative container lookups compute hashes and traverse internal
bucket arrays. Reusing the pointer from find() eliminates redundant duplicate
lookups and simplifies branching. Similarly, guarding lookups with a cheap
pointer or flag check avoids unnecessary hashing overhead on the common path
when specialized context is inactive.

## Examples

Bad:

```cpp
// Bad: duplicate lookup for presence then value
bool ignoreParam = specializedParams && specializedParams->find(parameter);
if (ignoreParam) {
    const Variable** uniformVar = specializedParams->find(parameter);
    fVariableMap.set(parameter, *fVariableMap.find(*uniformVar));
}

// Bad: unconditional lookup on hot path when specialization is rare
const Variable& var = *expr.as<VariableReference>().variable();
if (const Expression** specialized = fUnsizedArrayVariableMap.find(&var)) {
    return this->getStorageClass(**specialized);
}
```

Good:

```cpp
// Good: capture find() pointer and reuse directly
const Variable** specializedVar =
        specializedParams ? specializedParams->find(parameter) : nullptr;
if (specializedVar) {
    SpvId* uniformId = fVariableMap.find(*specializedVar);
    SkASSERT(uniformId);
    fVariableMap.set(parameter, *uniformId);
}

// Good: guard lookup behind cheap active-state check
const Variable& var = *expr.as<VariableReference>().variable();
if (fActiveSpecialization) {
    if (const Expression** specialized = fActiveSpecialization->find(&var)) {
        return this->getStorageClass(**specialized);
    }
}
```
