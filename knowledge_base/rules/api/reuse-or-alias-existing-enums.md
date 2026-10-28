---
name: skia/api/reuse-or-alias-existing-enums
description: When declaring types matching existing core enums, reuse the canonical enum or introduce a type alias instead of defining duplicate enums
rubric: readability
severity: nit
---

**Rule.** Prefer reusing or type-aliasing existing core Skia enums (such as
SkColorChannel, SkTileMode, or SkBlendMode) instead of declaring duplicate local
enums with identical enumerators. If domain-specific naming is desired at the
call site, declare a type alias (e.g., 'using LocalName = CoreEnum;') rather
than defining a new enum type.

**Why.** Defining duplicate enums creates unnecessary type proliferation and
requires writing and maintaining boilerplate conversion switch functions when
interoperating with core Skia APIs. Reusing or aliasing canonical enums ensures
direct API compatibility and eliminates conversion code without sacrificing
readable domain-specific naming.

## Examples

Bad:

```cpp
// Declaring a duplicate enum requires conversion functions to interoperate with Skia APIs.
enum class ChannelSelector {
    kR,
    kG,
    kB,
    kA,
};
```

Good:

```cpp
// Reusing or aliasing the core enum eliminates boilerplate conversion functions.
using ChannelSelector = SkColorChannel;
```
