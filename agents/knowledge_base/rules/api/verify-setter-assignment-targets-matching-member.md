---
name: skia/api/verify-setter-assignment-targets-matching-member
description: When implementing or updating setter methods and mutators, verify that assignments update the corresponding member field rather than mistakenly mutating an adjacent sibling variable
rubric: correctness
severity: should_fix
---

**Rule.** When implementing or updating setter methods, mutators, or parallel
state-assignment blocks, verify that each assignment updates the member variable
corresponding to the method or property name. Audit cloned or copy-pasted
setters to ensure the assignment statement, assertions, and parameter types all
refer to the intended target field rather than mutating an adjacent sibling
member.

**Why.** Copy-pasting adjacent accessor or mutator methods frequently leaves the
assignment statement untouched while method signatures and assertions are
updated, accidentally mutating a sibling member variable. This leaves the
intended target field unset or uninitialized while silently corrupting adjacent
state without generating compiler errors.

## Examples

Bad:

```cpp
void setMaskAction(skglyph::GlyphAction action) {
    using namespace skglyph;
    SkASSERT(static_cast<GlyphAction>(fMaskAction) == GlyphAction::kUnset);
    // Bug: copy-pasted setter mutates sibling field fSDFTAction instead of fMaskAction
    fSDFTAction = SkTo<uint32_t>(action);
}
```

Good:

```cpp
void setMaskAction(skglyph::GlyphAction action) {
    using namespace skglyph;
    SkASSERT(static_cast<GlyphAction>(fMaskAction) == GlyphAction::kUnset);
    fMaskAction = SkTo<uint32_t>(action);
}
```
