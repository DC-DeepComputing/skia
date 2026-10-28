---
name: skia/api/keep-client-specific-helpers-out-of-shared-interfaces
description: When implementing client-specific helper logic using public APIs, keep helpers internal to the consuming component rather than expanding shared interfaces
rubric: design
severity: should_fix
---

**Rule.** Do not add specialized or client-specific helper methods to shared or
public module interfaces (such as SkUnicode or SkShaper) when the logic can be
implemented using existing public APIs. Keep client-specific convenience
routines, text/data processors, or parsing helpers internal to the consuming
component (such as SkParagraph) as private class methods, static functions, or
anonymous-namespace utilities.

**Why.** Shared module interfaces define general, reusable contracts across
multiple consumers. Adding one-off helpers tailored to a single caller pollutes
public interfaces, creates long-term maintenance overhead, and locks shared
abstractions into narrow or incomplete semantics that only make sense for that
specific client.

## Examples

Bad:

```cpp
// modules/skunicode/include/SkUnicode.h
class SKUNICODE_API SkUnicode : public SkRefCnt {
public:
    // Specialized helper tailored specifically to OneLineShaper's requirements:
    SkUnichar getEmojiSequenceStart(const char** begin, const char* end);
};
```

Good:

```cpp
// modules/skparagraph/src/OneLineShaper.cpp
// Implement helper privately within the consuming component using existing public APIs:
SkUnichar OneLineShaper::getEmojiSequenceStart(SkUnicode* unicode,
                                              const char** begin,
                                              const char* end) {
    ...
}
```
