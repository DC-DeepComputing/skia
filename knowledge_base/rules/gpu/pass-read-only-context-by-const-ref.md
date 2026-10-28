---
name: skia/gpu/pass-read-only-context-by-const-ref
description: When passing read-only context or generator structures to helpers, pass by const reference rather than taking mutable references to move subfields before reallocation
rubric: readability
severity: should_fix
---

**Rule.** Pass read-only input, context, and generator data structures to helper
functions by const reference (const Context&) rather than by mutable reference
(Context&). Do not accept non-const references solely to std::move internal
fields (such as preamble strings or arrays) into member buffers when subsequent
operations (such as .reserve() or appending) will reallocate or copy the buffer
anyway.

**Why.** Taking non-const references to move internal fields breaks
const-correctness and destroys consistency across sibling helpers sharing the
same context data. Furthermore, moving a buffer immediately before calling
.reserve() on the destination reallocates heap memory and copies elements
anyway, negating any zero-copy benefit while risking use-after-move bugs when
reading sizes.

## Examples

Bad:

```cpp
// Bad: non-const reference taken to move subfield, but reserve() reallocates anyway
void ShaderInfo::generateVertexSkSL(const Caps* caps,
                                    const RenderStep* step,
                                    SharedGeneratorData& sharedData) {
    fVertexSkSL = std::move(sharedData.fSharedPreamble);
    fVertexSkSL.reserve(sharedData.fSharedPreamble.size() + vsPreamble.size() + 2);
    fVertexSkSL += "\n";
}
```

Good:

```cpp
// Good: pass read-only generator context by const reference
void ShaderInfo::generateVertexSkSL(const Caps* caps,
                                    const RenderStep* step,
                                    const SharedGeneratorData& sharedData) {
    fVertexSkSL.reserve(sharedData.fSharedPreamble.size() + vsPreamble.size() + 2);
    fVertexSkSL = sharedData.fSharedPreamble;
    fVertexSkSL += "\n";
}
```
