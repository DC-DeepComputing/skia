---
name: skia/avoid-temporary-code-duplication
description: When refactoring or migrating code across files, share transitional helpers via common headers rather than duplicating functions across translation units
rubric: design
severity: should_fix
---

**Rule.** Do not duplicate helper functions or logic across files during
refactoring or migrations, even as a temporary measure. Instead, expose and
share the function in an appropriate common or target header (annotated with an
attributed TODO comment if its placement is transitional) and update existing
call sites to invoke it directly.

**Why.** Duplicating code across translation units during a staged migration
creates divergence risk, code bloat, and confusion over which implementation is
authoritative. Sharing transitional helpers immediately via internal headers
keeps logic single-sourced, avoids duplicate bug fixes, and makes the remaining
migration steps explicit.

## Examples

Bad:

```cpp
// In src/core/SkRuntimeEffect.cpp:
// TODO(kjlubick) migrate this into GrFragmentProcessors
static GrFPResult make_effect_fp(...) { ... }

// And duplicated in src/gpu/ganesh/GrFragmentProcessors.cpp:
GrFPResult make_effect_fp(...) { ... }
```

Good:

```cpp
// In src/gpu/ganesh/GrFragmentProcessors.h:
// TODO(kjlubick, brianosman): Remove after all related effects have been migrated.
GrFPResult make_effect_fp(...);

// In src/core/SkRuntimeEffect.cpp:
return GrFragmentProcessors::make_effect_fp(...);
```
