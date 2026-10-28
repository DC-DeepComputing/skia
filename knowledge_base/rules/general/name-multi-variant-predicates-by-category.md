---
name: skia/name-multi-variant-predicates-by-category
description: When naming predicates for variant types or enum sets, use category names for multi-variant checks and qualify subtype-specific queries with the concrete subtype
rubric: readability
severity: should_fix
---

**Rule.** When declaring variables, predicates, or helpers that test whether an
enum value belongs to a range or set of variants, name them after the general
category or concept represented rather than a single specific variant (e.g.,
isContourVerb, not isVerbALine). Additionally, when adding a predicate to a
variant or discriminated union class that checks a property exclusive to one
underlying subtype, explicitly include the subtype in the method name (e.g.,
isVolatilePath(), not isVolatile()).

**Why.** Naming a multi-variant check after a single variant misleads readers
into believing only that specific variant is handled. Conversely, using an
unqualified predicate name on a variant type falsely suggests the property
applies across all subtypes; returning false for non-matching subtypes can
mislead callers into assuming the property was evaluated as false rather than
inapplicable.

## Examples

Bad:

```cpp
// Bad: checks a range of variants but named after a single variant.
bool isVerbALine = verb >= SkPathVerb::kLine && verb <= SkPathVerb::kCubic;

// Bad: unqualified predicate on a variant type queries a subtype-specific property.
bool isVolatile() const {
    return fType == Type::kPath && this->path().isVolatile();
}
```

Good:

```cpp
// Good: category-level name clearly conveys that multiple variants are matched.
bool isContourVerb = verb >= SkPathVerb::kLine && verb <= SkPathVerb::kCubic;

// Good: explicitly qualifies the predicate with the subtype being queried.
bool isVolatilePath() const {
    return fType == Type::kPath && this->path().isVolatile();
}
```
