---
name: skia/modules/indicate-non-rendering-runs-in-text-visitors
description: When yielding text runs in visitor callbacks that include non-rendering blocks such as trailing whitespace, provide metadata flags distinguishing them from drawable runs
rubric: correctness
severity: should_fix
---

**Rule.** When exposing or dispatching text runs in visitor or callback APIs
that yield non-rendering blocks (such as trailing whitespace or ghost clusters
used for caret positioning), include metadata flags or an explicit indicator in
the run record. Ensure consumers can reliably distinguish layout-only blocks
from visual, drawable glyph runs.

**Why.** Text run visitors are utilized both for rendering glyphs and for
calculating caret navigation or selection bounds across character offsets.
Yielding non-rendering blocks without distinguishing flags forces consumers to
guess or improperly render phantom glyphs, whereas explicit flags allow
renderers to skip non-drawing runs while preserving correct layout geometry.

## Examples

Bad:

```cpp
visitor({
    run.font,
    rec.fOffset,
    run.count,
    run.glyphs,
    run.positions,
    clusterPtr,
    // Bad: consumers cannot tell if this is a drawable run or trailing whitespace
});
```

Good:

```cpp
enum VisitorFlags {
    kWhiteSpace_VisitorFlag = 1 << 0,
};
struct VisitorInfo {
    const SkFont&   font;
    SkPoint         origin;
    SkScalar        advanceX;
    int             count;
    const uint16_t* glyphs;
    const SkPoint*  positions;
    const uint32_t* utf8Starts;
    unsigned        flags; // Distinguishes layout-only blocks from visual runs
};
```
