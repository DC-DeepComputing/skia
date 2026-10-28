---
name: skia/assert-remaining-counter-zero-after-chunk-processing
description: When consuming element batches, slots, or byte budgets across chunks in a loop, assert with SkASSERT that the remaining count or offset reaches zero
rubric: readability
severity: should_fix
---

**Rule.** When consuming elements, slots, bytes, or an offset budget across
chunks or ranges in a loop, assert with SkASSERT that the remaining count,
offset, or balance reaches zero upon loop exit (e.g., SkASSERT(remaining == 0)).
Do not leave chunked consumption loops without validating that the entire
allocated budget was accounted for.

**Why.** In loops that decrement an offset or process data in variable-sized
chunks, a calculation bug, unhandled tail element, or mismatched total size can
cause the loop to exit prematurely or overshoot the budget. Asserting that the
remaining balance reaches exactly zero acts as a postcondition check that
verifies accounting invariants, documents programmer intent, and catches
off-by-one or chunk-size errors early in debug builds.

## Examples

Bad:

```cpp
int offsetFromStackTop = out.slots.size();
for (const SlotRange& r : ranges) {
    gen->builder()->copy_stack_to_slots(r, offsetFromStackTop);
    offsetFromStackTop -= r.count;
}
// Missing assertion verifying that all slots were accounted for
```

Good:

```cpp
int offsetFromStackTop = out.slots.size();
for (const SlotRange& r : ranges) {
    gen->builder()->copy_stack_to_slots(r, offsetFromStackTop);
    offsetFromStackTop -= r.count;
}
SkASSERT(offsetFromStackTop == 0);
```
