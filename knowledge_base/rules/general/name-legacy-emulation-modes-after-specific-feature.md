---
name: skia/name-legacy-emulation-modes-after-specific-feature
description: When defining enum variants, modes, or compatibility macros, name them after the specific emulated feature rather than ambiguous qualifiers like kLegacy, OLD, or NEW
rubric: readability
severity: should_fix
---

**Rule.** When defining enum values, internal modes, types, or preprocessor
macros for compatibility, emulation, or staged migrations, name the identifier
descriptively after the specific feature, subsystem, or library being emulated
or toggled (e.g., 'kEmbossSpecular' instead of 'kLegacySpecular', or
'SK_PARAGRAPH_LIBTXT_SPACES_RESOLUTION' instead of
'SK_PARAGRAPH_OLD_SPACES_RESOLUTION'). Avoid generic prefixes like 'kLegacy...'
or 'kCompat...' and ambiguous temporal qualifiers like 'OLD' or 'NEW' that
obscure what concrete system is being emulated or falsely imply baseline
behavior is deprecated, and document the compatibility purpose in a comment.

**Why.** Relative temporal qualifiers ('OLD', 'NEW') and vague prefixes
('kLegacy', 'kCompat') are ambiguous about which behavior is the intended
baseline and what subsystem or behavior is being emulated. Naming identifiers
after the concrete emulated feature or library clarifies intent, avoids
misleading users about code deprecation, and simplifies future maintenance and
retirement audits.

## Examples

Bad:

```cpp
enum class Type {
    kDiffuse,
    kSpecular,
    // Vague legacy identifier leaves historical purpose unclear:
    kLegacySpecular,
};

// Misleading temporal qualifier implies baseline behavior is deprecated:
#if defined(SK_PARAGRAPH_OLD_SPACES_RESOLUTION)
    bool isWhitespace8 = fParagraph->getUnicode()->isWhitespace(codepoint);
#else
    bool isWhitespace8 = false;
#endif
```

Good:

```cpp
enum class Type {
    kDiffuse,
    kSpecular,
    // Explicitly named after the legacy emboss filter emulation:
    kEmbossSpecular,
};

// Descriptively named after the specific compatibility mode toggled:
#if defined(SK_PARAGRAPH_LIBTXT_SPACES_RESOLUTION)
    bool isWhitespace8 = false;
#else
    bool isWhitespace8 = fParagraph->getUnicode()->isWhitespace(codepoint);
#endif
```
