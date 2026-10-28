---
name: skia/api/bundle-callback-context-into-struct
description: When designing callback or hook signatures whose contextual parameters may evolve, bundle arguments into a dedicated context struct passed by const reference
rubric: readability
severity: nit
---

**Rule.** When defining callback, hook, or notification function signatures
whose contextual inputs may evolve, bundle parameters into a dedicated context
struct passed by const reference rather than passing individual arguments
directly. Even when only a single argument is initially needed, encapsulating it
within a context struct provides forward compatibility for future extensions.

**Why.** Passing individual arguments directly in callback signatures makes
future API extension a breaking change whenever additional context or metadata
is needed. Encapsulating arguments in a dedicated context struct passed by const
reference allows new members to be added without altering callback signatures or
breaking existing call sites.

## Examples

Bad:

```cpp
// Breaking change whenever additional contextual metadata is required
using UniformsCallback = std::function<sk_sp<const SkData>(const SkColorSpace*)>;
```

Good:

```cpp
// Forward-compatible: new fields can be added without altering the signature
struct UniformsCallbackContext {
    const SkColorSpace* fDstColorSpace;
};
using UniformsCallback = std::function<sk_sp<const SkData>(const UniformsCallbackContext&)>;
```
