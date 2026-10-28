---
name: skia/guard-fast-path-against-zero-elements
description: When adding fast-path or optimization branches on variable-length inputs, guard with count > 0 before evaluating element predicates or computing size-offset opcodes
rubric: correctness
severity: should_fix
---

**Rule.** Guard optimization and fast-path branches against zero-element inputs
(such as count > 0 or numSlots > 0) before inspecting elements or emitting
operations. Do not rely on element-checking helper predicates without first
verifying non-zero count, as loops over empty inputs often evaluate vacuously to
true and opcode arithmetic like baseOp + count - 1 will underflow.

**Why.** Helper predicates that test element uniformity or splattability often
iterate from index 1 and evaluate vacuously to true when the element count is
zero. Executing optimization branches without checking count > 0 risks
underflowing offset-based opcode calculations (such as baseOp + count - 1),
dereferencing invalid buffer pointers, and emitting spurious instructions.

## Examples

Bad:

```cpp
// Optimization branch runs before verifying numSlots > 0:
if (basePtr) {
    // Evaluates vacuously to true when numSlots == 0:
    if (immutable_data_is_splattable(immutablePtr, numSlots)) {
        // Underflows to ProgramOp::copy_constant - 1:
        auto stage = (ProgramOp)((int)ProgramOp::copy_constant + numSlots - 1);
        pipeline->push_back({stage, immutablePtr});
        return;
    }
}
if (numSlots > 0) {
    // Standard unoptimized handling
}
```

Good:

```cpp
// Ensure numSlots > 0 guards the optimization and helper predicates:
if (numSlots > 0) {
    if (basePtr) {
        if (immutable_data_is_splattable(immutablePtr, numSlots)) {
            auto stage = (ProgramOp)((int)ProgramOp::copy_constant + numSlots - 1);
            pipeline->push_back({stage, immutablePtr});
            return;
        }
    }
    // Standard unoptimized handling
}
```
