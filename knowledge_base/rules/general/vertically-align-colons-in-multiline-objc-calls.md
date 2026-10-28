---
name: skia/vertically-align-colons-in-multiline-objc-calls
description: When breaking Objective-C method calls across multiple lines, vertically align parameter colons across lines without spaces between colons and arguments
rubric: readability
severity: nit
---

**Rule.** When breaking Objective-C and Objective-C++ method calls across
multiple lines, align selector parameter colons vertically across all
continuation lines. Do not place spaces between the colons and their
corresponding argument expressions.

**Why.** Vertically aligning colons in multi-line message sends preserves visual
hierarchy and makes paired selector keywords and arguments instantly scannable,
conforming to standard Objective-C style conventions across Skia's Apple and
Metal backends.

## Examples

Bad:

```cpp
[(*fCommandEncoder) setVertexBuffer:buffer
                             offset:offset
                             atIndex:index];
```

Good:

```cpp
[(*fCommandEncoder) setVertexBuffer:buffer
                             offset:offset
                            atIndex:index];
```
