---
name: skia/document-concrete-dispatch-artifacts
description: When documenting methods that finalize processing stages and dispatch to handlers or callbacks, explicitly name the concrete artifacts produced rather than vaguely stating data is pushed
rubric: readability
severity: nit
---

**Rule.** When documenting methods that finalize an intermediate processing
stage and dispatch results to a callback, listener, or handler, explicitly name
the concrete output artifacts produced (such as shaped blobs, vertices, or glyph
runs). Avoid vague phrasing that merely states data is pushed or dispatched
without identifying what is delivered.

**Why.** Comments that vaguely state data is pushed to a handler obscure what
artifacts an intermediate processing stage produces. Explicitly naming concrete
output artifacts clarifies the stage's purpose and makes data flow evident
across the pipeline without requiring readers to inspect the implementation.

## Examples

Bad:

```cpp
// Perform final adjustments and push to the blob handler.
void flushChunk(const SkSVGRenderContext& ctx);
```

Good:

```cpp
// Perform final adjustments and push shaped blobs to the callback.
void flushChunk(const SkSVGRenderContext& ctx);
```
