---
name: skia/general-abbreviate-original-as-orig-not-ori
description: When naming identifiers representing original values or entities, use 'orig' or spell out 'original'; avoid 'ori' to prevent confusion with origin or orientation
rubric: readability
severity: nit
---

**Rule.** When abbreviating 'original' in variable, parameter, or field
identifiers, use 'orig' (e.g., origTop, origSegment) or spell out 'original'. Do
not use non-standard abbreviations such as 'ori'.

**Why.** In graphics and geometry code, the abbreviation 'ori' is ambiguous and
easily mistaken for coordinate 'origin' or 'orientation'. Standardizing on
'orig' or 'original' avoids confusion and aligns with established Skia naming
conventions.

## Examples

Bad:

```cpp
Vertex* oriTop = edge->fTop;
Vertex* oriBottom = edge->fBottom;
Segment* oriSegment = fAlloc->make<Segment>(oriTop, oriBottom);
```

Good:

```cpp
Vertex* origTop = edge->fTop;
Vertex* origBottom = edge->fBottom;
Segment* origSegment = fAlloc->make<Segment>(origTop, origBottom);
```
