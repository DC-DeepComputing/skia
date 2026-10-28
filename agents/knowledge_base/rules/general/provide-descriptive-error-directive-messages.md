---
name: skia/provide-descriptive-error-directive-messages
description: 'When writing #error preprocessor directives for invalid configurations or macro definitions, provide an explicit message explaining why it is disallowed rather than cryptic or questioning text'
rubric: readability
severity: nit
---

**Rule.** When writing #error preprocessor directives to guard against
disallowed configurations, mutually exclusive flags, or forbidden macro
definitions, provide an explicit, self-explanatory message explaining why the
configuration is disallowed and the associated hazard (such as test-only
dependencies or codesize bloat). Do not use vague placeholder text, rhetorical
questions, or uninformative error strings.

**Why.** Cryptic, questioning, or placeholder error strings in compile-time
preprocessor directives fail to explain why a build configuration is invalid or
how to resolve it. A descriptive diagnostic documents the invariant for future
maintainers and clearly communicates the architectural hazard causing the
failure.

## Examples

Bad:

```cpp
#if defined(GPU_TEST_UTILS)
#error "Why is this here?"
#endif
```

Good:

```cpp
#if defined(GPU_TEST_UTILS)
#error "This define should not be set, as it brings in test-only things and bloats codesize."
#endif
```
