---
name: skia/prefer-branch-assertions-over-trailing-implication
description: Assert branch invariants inside unconditional else blocks instead of trailing implications, but express standalone conditional assertions using implication inside SkASSERT
rubric: readability
severity: should_fix
---

**Rule.** When checking invariants or handling remaining variants under
alternate branches of a conditional ladder, assert expected conditions directly
inside an unconditional else block using SkASSERT (or SkASSERT_RELEASE) rather
than writing trailing material-implication asserts, guarding assertions with
matching else if checks, or silently assuming unasserted variants. Conversely,
when asserting a standalone conditional invariant with no alternate branch,
express the check using logical implication inside SkASSERT(!condition ||
consequent) rather than wrapping SkASSERT in an if (condition) statement that
leaves an empty block in release builds.

**Why.** Placing assertions directly inside an unconditional else block ensures
alternate branch violations fail loudly instead of being silently bypassed or
obscured by boolean logic. Conversely, wrapping a standalone assertion in an if
statement leaves an empty block in release builds where SkASSERT is compiled
out, redundantly evaluating conditions at runtime; inlining the implication into
SkASSERT allows the entire check to compile away cleanly.

## Examples

Bad:

```cpp
// Trailing implication after branch:
if (!dstReadRequired) {
    fDstReadStrategy = DstReadStrategy::kNoneRequired;
}
SkASSERT(!dstReadRequired || fDstReadStrategy != DstReadStrategy::kNoneRequired);

// Guarding assertion with identical else-if condition:
if (isCompressed(format)) {
    supported |= TextureUsage::kCopyDst;
} else if (format != TextureFormat::kExternal) {
    SkASSERT(format != TextureFormat::kExternal);
}

// Standalone assertion wrapped in if statement:
if (source.isRGB888Format()) {
    SkASSERT(supportedColorType == kRGB_888x_SkColorType);
}
```

Good:

```cpp
// Direct assertion in unconditional else block:
if (!dstReadRequired) {
    fDstReadStrategy = DstReadStrategy::kNoneRequired;
} else {
    SkASSERT(fDstReadStrategy != DstReadStrategy::kNoneRequired);
}

// Invariant asserted inside unconditional else block:
if (isCompressed(format)) {
    supported |= TextureUsage::kCopyDst;
} else {
    SkASSERT(format != TextureFormat::kExternal);
}

// Logical implication inside SkASSERT (compiles out in release):
SkASSERT(!source.isRGB888Format() || supportedColorType == kRGB_888x_SkColorType);
```
