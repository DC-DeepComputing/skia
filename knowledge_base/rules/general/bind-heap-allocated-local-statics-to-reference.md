---
name: skia/bind-heap-allocated-local-statics-to-reference
description: When declaring heap-allocated function-local static instances to avoid exit-time destructors, bind them directly to a reference instead of a raw pointer
rubric: readability
severity: nit
---

**Rule.** When declaring a function-local heap-allocated static instance (such
as an intentionally leaked SkMutex or helper object used to avoid exit-time
destructors), bind it directly to a static reference (static Type& var = *(new
Type); or static auto& var = *new Type;) rather than storing a static pointer.
Access the instance directly using reference semantics without pointer
dereferencing.

**Why.** Binding a heap-allocated static instance to a reference eliminates
pointer semantics, redundant dereferences (*var), and unnecessary null checks at
use sites while preserving standard C++11 thread-safe one-time dynamic
initialization.

## Examples

Bad:

```cpp
void LogDebugInfo() {
    // Bad: storing a static pointer forces pointer dereferencing (*mutex) at use sites.
    static SkMutex* mutex = new SkMutex{};
    SkAutoMutexExclusive ac(*mutex);
}
```

Good:

```cpp
void LogDebugInfo() {
    // Good: binding to a static reference provides clean reference semantics without indirection.
    static SkMutex& mutex = *(new SkMutex);
    SkAutoMutexExclusive ac(mutex);
}
```
