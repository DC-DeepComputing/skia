---
name: skia/avoid-dangling-else-across-preprocessor-blocks
description: 'When conditionally compiling independent feature or backend blocks (like SK_GANESH and SK_GRAPHITE), keep each block self-contained and avoid dangling ''else'' across #endif boundaries'
rubric: correctness
severity: should_fix
---

**Rule.** Do not chain an 'else' across independent preprocessor conditional
blocks (#if ... #endif) for optional build flags or backends (such as SK_GANESH
and SK_GRAPHITE). Keep each conditionally compiled block syntactically
self-contained with its own closed statement, or use separate independent if
checks so that disabling one feature flag cannot leave a dangling 'else'
attached to subsequent code.

**Why.** When an 'else' is placed at the end of an #if block immediately before
#endif, compiling with subsequent optional feature blocks disabled causes the
preprocessor to strip the intervening code. The dangling 'else' then attaches
directly to whatever unconditional statement follows, corrupting control flow
specifically under that build configuration.

## Examples

Bad:

```cpp
// Bad: if SK_GRAPHITE is disabled, 'else' dangles onto 'if (texImage)'
#if defined(SK_GANESH)
if (dContext) {
    texImage = SkImages::TextureFromImage(dContext, image, ...);
} else
#endif
#if defined(SK_GRAPHITE)
if (recorder) {
    texImage = SkImages::TextureFromImage(recorder, image, ...);
}
#endif
if (texImage) {
    canvas->drawImage(texImage, x, y);
}
```

Good:

```cpp
// Good: each conditional block is self-contained without dangling control flow
#if defined(SK_GANESH)
if (dContext) {
    texImage = SkImages::TextureFromImage(dContext, image, ...);
}
#endif
#if defined(SK_GRAPHITE)
if (recorder) {
    texImage = SkImages::TextureFromImage(recorder, image, ...);
}
#endif
if (texImage) {
    canvas->drawImage(texImage, x, y);
}
```
