---
name: skia/tests/document-test-input-domain-restrictions
description: When a test or test generator intentionally restricts its input domain or omits edge cases, document the rationale and where omitted cases are handled
rubric: readability
severity: nit
---

**Rule.** When authoring unit tests or test input generators that intentionally
restrict their input domain or omit boundary conditions (such as only generating
points along bounding edges rather than interior points), add an explanatory
comment detailing the rationale. Clarify the component's underlying assumptions
and explain why the omitted cases are either not applicable or handled elsewhere
in the pipeline.

**Why.** Without explanatory comments, reviewers and future maintainers cannot
determine whether restricted input domains or missing edge cases represent
deliberate testing choices or oversight. Documenting the rationale prevents
false assumptions about test coverage gaps and clarifies design invariants for
future refactorings.

## Examples

Bad:

```cpp
// Bad: Restricts generated points to bounding edges without explaining why interior points are omitted
static SkPoint pick_random_square_point(SkRandom* rand) {
    int edge = rand->nextU() % 4;
    float t = rand->nextF();
    switch (edge) {
        case 0: return {t, 0.0f};
        case 1: return {1.0f, t};
        case 2: return {t, 1.0f};
        default: return {0.0f, t};
    }
}
```

Good:

```cpp
// Good: Explains why interior points are omitted and where that case is handled
// Note: Although line segments can end inside a pixel, this test evaluates LUT quantization
// which models lines as infinite traversals crossing the entire pixel. Downstream subsample
// masking logic independently handles segment endpoints falling within the interior.
static SkPoint pick_random_square_point(SkRandom* rand) {
    int edge = rand->nextU() % 4;
    float t = rand->nextF();
    switch (edge) {
        case 0: return {t, 0.0f};
        case 1: return {1.0f, t};
        case 2: return {t, 1.0f};
        default: return {0.0f, t};
    }
}
```
