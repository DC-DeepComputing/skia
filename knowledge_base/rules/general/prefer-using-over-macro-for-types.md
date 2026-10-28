---
name: skia/prefer-using-over-macro-for-types
description: 'When creating type or template aliases, use C++ using declarations or template aliases instead of #define preprocessor macros'
rubric: readability
severity: should_fix
---

**Rule.** Prefer C++ type aliases (`using Name = Type;`) or template aliases
(`template <typename T> using Name = Template<T>;`) over `#define` preprocessor
macros when aliasing types or templates. Do not use `#define` for type
substitutions.

**Why.** Preprocessor macros perform global, unscoped textual substitution that
ignores C++ namespaces and scopes, risking subtle name collisions and bypassing
type checking. C++ type and template aliases respect scoping rules and integrate
cleanly with the language's type system and template mechanics.

## Examples

Bad:

```cpp
// Bad: unscoped preprocessor macro ignores namespaces and template mechanics
#define SkSpan std::span
```

Good:

```cpp
// Good: template alias respects namespaces and integrates with C++ type mechanics
template <typename T> using SkSpan = std::span<T>;
```
