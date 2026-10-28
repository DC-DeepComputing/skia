---
name: skia/api/document-cpp-contracts-not-downstream-bindings
description: When documenting C++ structs, pointer lengths, or buffer counts, describe the native C++ contract rather than downstream wrapper or binding behavior
rubric: readability
severity: should_fix
---

**Rule.** When documenting C++ APIs, structs, or callbacks—especially pointer
lengths, buffer capacities, or array bounds—ensure comments describe the native
C++ implementation contract rather than downstream language bindings or wrapper
adaptations (such as CanvasKit, Emscripten, or JNI). Keep wrapper-specific
transformations, ergonomics, or synthetic sentinel elements documented in the
binding layer rather than in core C++ declarations.

**Why.** Downstream wrappers or language bindings may reshape, pad, or append
trailing elements to buffers for ergonomics in target languages like JavaScript.
Describing wrapper-specific behavior in C++ header comments creates misleading
contracts and hazards, risking out-of-bounds reads or buffer overruns by C++
consumers.

## Examples

Bad:

```cpp
// Visitor struct describing glyph run geometry.
struct VisitorInfo {
    int             count;
    const uint16_t* glyphs;     // count values
    const SkPoint*  positions;  // count+1 values (downstream JS binding appends end position)
    const uint32_t* utf8Starts; // count+1 values
};
```

Good:

```cpp
// Visitor struct describing glyph run geometry.
struct VisitorInfo {
    int             count;
    const uint16_t* glyphs;     // count values
    const SkPoint*  positions;  // count values
    const uint32_t* utf8Starts; // count+1 values (includes end offset)
};
```
