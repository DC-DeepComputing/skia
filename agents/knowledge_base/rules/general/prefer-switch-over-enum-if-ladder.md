---
name: skia/prefer-switch-over-enum-if-ladder
description: When branching across multiple enum values, prefer a switch statement over an if-else ladder and avoid mixing negative checks into positive chains
rubric: readability
severity: nit
---

**Rule.** When branching on enum values across multiple enumerators, prefer a
switch statement with explicit case labels over an if ... else if ladder. In
particular, avoid mixing negative inequality conditions (!=) into chains of
positive equality checks; express unhandled variants in a dedicated default:
label and document intentional no-ops with an explicit break and comment.

**Why.** In an if-else if ladder, inserting a negative condition (!=) among
positive equality checks easily misleads readers and obscures which enumerators
are handled. A switch statement clearly partitions variant handling, makes no-op
branches explicit, cleanly isolates fallback logic in default:, and enables
compiler exhaustiveness diagnostics.

## Examples

Bad:

```cpp
if (baseType->typeKind() == Type::TypeKind::kTexture) {
    permittedLayoutFlags &= ~Layout::kSampler_Flag;
} else if (baseType->typeKind() == Type::TypeKind::kSeparateSampler) {
    permittedLayoutFlags &= ~Layout::kTexture_Flag;
} else if (baseType->typeKind() != Type::TypeKind::kSampler) {
    permittedLayoutFlags &= ~Layout::kTexture_Flag;
    permittedLayoutFlags &= ~Layout::kSampler_Flag;
}
```

Good:

```cpp
switch (baseType->typeKind()) {
    case Type::TypeKind::kSampler:
        // Both texture and sampler flags are permitted.
        break;
    case Type::TypeKind::kTexture:
        permittedLayoutFlags &= ~Layout::kSampler_Flag;
        break;
    case Type::TypeKind::kSeparateSampler:
        permittedLayoutFlags &= ~Layout::kTexture_Flag;
        break;
    default:
        permittedLayoutFlags &= ~(Layout::kTexture_Flag | Layout::kSampler_Flag);
        break;
}
```
