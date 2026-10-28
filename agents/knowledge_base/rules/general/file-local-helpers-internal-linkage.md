---
name: skia/file-local-helpers-internal-linkage
description: When defining file-local helper functions, constants, or global variables in C++ implementation or test files, wrap them in an anonymous namespace or declare them static
rubric: readability
severity: should_fix
---

**Rule.** Give file-local helper functions, constants, and global variables
defined in C++ implementation and test (.cpp) files internal linkage by
enclosing them in an anonymous namespace (namespace { ... }) or declaring them
static. Do not leave translation-unit local helpers or file-scope globals with
external linkage in a named namespace or global scope.

**Why.** Non-class helper functions and non-const global variables without
internal linkage inherit external linkage by default, which pollutes the global
symbol table, risks linker collisions and ODR violations across translation
units, and prevents compiler optimizations like dead-code elimination.

## Examples

Bad:

```cpp
// Bad: Free helper and file-scope global left with external linkage
void draw_to_mask(const SkIRect& bounds) { ... }
const SkMaskGamma& linear_gamma() { ... }
RenderDoc gRenderDoc;
```

Good:

```cpp
// Good: Anonymous namespace or static provides internal linkage
namespace {
void draw_to_mask(const SkIRect& bounds) { ... }
}  // namespace

static const SkMaskGamma& linear_gamma() { ... }
static RenderDoc gRenderDoc;
```
