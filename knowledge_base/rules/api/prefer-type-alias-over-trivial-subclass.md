---
name: skia/api/prefer-type-alias-over-trivial-subclass
description: When aliasing or deprecating types for backward compatibility, prefer using type aliases over trivial subclasses and update forward declarations
rubric: readability
severity: should_fix
---

**Rule.** Prefer C++ type aliases (`using OldName = NewName;`) over trivial
subclassing when aliasing or deprecating types for backward compatibility.
Because type aliases cannot be forward-declared (`class OldName;`), update or
remove existing forward declarations across internal headers and embedder code
to reference the underlying class or include the defining header instead.

**Why.** Trivial subclassing introduces a distinct derived type rather than a
true alias, requiring forwarded constructors and assignment operators, risking
object slicing, and preventing transparent interchangeability in containers or
smart pointers. Type aliases make the types identical at compile time,
eliminating boilerplate while requiring callers that previously relied on
forward declarations to reference the true underlying type.

## Examples

Bad:

```cpp
/** DEPRECATED: Trivial subclass aliasing SkRuntimeEffectBuilder. */
class SK_API SkRuntimeShaderBuilder : public SkRuntimeEffectBuilder {
public:
    explicit SkRuntimeShaderBuilder(sk_sp<SkRuntimeEffect> effect)
            : SkRuntimeEffectBuilder(std::move(effect)) {}
};
```

Good:

```cpp
/** DEPRECATED: Subclass logic has been moved to base class SkRuntimeEffectBuilder. */
using SkRuntimeShaderBuilder = SkRuntimeEffectBuilder;
```
