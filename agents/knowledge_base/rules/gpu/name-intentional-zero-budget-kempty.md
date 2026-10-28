---
name: skia/gpu/name-intentional-zero-budget-kempty
description: When creating GPU ResourceProviders for operations that do not allocate budgeted resources, pass a zero budget and name the constant kEmpty... rather than kDefault...
rubric: readability
severity: should_fix
---

**Rule.** When creating temporary or auxiliary GPU ResourceProviders for
operations that do not allocate budgeted GPU resources (such as precompilation),
specify a cache budget of 0 bytes rather than an arbitrary non-zero budget. When
defining constants for intentionally zero cache budgets or capacity limits, name
the constant with a kEmpty... or kZero... prefix (e.g., kEmptyBudget) rather
than kDefault....

**Why.** Operations like pipeline precompilation produce objects stored in
global caches rather than budgeted buffers or textures in ResourceCache, making
non-zero budgets misleading. Naming an intentional zero budget kDefault...
falsely implies that the subsystem normally defaults to unbudgeted or disabled
caching, whereas kEmpty... or kZero... clearly signals an intentional zero
invariant.

## Examples

Bad:

```cpp
// Bad: Arbitrary non-zero budget for operations that do not allocate budgeted resources.
static constexpr size_t kDefaultBudgetInBytes = 256 * (1 << 20);
auto provider = sharedContext->makeResourceProvider(&singleOwner, SK_InvalidGenID, kDefaultBudgetInBytes);

// Bad: 'kDefault...' misleadingly implies the subsystem's standard default budget is zero.
static constexpr size_t kDefaultBudgetInBytes = 0;
auto provider = sharedContext->makeResourceProvider(&singleOwner, SK_InvalidGenID, kDefaultBudgetInBytes);
```

Good:

```cpp
// Good: Pass zero budget for unbudgeted operations and name constant kEmpty... / kZero...
static constexpr size_t kEmptyBudget = 0;
auto provider = sharedContext->makeResourceProvider(&singleOwner, SK_InvalidGenID, kEmptyBudget);
```
