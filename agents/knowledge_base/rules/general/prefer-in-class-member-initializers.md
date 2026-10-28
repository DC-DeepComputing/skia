---
name: skia/prefer-in-class-member-initializers
description: When defining structs or classes, particularly with = default constructors, initialize members (especially pointers to nullptr) at declaration rather than leaving them uninitialized or assigning post-construction
rubric: readability
severity: should_fix
---

**Rule.** When struct or class member variables have fixed default or sentinel
values, initialize them directly at declaration using in-class member
initializers (e.g., initializing pointers to '= nullptr') rather than assigning
constants in constructor bodies, defining boilerplate constructors, or manually
assigning default values after allocation (e.g., after alloc->make<T>()).
Particularly when relying on '= default' constructors, provide in-class
initializers for member variables to eliminate the risk of uninitialized access.
On plain configuration, context, or options structs, prefer in-class defaults
without custom constructors to preserve aggregate initialization and clean
default parameters. When default constructors are provided, define them as '=
default;' (marked constexpr where applicable).

**Why.** In-class member initializers eliminate boilerplate constructors and
call-site assignments, preserve aggregate initialization, and ensure a single
canonical initial state across all constructors. Defaulting constructors ('=
default') without in-class initializers leaves primitive and pointer members
uninitialized, risking undefined behavior or MSan violations. In-class defaults
also guarantee that heap-, arena-, and stack-allocated instances (such as in
tests) are safely initialized to canonical defaults without call-site
boilerplate.

## Examples

Bad:

```cpp
// Bad: Leaving member pointer uninitialized when defaulting constructor.
class ReturnsInputAlphaVisitor : public ProgramVisitor {
public:
    ReturnsInputAlphaVisitor() = default;
private:
    const Variable* fInputVar;
};

// Bad: Boilerplate constructor on config struct prevents aggregate initialization.
struct BufferSizes {
    BufferSizes(uint32_t minSize, uint32_t maxSize) : fMinSize(minSize), fMaxSize(maxSize) {}
    uint32_t fMinSize;
    uint32_t fMaxSize;
};

// Bad: Manually assigning default values post-allocation instead of in-class defaults.
struct InitLaneMasksCtx {
    uint8_t* tail;
};
auto* ctx = alloc->make<InitLaneMasksCtx>();
ctx->tail = nullptr;
```

Good:

```cpp
// Good: Initializing pointer to nullptr at declaration ensures safe default state.
class ReturnsInputAlphaVisitor : public ProgramVisitor {
public:
    ReturnsInputAlphaVisitor() = default;
private:
    const Variable* fInputVar = nullptr;
};

// Good: In-class defaults enable aggregate initialization and clean call-site defaults.
struct BufferSizes {
    uint32_t fMinSize = 16 << 10;
    uint32_t fMaxSize = 1 << 20;
};

// Good: In-class default eliminates post-allocation boilerplate on arena/stack instances.
struct InitLaneMasksCtx {
    uint8_t* tail = nullptr;
};
auto* ctx = alloc->make<InitLaneMasksCtx>();
```
