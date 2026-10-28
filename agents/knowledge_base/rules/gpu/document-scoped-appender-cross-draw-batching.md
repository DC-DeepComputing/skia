---
name: skia/gpu/document-scoped-appender-cross-draw-batching
description: When documenting the lifecycle of scoped draw appenders in DrawWriter, accurately describe cross-draw batching across consecutive draws instead of implying an immediate flush
rubric: readability
severity: should_fix
---

**Rule.** When documenting the lifecycle and count updates of scoped draw
appenders (such as DrawWriter::DynamicInstances or Appender subclasses),
accurately describe cross-draw batching across consecutive draw calls (such as
RenderStep::writeVertices). Clarify that accumulated instance or vertex counts
are combined across draws when pipeline state remains unchanged, rather than
implying that an appender's destruction forces or directly corresponds to an
immediate flush.

**Why.** DrawWriter aggregates vertex and instance data across multiple
RenderStep::writeVertices calls as long as the pipeline state remains invariant,
avoiding redundant draw calls and buffer binds. Documenting that scoped count
updates only prepare for an ensuing flush misleads readers into assuming an
appender's destruction triggers an immediate flush, obscuring how dynamic
instance batching operates across consecutive draw calls.

## Examples

Bad:

```cpp
// In the DrawWriter::DynamicInstances destructor:
// This occurs after all data appending for dynamic instances is finished.
// The update makes the final index count for these instances visible for the flush.
```

Good:

```cpp
// In the DrawWriter::DynamicInstances destructor:
// This occurs after all data appending for dynamic instances is finished. The update
// makes the final index count for these instances visible for the flush or combines
// the count with the next draw call's DynamicInstances object if there was no
// pipeline change between calls to RenderStep::writeVertices.
```
