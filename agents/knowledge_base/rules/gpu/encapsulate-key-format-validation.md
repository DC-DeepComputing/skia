---
name: skia/gpu/encapsulate-key-format-validation
description: When validating or traversing structured packed representations like PaintParamsKey, encapsulate format parsing and block iteration within the class rather than in external utilities
rubric: design
severity: should_fix
---

**Rule.** Encapsulate internal format parsing, block or snippet iteration, and
validity checks directly as member methods on structured key classes (such as
PaintParamsKey). Do not write external free helper functions in serialization
utilities or caller files that unpack and traverse the key's internal
representation.

**Why.** Classes that manage packed binary or block representations are
authoritative over their internal layout and invariants. Delegating format
traversal and validation to external helper functions leaks internal encoding
details, breaks encapsulation, and scatters format-parsing logic across utility
files.

## Examples

Bad:

```cpp
// In SerializationUtils.cpp
[[nodiscard]] bool is_block_valid(const ShaderCodeDictionary* dict, SkSpan<const uint32_t> keyData, int* currentIndex);
[[nodiscard]] bool is_key_valid(const ShaderCodeDictionary* dict, PaintParamsKey key) {
    SkSpan<const uint32_t> data = key.data();
    int currentIndex = 0;
    while (currentIndex < SkTo<int>(data.size())) {
        if (!is_block_valid(dict, data, &currentIndex)) {
            return false;
        }
    }
    return true;
}
```

Good:

```cpp
// In PaintParamsKey.h / PaintParamsKey.cpp
class PaintParamsKey {
public:
    [[nodiscard]] bool isSerializable(const ShaderCodeDictionary*) const;
    ...
};
```
