---
name: skia/rust/document-unowned-adapter-pointer-migration-todo
description: When retaining unowned raw pointers in shared adapters or FFI bridges instead of abstracting ownership, document the migration path with an actionable TODO
rubric: readability
severity: nit
---

**Rule.** When deferring ownership abstraction for a shared adapter or FFI
bridge class that retains an unowned raw pointer (such as an input stream),
document the technical debt with an actionable TODO comment. Detail the planned
architectural migration, such as replacing the raw pointer member with a pure
virtual accessor (e.g., virtual SkStream& stream() = 0) to delegate resource
ownership and lifetime management to derived classes.

**Why.** Holding raw unowned pointers in shared adapter layers forces external
callers into rigid lifetime guarantees and prevents derived clients from
managing custom resource ownership or caching. Documenting the planned migration
with concrete refactoring steps preserves the design path when full ownership
abstraction is deferred.

## Examples

Bad:

```cpp
class SkStreamAdapter {
public:
    explicit SkStreamAdapter(SkStream* stream) : fStream(stream) {}
private:
    SkStream* fStream;
};
```

Good:

```cpp
class SkStreamAdapter {
public:
    explicit SkStreamAdapter(SkStream* stream) : fStream(stream) {}
private:
    // TODO: Consider avoiding holding a raw / unowned pointer here:
    // * Replace `fStream` field with an abstract method declaration:
    //   `virtual SkStream& stream() = 0`.
    // * Derive-from/implement this in child classes to manage stream ownership.
    SkStream* fStream;
};
```
