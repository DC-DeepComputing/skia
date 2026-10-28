---
name: skia/tests/exercise-all-swappable-backend-implementations
description: 'When authoring tests or GMs for components with swappable implementations, exercise each available backend rather than selecting one via cascading #if fallbacks'
rubric: testing
severity: nit
---

**Rule.** Do not author unit tests or GMs that select only a single backend
implementation via an #if priority cascade (or early-return fallback chain) when
testing components with multiple swappable implementations (such as SkUnicode or
SkShaper providers). Instead, instantiate and run test cases across each enabled
backend implementation, or explicitly document deferred coverage with a tracking
TODO.

**Why.** Chaining #if defined(...) blocks or early returns only exercises
whichever backend happens to be listed first in the cascade when multiple
backends are enabled in the build. This leaves alternative compiled-in
implementations unexercised in CI and masks backend-specific regressions or
discrepancies.

## Examples

Bad:

```cpp
sk_sp<SkUnicode> get_unicode() {
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::ICU::Make()) { return unicode; }
#endif
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::Libgrapheme::Make()) { return unicode; }
#endif
    return nullptr;
}
DEF_GM(return new ParagraphGM(get_unicode());)
```

Good:

```cpp
// Exercise each available backend explicitly:
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
DEF_GM(return new ParagraphGM(SkUnicodes::ICU::Make());)
#endif
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
DEF_GM(return new ParagraphGM(SkUnicodes::Libgrapheme::Make());)
#endif
// If multi-backend parameterization is deferred, document it:
// TODO(bug): Make ParagraphGM work across all compiled SkUnicode providers.
```
