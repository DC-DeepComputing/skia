---
name: skia/avoid-consecutive-anonymous-namespaces
description: When structuring translation units, do not open consecutive anonymous namespaces back-to-back; merge them or use distinct named namespaces for logical scoping
rubric: readability
severity: nit
---

**Rule.** Do not declare consecutive anonymous namespaces back-to-back in the
same translation unit without intervening code. Merge them into a single
anonymous namespace, or use distinct named namespaces (optionally enclosing
anonymous namespaces for internal linkage) when separate logical scoping or
symbol isolation is intended.

**Why.** In C++, all anonymous namespaces within the same translation unit share
identical scope, so closing and immediately reopening an anonymous namespace
provides neither symbol isolation nor protection against name collisions.
Merging them eliminates redundant boilerplate, while explicit named namespaces
correctly establish distinct symbol scopes.

## Examples

Bad:

```cpp
namespace {
struct HeaderOutlineEntry { ... };
void make_entry(...) { ... }
}  // namespace

namespace {
struct StructElemOutlineEntry { ... };
void make_entry(...) { ... }  // Name collision with earlier make_entry
}  // namespace
```

Good:

```cpp
// Option 1: Merge into a single anonymous namespace when separate scopes are unneeded.
namespace {
struct HeaderOutlineEntry { ... };
struct StructElemOutlineEntry { ... };
}  // namespace

// Option 2: Use distinct named namespaces to logically isolate identically named symbols.
namespace header_outline {
namespace {
struct Entry { ... };
void make_entry(...) { ... }
}  // namespace
}  // namespace header_outline

namespace structelem_outline {
namespace {
struct Entry { ... };
void make_entry(...) { ... }
}  // namespace
}  // namespace structelem_outline
```
