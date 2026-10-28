---
name: skia/revert-single-use-condition-variables
description: When evaluating complex compound conditionals, extract opaque subexpressions into descriptive const bool variables, but avoid redundant single-use intermediates if inlined clauses have per-clause comments
rubric: readability
severity: should_fix
---

**Rule.** When conditional statements evaluate complex or multi-clause compound
boolean expressions, extract the sub-expressions into descriptive const bool
local variables to document intent and make branching self-explanatory.
Conversely, do not break up an already clear compound condition into redundant
single-use intermediate variables when the inlined clauses are already explained
by per-clause comments or when planned secondary reuse was eliminated.

**Why.** Inlining opaque, multi-clause boolean logic directly inside if
statements obscures branching intent and makes compound conditions hard to
parse. Extracting sub-expressions into well-named const bool variables documents
the check without redundant comments, whereas introducing intermediate variables
for conditions that already have clear inline comments adds unnecessary
indirection.

## Examples

Bad:

```cpp
// Bad: Inlining complex multi-clause conditions in 'if' hides intent:
const bool swizzleDoesNotNeedGamutTransform =
        data.fReadSwizzle == ReadSwizzle::kRGBA || data.fReadSwizzle == ReadSwizzle::kRGB1;
if (!(data.fSteps.flags.linearize || data.fSteps.flags.encode ||
      data.fSteps.flags.gamut_transform) &&
    swizzleDoesNotNeedGamutTransform) { ... }

// Bad: Creating single-use intermediates when inlined clauses are commented:
const bool needsMsaaForTarget = desc.fSampleCount > SampleCount::k1 &&
                                targetInfo.sampleCount() == SampleCount::k1;
const bool needsMSAA = needsMsaaForTarget && !msaaRenderToSingleSampledSupport;
```

Good:

```cpp
// Good: Extract complex sub-expressions into descriptive const bool variables:
const bool xformNeedsGamutOrXferFn = data.fSteps.flags.linearize ||
                                     data.fSteps.flags.encode ||
                                     data.fSteps.flags.gamut_transform;
const bool swizzleNeedsGamutTransform = !(data.fReadSwizzle == ReadSwizzle::kRGBA ||
                                          data.fReadSwizzle == ReadSwizzle::kRGB1);
if (!(xformNeedsGamutOrXferFn || swizzleNeedsGamutTransform)) { ... }

// Good: Keep compound condition together when clauses have inline comments:
const bool needsMSAA = desc.fSampleCount > SampleCount::k1 &&         // pass uses MSAA
                       targetInfo.sampleCount() == SampleCount::k1 && // target is single-sampled
                       !msaaRenderToSingleSampledSupport;             // no extension available
```
