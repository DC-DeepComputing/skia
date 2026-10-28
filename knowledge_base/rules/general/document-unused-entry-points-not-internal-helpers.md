---
name: skia/document-unused-entry-points-not-internal-helpers
description: When documenting that functions or methods are currently unused, annotate the public entry points lacking consumers rather than internal helper functions called by them
rubric: readability
severity: should_fix
---

**Rule.** When documenting that code is currently unused or uncalled, place the
explanatory comment on the public entry points or callers that lack consumers
rather than on internal helper functions actively invoked by those entry points.
Keep internal helper declarations free of misleading unused annotations when
other methods within the class call them.

**Why.** Marking an internal helper as unused is misleading and directly
contradicts the implementation when sibling methods or entry points actively
call it. Documenting unused status at the public entry points accurately
identifies where external callers are absent and avoids confusing maintainers
auditing call sites or searching for dead code.

## Examples

Bad:

```cpp
// Bad: Marking internal helper as unused when member functions call it.
void draw(BindBufferInfo vertices, unsigned int vertexCount) {
    this->bindAndFlush({}, {}, vertices, 0, vertexCount);
}

// Currently unused
void bindAndFlush(BindBufferInfo staticData, BindBufferInfo indices, BindBufferInfo appendData,
                  uint32_t templateCount, unsigned int drawCount);
```

Good:

```cpp
// Good: Annotate the public entry points that lack callers.
// Issues draws with fully specified data. These are currently unused.
void draw(BindBufferInfo vertices, unsigned int vertexCount) {
    this->bindAndFlush({}, {}, vertices, 0, vertexCount);
}

void bindAndFlush(BindBufferInfo staticData, BindBufferInfo indices, BindBufferInfo appendData,
                  uint32_t templateCount, unsigned int drawCount);
```
