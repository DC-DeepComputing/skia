---
name: skia/consolidate-conditional-array-initializers
description: 'When initializing static arrays or lists with build-dependent items, consolidate conditional entries into one #if/#else/#endif block and group unconditional entries together'
rubric: readability
severity: nit
---

**Rule.** When initializing static arrays, tables, or lists that contain
configuration-dependent entries (such as debug vs. release toggles or platform
features), consolidate mutually exclusive conditional entries into a single `#if
... #else ... #endif` block. Group common unconditional entries together rather
than scattering separate `#if` directives throughout the list initializer.

**Why.** Interleaving multiple `#if` and `#if !defined(...)` blocks across an
array initializer fragments list definitions and obscures which entries are
common versus configuration-specific. Consolidating mutually exclusive entries
into a single `#if/#else/#endif` section keeps build variants clearly visible
and avoids scattered preprocessor boilerplate.

## Examples

Bad:

```cpp
static constexpr const char* kToggles[] = {
#if !defined(SK_DEBUG)
        "skip_validation",
#endif
        "disable_lazy_clear_for_mapped_at_creation_buffer",
        "allow_unsafe_apis",
#if defined(SK_DEBUG)
        "use_user_defined_labels_in_backend",
#endif
        "disable_robustness",
};
```

Good:

```cpp
static constexpr const char* kToggles[] = {
#if defined(SK_DEBUG)
        // Setting labels on backend objects has performance overhead.
        "use_user_defined_labels_in_backend",
#else
        "skip_validation",
#endif
        "disable_lazy_clear_for_mapped_at_creation_buffer",
        "allow_unsafe_apis",
        "disable_robustness",
};
```
