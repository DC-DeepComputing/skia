---
name: skia/sksl/avoid-redundant-specialization-maps-in-codegen
description: In SkSL code generators, avoid maintaining auxiliary maps for subsets of specialized parameters; track and query the active specialization pointer directly with null checks
rubric: design
severity: should_fix
---

**Rule.** In SkSL backend code generators, do not maintain redundant auxiliary
maps or secondary caches to track subsets of function specialization data.
Instead, track the active specialization pointer (such as fActiveSpecialization)
on the generator during function instantiation, and query it directly with a
null check to handle unspecialized functions.

**Why.** Specialized parameter mappings are already held in
SpecializedParameters. Creating and populating auxiliary maps for parameter
subsets duplicates state, increases allocation overhead, and complicates
maintenance. Querying the active specialization pointer directly provides a
single source of truth and allows unspecialized functions to take a fast-path
null bailout.

## Examples

Bad:

```cpp
// In code generator: maintain a secondary map for a subset of specialized parameters
THashMap<const Variable*, const Expression*> fUnsizedArrayVariableMap;
...
if (specializedExpr && (*specializedExpr)->is<FieldAccess>()) {
    fUnsizedArrayVariableMap.set(parameter, *specializedExpr);
}
```

Good:

```cpp
// Track active specialization pointer during function instantiation and query directly
fActiveSpecialization = specializedParams;
...
if (fActiveSpecialization) {
    const Expression** specializedExpr = fActiveSpecialization->find(&var);
    if (specializedExpr && (*specializedExpr)->is<FieldAccess>()) {
        return this->getStorageClass(**specializedExpr);
    }
}
```
