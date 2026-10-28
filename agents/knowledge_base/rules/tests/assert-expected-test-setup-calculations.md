---
name: skia/tests/assert-expected-test-setup-calculations
description: When deriving test scenario capacities or limits from multiple subsystem constants, assert that the calculated value matches expected quantities using SkASSERT or static_assert
rubric: readability
severity: should_fix
---

**Rule.** When test setup derives scenario limits, capacities, or item counts
from combinations of constants and subsystem properties (such as grid dimensions
multiplied by maximum pages), assert that the computed quantity matches the
expected test scenario value using SkASSERT or static_assert. Do not leave
multi-constant capacity derivations unasserted.

**Why.** Deriving test capacities or iteration counts from external subsystem
constants creates an implicit dependency on those definitions. Asserting the
calculated quantity guarantees that future changes to subsystem limits or
configuration formulas fail fast at test setup rather than silently altering
test coverage or causing confusing downstream failures.

## Examples

Bad:

```cpp
constexpr int numPlots = 2;
constexpr int maxPages = skgpu::PlotLocator::kMaxMultitexturePages;
constexpr int totalPlots = numPlots * numPlots * maxPages;

// totalPlots assumed to be 16, but unverified
std::vector<skgpu::AtlasLocator> locators(totalPlots);
```

Good:

```cpp
constexpr int numPlots = 2;
constexpr int maxPages = skgpu::PlotLocator::kMaxMultitexturePages;
constexpr int totalPlots = numPlots * numPlots * maxPages;

std::vector<skgpu::AtlasLocator> locators(totalPlots);
SkASSERT(totalPlots == 16);
```
