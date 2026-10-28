---
name: skia/general-prefer-fallback-guards-over-preprocessor-early-retur
description: 'When initializing fallback chains across conditionally compiled #if blocks, guard subsequent blocks with if (!var) rather than repeating early returns'
rubric: readability
severity: nit
---

**Rule.** When initializing a fallback chain or cascading provider setup across
conditionally compiled #if defined(...) blocks, guard subsequent attempts with
if (!result) rather than inserting early returns (if (result) return;) inside
each preceding block. Keep each conditional compilation block self-contained and
uniform with a single trailing return at function exit.

**Why.** Repeating early returns across each preprocessor block introduces
repetitive boilerplate and scatters multiple exit points across conditional
branches, which is particularly undesirable in constructors or shared
initialization routines. Guarding subsequent attempts with if (!result) ensures
each fallback is evaluated only when needed while maintaining a clean, uniform
single exit point.

## Examples

Bad:

```cpp
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    fUnicode = SkUnicodes::ICU::Make();
    if (fUnicode) {
        return;
    }
#endif
#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
    fUnicode = SkUnicodes::ICU4X::Make();
    if (fUnicode) {
        return;
    }
#endif
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
    fUnicode = SkUnicodes::Libgrapheme::Make();
#endif
```

Good:

```cpp
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    fUnicode = SkUnicodes::ICU::Make();
#endif
#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
    if (!fUnicode) {
        fUnicode = SkUnicodes::ICU4X::Make();
    }
#endif
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
    if (!fUnicode) {
        fUnicode = SkUnicodes::Libgrapheme::Make();
    }
#endif
```
