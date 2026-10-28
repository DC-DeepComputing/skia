---
name: skia/use-if-constexpr-for-template-conditions
description: When branching on template parameters, type traits, compile-time constants, or configuration macros, use 'if constexpr' rather than runtime 'if'
rubric: readability
severity: should_fix
---

**Rule.** Use 'if constexpr' instead of runtime 'if' when branching on template
boolean parameters, type traits, compile-time constants, enum values, or
configuration levels in template helpers and macros. Ensure compile-time branch
selection is explicit to guarantee dead-code elimination across all build
configurations and prevent passing dynamic runtime expressions where a constant
expression is expected.

**Why.** While optimizing compilers often eliminate branches conditioned on
constant values, using 'if constexpr' makes compile-time evaluation intent
explicit and guarantees dead-code elimination across all optimization levels
without runtime overhead. In templates and macros, it also enforces that
arguments are valid compile-time constants and prevents compiling unselected
invalid branches.

## Examples

Bad:

```cpp
template <bool kAppendVerts>
void reserve(unsigned int count) {
    // Runtime 'if' on compile-time template boolean
    if (kAppendVerts) {
        alignedCount = SkAlign4(fPendingCount + count) - fPendingCount;
    }
}

#define SKIA_LOG(priority, fmt, ...)                        \
    do {                                                    \
        if (priority <= SKIA_LOWEST_ACTIVE_LOG_PRIORITY) {  \
            SkLog(priority, fmt, ##__VA_ARGS__);            \
        }                                                   \
    } while (0)
```

Good:

```cpp
template <bool kAppendVerts>
void reserve(unsigned int count) {
    // Compile-time 'if constexpr' guarantees branch elimination
    if constexpr (kAppendVerts) {
        alignedCount = SkAlign4(fPendingCount + count) - fPendingCount;
    }
}

#define SKIA_LOG(priority, fmt, ...)                                  \
    do {                                                              \
        if constexpr (priority <= SKIA_LOWEST_ACTIVE_LOG_PRIORITY) {  \
            SkLog(priority, fmt, ##__VA_ARGS__);                      \
        }                                                             \
    } while (0)
```
