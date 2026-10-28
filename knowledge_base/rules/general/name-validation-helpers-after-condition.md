---
name: skia/name-validation-helpers-after-condition
description: When naming validation or assertion helper functions, describe the specific condition, relationship, or target being verified rather than using vague placeholders like expectation
rubric: readability
severity: nit
---

**Rule.** Name validation, verification, and assertion helper functions after
the specific condition, invariant, or target of comparison being checked (such
as check_dimensions_match_target) rather than vague names (such as
checkDimensionsExpectation) that leave the criteria implicit. Explicitly state
what condition or relationship is expected so declarations and call sites are
self-documenting.

**Why.** Validation helpers named with vague terms like 'expectation' or bare
'check' obscure what invariant or relationship is actually being verified,
forcing readers to inspect the helper implementation. Naming the concrete
comparison target or condition makes caller assumptions and failure causes
obvious at call sites.

## Examples

Bad:

```cpp
// Vague name leaves the condition and relationship being checked implicit
void checkDimensionsExpectation(SkISize expected) const;
```

Good:

```cpp
// Explicitly identifies the expected condition and relationship being verified
static void check_dimensions_match_target(const GrSurfaceProxy* proxy, SkISize expected);
```
