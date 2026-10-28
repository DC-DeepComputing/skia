---
name: skia/construct-in-place-with-skarenaalloc-make
description: When allocating objects with SkArenaAlloc, pass constructor arguments directly to alloc->make<T>(...) rather than default-allocating and assigning in separate statements
rubric: readability
severity: should_fix
---

**Rule.** When allocating objects in an SkArenaAlloc (or SkSTArenaAlloc), pass
constructor arguments directly to alloc->make<T>(...)—such as passing an
existing object to copy-construct—rather than calling parameterless
alloc->make<T>() and assigning fields or dereferencing to copy in separate
statements. Construct objects in place directly within the make<T> call.

**Why.** SkArenaAlloc::make<T>(Args&&...) perfectly forwards arguments to
placement new, constructing the object directly in arena memory. Constructing in
place avoids redundant default-initialization followed by assignment, eliminates
intermediate temporaries, and works cleanly for types that lack a default
constructor.

## Examples

Bad:

```cpp
// Redundant default construction followed by copy assignment:
Context* ptr = alloc->make<Context>();
*ptr = existingContext;
return ptr;
```

Good:

```cpp
// Direct in-place copy construction:
return alloc->make<Context>(existingContext);
```
