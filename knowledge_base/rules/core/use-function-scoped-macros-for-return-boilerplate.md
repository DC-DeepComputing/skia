---
name: skia/core/use-function-scoped-macros-for-return-boilerplate
description: When reducing repetitive return boilerplate across multiple early exits, define a function-scoped helper macro enclosed in do-while(0) and undefined immediately after the function
rubric: readability
severity: nit
---

**Rule.** When a function has repetitive return boilerplate across multiple
early-exit points (such as updating an optional out-parameter and returning a
status or nullopt), encapsulate the return sequence in a function-scoped helper
macro. Enforce macro hygiene by wrapping multi-statement macro bodies in do {
... } while (0) and #undef'ing the macro identifier immediately following the
function definition.

**Why.** Repeating identical out-parameter assignments and return statements
across numerous error branches clutters control flow and risks omitted updates
on new exit paths. Encapsulating repetitive returns in a local helper macro
keeps early exits uniform, while wrapping in do { ... } while (0) and #undef'ing
immediately after use avoids macro namespace leakage and syntax pitfalls.

## Examples

Bad:

```cpp
std::optional<SkPath> SkPath::ReadFromMemory(const void* storage, size_t length, size_t* bytesRead) {
    SkRBuffer buffer(storage, length);
    uint32_t packed;
    if (!buffer.readU32(&packed)) {
        if (bytesRead) { *bytesRead = 0; }
        return std::nullopt;
    }
    // ... repeated across multiple error exits ...
    if (bytesRead) { *bytesRead = buffer.pos(); }
    return path;
}
```

Good:

```cpp
#define RETURN_PATH_AND_BYTES(p, b) \
    do { if (bytesRead) { *bytesRead = (b); } return (p); } while (0)

std::optional<SkPath> SkPath::ReadFromMemory(const void* storage, size_t length, size_t* bytesRead) {
    SkRBuffer buffer(storage, length);
    uint32_t packed;
    if (!buffer.readU32(&packed)) {
        RETURN_PATH_AND_BYTES(std::nullopt, 0);
    }
    // ...
    RETURN_PATH_AND_BYTES(path, buffer.pos());
}
#undef RETURN_PATH_AND_BYTES
```
