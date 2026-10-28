---
name: skia/prefer-direct-boolean-assignment
description: When initializing or updating boolean flags from conditions, bitmasks, or composite capabilities, assign boolean expressions directly instead of branching or re-evaluating prerequisites
rubric: readability
severity: should_fix
---

**Rule.** Prefer direct boolean assignment (`flag = condition;`, or `flag =
SkToBool(mask & kBit);` for bitmasks) over branching constructs like `if
(!condition) { flag = false; }` or `if/else` ladders when setting or deriving
boolean flags. Similarly, when a composite capability or policy flag already
incorporates a prerequisite check, assign the combined boolean expression
directly (e.g., `flag = base || composite;`) rather than re-evaluating the
prerequisite inside branched fallback logic. Compute and initialize boolean
flags directly upfront before branching on combinations of them, eliminating
unnecessary control flow and flattening subsequent conditionals.

**Why.** Direct boolean assignment eliminates repetitive branching boilerplate,
avoids redundant bitwise evaluations or prerequisite re-evaluations, and
flattens nested conditionals, making flag initialization and downstream logic
concise, cohesive, and easy to read.

## Examples

Bad:

```cpp
// Variant 1: Conditional flag synchronization
if (!exts.fSynchronization2KHR) {
    toAdd.fSynchronization2 = false;
}

// Variant 2: Nested branching instead of upfront bitmask flag assignment
if (!(formatFeatures & VK_FORMAT_FEATURE_SEPARATE_RECONSTRUCTION_FILTER_BIT)) {
    if (!(formatFeatures & VK_FORMAT_FEATURE_FILTER_LINEAR_BIT)) {
        fChromaFilter = VK_FILTER_NEAREST;
    }
    fSamplerFilterMustMatchChromaFilter = true;
}
fSupportsLinearFilter = SkToBool(formatFeatures & VK_FORMAT_FEATURE_FILTER_LINEAR_BIT);

// Variant 3: Branched fallback re-evaluating prerequisite included in composite flag
if (this->glCaps().strictProtectedness()) {
    texDesc.fIsProtected = skgpu::Protected(isProtected == skgpu::Protected::kYes ||
                                            this->glCaps().supportsProtectedContent());
} else {
    texDesc.fIsProtected = isProtected;
}
```

Good:

```cpp
// Variant 1: Direct boolean assignment
toAdd.fSynchronization2 = exts.fSynchronization2KHR;

// Variant 2: Compute boolean flags upfront from bitmasks
fSamplerFilterMustMatchChromaFilter =
        !SkToBool(formatFeatures & VK_FORMAT_FEATURE_SEPARATE_RECONSTRUCTION_FILTER_BIT);
fSupportsLinearFilter = SkToBool(formatFeatures & VK_FORMAT_FEATURE_FILTER_LINEAR_BIT);
if (fSamplerFilterMustMatchChromaFilter && !fSupportsLinearFilter) {
    fChromaFilter = VK_FILTER_NEAREST;
}

// Variant 3: Direct boolean assignment using composite capability flag
texDesc.fIsProtected = skgpu::Protected(isProtected == skgpu::Protected::kYes ||
                                        this->glCaps().strictProtectedness());
```
