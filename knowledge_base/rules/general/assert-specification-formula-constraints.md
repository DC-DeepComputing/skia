---
name: skia/assert-specification-formula-constraints
description: When implementing mathematical formulas or algorithms defined by external specifications, assert parameter constraints and domain preconditions with SkASSERT
rubric: correctness
severity: should_fix
---

**Rule.** When implementing mathematical formulas or algorithms defined by an
external standard or specification, assert that parameters satisfy all
specification-defined constraints and domain preconditions (such as unit
intervals [0, 1] or sum tolerances) with SkASSERT. For objects parsed from
external sources, enforce or assert pre-validated invariants before evaluating
the formula.

**Why.** Evaluating formulas with out-of-spec or uninitialized parameters leads
to numerical instability, invalid color calculations, or NaN propagation.
Asserting domain preconditions in debug builds verifies mathematical assumptions
at runtime, documents specification constraints in code, and catches data
corruption early.

## Examples

Bad:

```cpp
SkColor4f Agtm::ComponentMixingFunction::evaluate(const SkColor4f& c) const {
    // This implements the math in Formula 3 of SMPTE ST 2094-50.
    float common = fRed * c.fR + fGreen * c.fG + fBlue * c.fB +
                   fMax * std::max(std::max(c.fR, c.fG), c.fB) +
                   fMin * std::min(std::max(c.fR, c.fG), c.fB);
    return {fComponent * c.fR + common, fComponent * c.fG + common, fComponent * c.fB + common, c.fA};
}
```

Good:

```cpp
SkColor4f Agtm::ComponentMixingFunction::evaluate(const SkColor4f& c) const {
    // Assert that the parameters satisfy the constraints in Clause 5.2.2.
    SkASSERT(0.f <= fRed        && fRed       <= 1.f);
    SkASSERT(0.f <= fGreen      && fGreen     <= 1.f);
    SkASSERT(0.f <= fBlue       && fBlue      <= 1.f);
    SkASSERT(0.f <= fMax        && fMax       <= 1.f);
    SkASSERT(0.f <= fMin        && fMin       <= 1.f);
    SkASSERT(0.f <= fComponent  && fComponent <= 1.f);
    SkASSERT(0.99999f <= fRed + fGreen + fBlue + fMax + fMin + fComponent);
    SkASSERT(1.00001f >= fRed + fGreen + fBlue + fMax + fMin + fComponent);

    // This implements the math in Formula 3 of SMPTE ST 2094-50.
    float common = fRed * c.fR + fGreen * c.fG + fBlue * c.fB + ...;
    ...
}
```
