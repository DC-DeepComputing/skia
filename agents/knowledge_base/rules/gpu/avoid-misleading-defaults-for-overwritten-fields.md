---
name: skia/gpu/avoid-misleading-defaults-for-overwritten-fields
description: When defining helper structs with default member initializers, avoid giving specific fields non-uniform defaults and comments if callers unconditionally overwrite them
rubric: readability
severity: should_fix
---

**Rule.** Do not assign non-uniform default member initializers or add
explanatory comments justifying a default value for struct fields when all code
paths constructing or populating the struct unconditionally overwrite those
fields. Keep default initializers uniform across the struct and document field
semantics at the usage site rather than commenting dead default states.

**Why.** Giving a field a different default value accompanied by an explanatory
comment implies that certain code paths fall back to that default. When callers
unconditionally overwrite the field, such comments and non-uniform initializers
are misleading dead documentation that obscures how the struct is actually used.

## Examples

Bad:

```cpp
struct FeaturesToAdd {
    bool fDynamicRendering = true;
    // By default, do not query for or enable YCbCr conversion. Our implementation always opts to
    // query for it in 1.1, meaning it either will be enabled or the client manually disables it.
    bool fSamplerYcbcrConversion = false;
};
```

Good:

```cpp
struct FeaturesToAdd {
    bool fDynamicRendering = true;
    bool fSamplerYcbcrConversion = true;
};
```
