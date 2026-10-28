---
name: skia/partition_alloc/distinct-template-trait-constants-in-stubs
description: When defining no-op or fallback stub implementations of template trait constants, assign distinct values rather than collapsing differing traits to zero
rubric: correctness
severity: should_fix
---

**Rule.** In no-op or fallback stub implementations of template classes that
take trait or policy constants, assign distinct values (such as unique bit
flags) to differing traits rather than collapsing them all to zero. Ensure
distinct traits produce distinct template instantiations across all build
configurations.

**Why.** Collapsing distinct trait or policy constants to zero in fallback
builds causes distinct template instantiations (such as raw_ptr<T, TraitA> and
raw_ptr<T, TraitB>) to collapse into identical concrete types. This allows
invalid cross-trait assignments or implicit conversions to compile undetected in
fallback configurations while failing in builds where the feature is enabled.

## Examples

Bad:

```cpp
// Fallback no-op stub:
using RawPtrTraits = int;
constexpr RawPtrTraits DisableDanglingPtrDetection = 0;
constexpr RawPtrTraits DanglingUntriaged = 0;
constexpr RawPtrTraits AllowPtrArithmetic = 0;

template <typename T, RawPtrTraits Traits = 0>
class raw_ptr { /* ... */ };
```

Good:

```cpp
// Fallback no-op stub:
using RawPtrTraits = int;
constexpr RawPtrTraits DisableDanglingPtrDetection = 1 << 0;
constexpr RawPtrTraits DanglingUntriaged = 1 << 0;
constexpr RawPtrTraits AllowPtrArithmetic = 1 << 1;

template <typename T, RawPtrTraits Traits = 0>
class raw_ptr { /* ... */ };
```
