---
name: skia/prefer-extended-enum-over-enum-bool-pair
description: When extending or qualifying an enum variant with a condition, define a dedicated enumerator instead of pairing the enum with an auxiliary boolean
rubric: readability
severity: should_fix
---

**Rule.** Do not bundle an enum with an auxiliary boolean flag (into a struct,
tuple, or parameter pair) when that boolean is only meaningful for a specific
enum variant. Instead, define an extended or mirrored domain enum that
represents each distinct state as its own first-class enumerator, and map down
to the underlying or external enum only at API boundaries.

**Why.** Pairing an enum with an auxiliary boolean creates invalid state
combinations for every variant where the boolean is meaningless or nonsensical.
Defining a dedicated domain enum makes invalid states unrepresentable,
simplifies method signatures, and enables the compiler to enforce exhaustive
switch coverage across all variants.

## Examples

Bad:

```cpp
// Bad: fIsBufferAsUniformClass is meaningless unless fStorageClass == SpvStorageClassUniform
struct StorageClassPair {
    SpvStorageClass fStorageClass;
    bool fIsBufferAsUniformClass;
};

void emitVariable(StorageClassPair storageClass);
```

Good:

```cpp
// Good: Mirror external enum with first-class enumerators to make invalid states unrepresentable
enum class StorageClass {
    kUniformConstant,
    kInput,
    kUniform,
    kStorageBuffer, // First-class variant for storage buffers
    kOutput,
};

static SpvStorageClass get_storage_class_spv_id(StorageClass storageClass) {
    switch (storageClass) {
        case StorageClass::kStorageBuffer: return SpvStorageClassUniform;
        ...
    }
}
```
