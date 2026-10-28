---
name: skia/name-classes-after-domain-concepts-not-abbreviations
description: When declaring classes or types encapsulating a specific domain concept or enum, use the full concept name rather than vague or truncated abbreviations
rubric: readability
severity: nit
---

**Rule.** When naming classes, structs, or types that encapsulate, operate on,
or specialize a domain concept or enum (such as SkBlendMode), spell out the full
domain concept name rather than using vague or truncated abbreviations (e.g.,
use 'BlendMode' instead of 'Mode' in SkBlendModeColorFilter). Keep type names
unambiguous and consistent with the specific concepts and enums they represent
across the codebase.

**Why.** Truncated names like 'Mode' or 'Format' are ambiguous because many
unrelated modes and formats exist across the codebase. Naming classes after the
specific domain concept or enum they encapsulate makes the type's purpose
immediately clear, prevents confusion with other subsystems, and ensures
predictable naming across related types, factory functions, and macros.

## Examples

Bad:

```cpp
// Bad: 'Mode' is a vague, truncated abbreviation for SkBlendMode.
class SkModeColorFilter final : public SkColorFilterBase {
public:
    SkModeColorFilter(const SkColor4f& color, SkBlendMode mode);
    SkBlendMode mode() const { return fMode; }
private:
    SkBlendMode fMode;
};
```

Good:

```cpp
// Good: 'BlendMode' spells out the specific domain concept and enum encapsulated.
class SkBlendModeColorFilter final : public SkColorFilterBase {
public:
    SkBlendModeColorFilter(const SkColor4f& color, SkBlendMode mode);
    SkBlendMode mode() const { return fMode; }
private:
    SkBlendMode fMode;
};
```
