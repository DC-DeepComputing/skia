---
name: skia/modules/name-line-break-flags-by-break-priority
description: When defining line-break opportunity flags or code-unit classification properties in text layout, name flags after break-priority tiers rather than specific characters
rubric: readability
severity: should_fix
---

**Rule.** Name line-break opportunity flags, code-unit classification enums, and
related query predicates after their break category or priority hierarchy (such
as kPartOfIntraWordBreak, kPartOfWhiteSpaceBreak, or isIntraWordBreak()) rather
than specific character classes (such as kPartOfNonBreakingSpace). Align flag
naming with the break tiers established in Unicode text segmentation and
paragraph wrapping models.

**Why.** Line-breaking and text-wrapping algorithms evaluate opportunities along
a break-priority hierarchy (e.g. hard break, whitespace break, intra-word break,
grapheme break). Naming a flag after a specific character class obscures its
role in the line-wrapping priority model, prevents other code points or
hyphenation points that share the same breaking tier from reusing the flag, and
creates inconsistent terminology across segmentation backends.

## Examples

Bad:

```cpp
enum CodeUnitFlags {
    kPartOfWhiteSpace = 0x01,
    kGraphemeStart = 0x02,
    kSoftLineBreakBefore = 0x04,
    kHardLineBreakBefore = 0x08,
    kPartOfNonBreakingSpace = 0x10,
};
```

Good:

```cpp
enum CodeUnitFlags {
    kPartOfWhiteSpaceBreak = 0x01,
    kGraphemeStart = 0x02,
    kSoftLineBreakBefore = 0x04,
    kHardLineBreakBefore = 0x08,
    kPartOfIntraWordBreak = 0x10,
};
```
