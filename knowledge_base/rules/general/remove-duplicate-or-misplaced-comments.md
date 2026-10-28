---
name: skia/remove-duplicate-or-misplaced-comments
description: When authoring or duplicating code blocks, variable declarations, or struct/interface fields, update or remove copy-pasted comments so they do not linger above unrelated logic
rubric: readability
severity: should_fix
---

**Rule.** When refactoring, duplicating, or copy-pasting sequential code blocks,
variable declarations, or struct/interface fields, ensure accompanying comments
and doc blocks accurately describe the immediately following logic or member.
Remove or update duplicate comments rather than leaving copy-pasted descriptions
above unrelated statements or properties where they describe a preceding
operation, adjacent field, or incorrect units/types.

**Why.** Leaving copy-pasted or duplicate comments above unrelated statements or
declarations creates misleading documentation, visual clutter, and contradicts
actual API semantics. Stale copied comments confuse consumers and maintainers
about whether the annotated field or statement performs the described operation
or represents different units and types.

## Examples

Bad:

```cpp
// Variant 1: Stale copy-pasted comment over sequential operations
SkString familyName;
proxy->getFamilyName(&familyName);

// The family name(s) are not reported.
// This would be very helpful for aliases, like "sans-serif", "Arial", etc.
STArray<4, SkLanguage> skLangs;
proxy->getLanguages(&skLangs);

// Variant 2: Copy-pasted doc comments on struct/interface fields
export interface TransformValue {
    /** The text string payload. */
    anchor: Point;
    /** Font size. */
    position: Point;
    /** The text string payload. */
    scale: Vector2;
}
```

Good:

```cpp
// Variant 1: Removed redundant/misplaced comment
SkString familyName;
proxy->getFamilyName(&familyName);

STArray<4, SkLanguage> skLangs;
proxy->getLanguages(&skLangs);

// Variant 2: Synchronized, accurate field doc comments
export interface TransformValue {
    /** Anchor point for transform (x, y). */
    anchor: Point;
    /** Position of transform (x, y). */
    position: Point;
    /** Scale of transform (x, y factors). */
    scale: Vector2;
}
```
