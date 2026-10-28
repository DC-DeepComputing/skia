---
name: skia/gpu/use-compressed-painters-order
description: When tracking draw sequencing, layer ordering, or depth in Graphite, use strongly typed wrappers like CompressedPaintersOrder instead of raw uint32_t
rubric: readability
severity: should_fix
---

**Rule.** In Graphite, represent draw sequencing, layer ordering, and painter's
depth using dedicated strongly typed monotonic wrappers (such as
`CompressedPaintersOrder` or `PaintersDepth`) rather than primitive integer
types like `uint32_t`. Pass and store ordering values using these types to
prevent ordinal domain mismatches and make sequencing semantics explicit.

**Why.** Graphite defines `CompressedPaintersOrder`, `PaintersDepth`, and
`DisjointStencilIndex` as `MonotonicValue` wrappers to guarantee type safety
across distinct ordering domains. Using raw integers like `uint32_t` obscures
sequencing semantics, permits accidental arithmetic or interchange between
unrelated domains, and bypasses monotonic comparison invariants.

## Examples

Bad:

```cpp
// Bad: Raw integer obscures painter's order semantics and allows domain confusion
struct Layer {
    Layer(uint32_t& index) : fIndex(index++) {}
    const uint32_t fIndex;
};
void recordDraw(uint32_t order);
```

Good:

```cpp
// Good: Strongly typed monotonic wrapper enforces type safety and ordering intent
struct Layer {
    Layer(const CompressedPaintersOrder& order) : fOrder(order) {}
    const CompressedPaintersOrder fOrder;
};
void recordDraw(CompressedPaintersOrder order);
```
