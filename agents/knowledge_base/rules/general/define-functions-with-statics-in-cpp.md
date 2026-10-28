---
name: skia/define-functions-with-statics-in-cpp
description: When declaring functions with static local variables or counters in headers, define them in .cpp translation units instead of exporting with SK_SPI or SK_API
rubric: design
severity: should_fix
---

**Rule.** Do not define functions containing static or stateful objects (such as
function-local statics, atomic counters, or singleton instances) inline in
headers and export them with SK_SPI or SK_API. Declare the function in the
header without symbol export macros and move its definition into a .cpp
translation unit.

**Why.** Functions defined inline in headers that declare static objects get
instantiated separately with hidden visibility in each shared library that
includes them, leading to duplicate state across dynamic shared objects (DSOs).
While exporting them via SK_SPI or SK_API forces dynamic linker deduplication,
it needlessly enlarges Skia's exported symbol table with internal implementation
details. Moving definitions into .cpp translation units prevents duplicate DSO
state while keeping symbols private.

## Examples

Bad:

```cpp
// In GrVkSampler.h
static SK_SPI uint32_t GenID() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID++;
    } while (id == SK_InvalidUniqueID);
    return id;
}
```

Good:

```cpp
// In GrVkSampler.h
static uint32_t GenID();

// In GrVkSampler.cpp
uint32_t GrVkSampler::GenID() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID++;
    } while (id == SK_InvalidUniqueID);
    return id;
}
```
