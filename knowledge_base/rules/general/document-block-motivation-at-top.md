---
name: skia/document-block-motivation-at-top
description: When documenting multi-step operations or parallel blocks, place high-level rationale above the sequence rather than burying it, and keep step comments adjacent
rubric: readability
severity: nit
---

**Rule.** When implementing a multi-step operation or sequence of parallel
blocks (such as distinct buffer types or fields), document high-level rationale
or shared invariants concisely at block entry before initiating work, rather
than burying explanations inside intermediate helper calls or an arbitrary
single instance. Conversely, place implementation comments explaining a specific
loop, sort key, or side-effect-triggering step (such as KeyContext creation,
toKey calls, or recorder flushes) immediately adjacent to that statement or loop
rather than hoisting them above outer function definitions or preceding
unrelated setup code.

**Why.** Placing comments at the exact scope of logic they describe ensures
rationale is immediately visible without obscuring nested control flow.
High-level block comments clarify overall intent and common invariants at entry
without misleading readers into thinking shared logic is unique to a single
instance, while placing operation-specific comments adjacent to their statements
prevents attributing side effects or internal steps to preceding unrelated setup
code.

## Examples

Bad:

```cpp
// Bad: Burying rationale inside intermediate helper calls or an arbitrary single instance
if (SkJpegEncoder::Encode(&buffer, pm, jpegOptions)) {
    // By decoding this back into an image, it can follow the fast path during drawImage.
    img = SkCodecs::DeferredImage(std::move(codec));
}
if (fInstances.fBuffer) {
    fInstances.fOffset = 0; // Reset offset/size for comparison against fBoundInstances.
}

// Bad: Hoisting step/side-effect comments above preceding unrelated setup code
// Determine paint ID and uniforms before recording; may trigger a Recorder flush.
if (!renderer || !renderer->emitsPrimitiveColor()) { primitiveBlender = nullptr; }
KeyContext keyContext{fRecorder, ...};
```

Good:

```cpp
// Good: Placing high-level rationale concisely at the top of the block sequence
// By encoding into jpeg, this follows the fast path during drawImage.
if (SkJpegEncoder::Encode(&buffer, pm, jpegOptions)) { ... }

// Calculate base offsets and reset offset/size for comparison against fBoundFoo.
uint32_t pendingBaseVertex = 0;
uint32_t pendingBaseInstance = 0;

// Good: Placing side-effect comments immediately adjacent to the executing statement
if (!renderer || !renderer->emitsPrimitiveColor()) { primitiveBlender = nullptr; }
// Determine paint ID and uniforms before recording; may trigger a Recorder flush.
KeyContext keyContext{fRecorder, ...};
```
