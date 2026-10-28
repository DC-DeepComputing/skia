---
name: skia/tests/use-distinct-values-for-struct-fields
description: When testing structs, FFI bindings, or multi-property structures, mutate multiple properties concurrently and assign distinct values to each field to detect clobbering and transpositions
rubric: testing
severity: blocking
---

**Rule.** When authoring unit tests for structs, FFI mappings, serialization,
slot bindings, or multi-element structures (such as vectors, coordinates, and
tuples), assign distinct, unique values to each field or component rather than
reusing identical numbers or default values, ensuring swapped assignments or
copy-paste indexing bugs trigger assertion failures. Additionally, when testing
APIs or bindings that update multi-property structs or slot values, mutate and
verify multiple non-default properties concurrently rather than testing a single
property in isolation.

**Why.** Using identical values across fields or vector components (such as [50,
50]) masks copy-paste indexing errors and accidental transpositions. Similarly,
testing only a single property when mutating multi-property structs or bindings
masks scratch-buffer clobbering, field aliasing, and marshalling overwrite
defects during data transfer.

## Examples

Bad:

```cpp
// Bad: Struct fields share identical values; transposition cannot be detected
rust_icc::TransferFunction rust_tf;
rust_tf.e = 0.0f;
rust_tf.f = 0.0f;

// Bad: Vector components use identical values; masks index typos like reading arr[0] twice
expect(animation.setVec2Slot('ScaleGroup', [50, 50])).toBeTruthy();
expectArrayCloseTo(animation.getVec2Slot('ScaleGroup'), [50, 50], 4);

// Bad: Testing only one property masks scratch buffer clobbering across multiple fields
textProp.text = 'new text';
expect(animation.getTextSlot('TextSource').text).toBe('new text');
```

Good:

```cpp
// Good: Assign distinct values so accidental field transpositions are caught
rust_icc::TransferFunction rust_tf;
rust_tf.e = 0.0f;
rust_tf.f = 1.0f;

// Good: Use distinct component values to verify element ordering and indexing
expect(animation.setVec2Slot('ScaleGroup', [25, 50])).toBeTruthy();
expectArrayCloseTo(animation.getVec2Slot('ScaleGroup'), [25, 50], 4);

// Good: Mutate and verify multiple properties simultaneously to catch marshalling overwrites
textProp.text = 'new text';
textProp.fillColor = CanvasKit.CYAN;
textProp.strokeColor = CanvasKit.MAGENTA;
const newTextSlot = animation.getTextSlot('TextSource');
expect(newTextSlot.text).toBe('new text');
expectArrayCloseTo(newTextSlot.fillColor, CanvasKit.CYAN, 4);
expectArrayCloseTo(newTextSlot.strokeColor, CanvasKit.MAGENTA, 4);
```
