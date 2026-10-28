---
name: skia/default-specialized-boolean-template-parameters
description: When declaring templates with specialized boolean non-type parameters, provide default values representing standard behavior so general call sites can omit redundant arguments
rubric: readability
severity: nit
---

**Rule.** When declaring function or class templates with trailing boolean
non-type parameters that represent specialized modes or optional variations,
supply a default value (such as = false or = true) corresponding to the standard
or common case. At call sites that rely on the baseline behavior, omit the
defaulted template argument instead of passing explicit redundant literals.

**Why.** Defaulting specialized boolean template parameters to their baseline
mode reduces repetitive call-site boilerplate and visual noise. It allows
standard callers to invoke templates without specifying redundant compile-time
flags while still allowing specialized callers to override the behavior.

## Examples

Bad:

```cpp
// Bad: Omitting default value forces standard callers to pass redundant arguments
template <bool kIsStencil, bool kIsDepthOnly>
std::pair<BoundsTest, BindingWrapper*> test(const Rect& drawBounds,
                                            const LayerKey& key,
                                            bool requiresBarrier);

// Standard call site passing baseline false explicitly:
current->test<true, false>(bounds, key, barrier);
```

Good:

```cpp
// Good: Default specialized parameter to standard mode; callers omit redundant argument
template <bool kIsStencil, bool kIsDepthOnly = false>
std::pair<BoundsTest, BindingWrapper*> test(const Rect& drawBounds,
                                            const LayerKey& key,
                                            bool requiresBarrier);

// Standard call site only specifies required non-default parameter:
current->test</*kIsStencil=*/true>(bounds, key, barrier);
```
