---
name: skia/generalize-utility-function-doc-comments
description: When documenting shared utilities and wrapper functions, describe general contracts rather than single callers on helpers, and detail concrete setup or adaptation performed by wrappers
rubric: readability
severity: should_fix
---

**Rule.** When documenting functions, methods, or shared utilities, describe
their general contract, mathematical invariants, and return value conditions
rather than framing comments solely around a single caller or specific data
type; mention specific callers only as supplementary context. When writing or
refactoring wrapper and adapter functions around underlying helpers, retain
variant-specific semantics on the wrappers and describe the concrete setup,
context preparation, or adaptation performed rather than merely stating that the
function wraps or delegates to another API.

**Why.** Restricting a shared helper's documentation to one caller misleads
readers into assuming it only supports that narrow scenario. Conversely, stating
that a wrapper merely delegates to an underlying API without explaining its
concrete adaptation or context preparation forces maintainers to inspect the
implementation to understand why the wrapper exists and how it interfaces with
the underlying system.

## Examples

Bad:

```cpp
// Variant 1: Shared utility tied to a single caller or data type
// Vertex buffers will explicitly require a certain alignment. To ensure correctness...
uint32_t align_to_req_min_lcm(uint32_t bytes, uint32_t req, uint32_t min);

// Variant 2: Specific doc comment left on generalized helper instead of wrappers
// Translates into copy_slots_unmasked (from values into temp stack) in Raster Pipeline.
void push_slots_or_immutable(SlotRange src, BuilderOp op);

// Variant 3: Wrapper comment merely stating it wraps another function
/**
 * Create a filtered version of 'src' using this filter. This is basically a wrapper
 * around filterImage that implements the MakeWithFilter API calls.
 */
sk_sp<SkSpecialImage> filterImage(...);
```

Good:

```cpp
// Variant 1: General contract and invariants across all contexts
// Buffers can explicitly require a certain alignment. To ensure correctness...
uint32_t align_to_req_min_lcm(uint32_t bytes, uint32_t req, uint32_t min);

// Variant 2: Specific doc comments placed on specialized wrappers, not shared helper
// Translates into copy_slots_unmasked (from values into temp stack) in Raster Pipeline.
void push_slots(SlotRange src) { push_slots_or_immutable(src, BuilderOp::push_slots); }

// Variant 3: Wrapper documents concrete context preparation and adaptation
/**
 * Create a filtered version of 'src' using this filter. This is basically a wrapper
 * around filterImage that prepares the skif::Context to filter the 'src' image directly,
 * for implementing the SkImages::MakeWithFilter API calls.
 */
sk_sp<SkSpecialImage> filterImage(...);
```
