---
name: skia/factor-repeated-search-fallback-logic-into-helpers
description: When sequential search loops or fallback stages duplicate candidate validation or diagnostic logging, factor the repeated logic into a parameterized helper function
rubric: design
severity: nit
---

**Rule.** When consecutive search loops, fallback stages, or lookup branches
execute repeated candidate validation, property checks, or diagnostic logging,
extract the common logic into a shared helper function (or extend an existing
validation helper with differentiating context, such as a scope or stage tag).
Avoid duplicating identical logging blocks and predicate tests at each fallback
step.

**Why.** Duplicating validation and diagnostic logging across multiple search or
fallback stages inflates code size, clutters search flow, and risks inconsistent
logging or behavioral drift when criteria change. Centralizing evaluation in a
parameterized helper keeps search stages concise and ensures uniform
diagnostics.

## Examples

Bad:

```cpp
// Bad: Duplicating verbose logging and validation across search loops
for (int i = 0; i < fNameToFamilyMap.size(); ++i) {
    if (has_locale_and_character(face, langTag, character)) {
        if constexpr (kVerbose) {
            SkDebugf("Found U+%" PRIx32 " in %s : %s\n", character, name.c_str(), langTag.c_str());
        }
        return face;
    }
}
for (int i = 0; i < fFallbackFamilies.size(); ++i) {
    if (has_locale_and_character(face, langTag, character)) {
        if constexpr (kVerbose) {
            SkDebugf("Found U+%" PRIx32 " in %s : %s\n", character, name.c_str(), langTag.c_str());
        }
        return face;
    }
}
```

Good:

```cpp
// Good: Parameterize helper with scope context to deduplicate logging and checks
static bool has_locale_and_character(SkTypeface* face, const SkString& langTag,
                                     SkUnichar character, const char* scope) {
    if (!face || !matches_lang(face, langTag)) {
        return false;
    }
    if constexpr (kVerbose) {
        SkDebugf("Found U+%" PRIx32 " in %s : %s in scope %s\n",
                 character, name.c_str(), langTag.c_str(), scope);
    }
    return face->unicharToGlyph(character) != 0;
}

// Call sites pass differentiating stage/scope name:
if (has_locale_and_character(face, langTag, character, "family")) { return face; }
if (has_locale_and_character(face, langTag, character, "fallback")) { return face; }
```
