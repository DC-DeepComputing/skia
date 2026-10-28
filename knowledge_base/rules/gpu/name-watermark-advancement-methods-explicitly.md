---
name: skia/gpu/name-watermark-advancement-methods-explicitly
description: When declaring methods that advance internal sequence counters or watermarks for collaborator subsystems, use explicit Next prefixes and operation qualifiers rather than bare update
rubric: readability
severity: nit
---

**Rule.** Do not use ambiguous method names like update<Property> (e.g.,
updateDepth) for methods that advance internal sequence counters, orders, or
watermarks on behalf of collaborator subsystems. Explicitly specify both the
forward-looking target being updated (such as NextDepth or NextOrder) and the
triggering context or subsystem operation (such as updateNextDepthForClipping).

**Why.** A bare name like updateDepth(depth) is ambiguous: callers cannot tell
whether it mutates the depth of an existing draw, sets a persistent device
property, or advances an internal watermark counter for subsequent draws.
Specifying Next<Property> alongside the triggering operation makes sequence
advancement explicit and clarifies that future draws are being coordinated with
external operations like clipping.

## Examples

Bad:

```cpp
// Ambiguous whether this mutates current draw depth or advances device watermark
void updateDepth(PaintersDepth depth);
```

Good:

```cpp
// Explicitly indicates the depth watermark for future draws advances for clipping
void updateNextDepthForClipping(PaintersDepth depth);
```
