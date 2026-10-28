---
name: skia/prefer-skspan-subspan-over-last
description: When dropping a prefix of elements from an SkSpan or std::span, use span.subspan(offset) instead of span.last(span.size() - offset)
rubric: readability
severity: should_fix
---

**Rule.** When advancing a span or dropping a consumed prefix of elements from
an SkSpan (or std::span), use span.subspan(offset) rather than
span.last(span.size() - offset). Avoid manually calculating span.size() - offset
to obtain the suffix subspan.

**Why.** SkSpan::subspan(offset) directly returns a subspan from the specified
element offset to the end of the span. Computing span.size() - offset and
passing it to last() is redundant, less readable, and obscures the intent of
advancing through the span.

## Examples

Bad:

```cpp
pts = pts.last(pts.size() - rc->fPointsConsumed);
vbs = vbs.last(vbs.size() - rc->fVerbsConsumed);
```

Good:

```cpp
pts = pts.subspan(rc->fPointsConsumed);
vbs = vbs.subspan(rc->fVerbsConsumed);
```
