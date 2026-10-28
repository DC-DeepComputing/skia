---
name: skia/tests/capitalize-compound-proper-name-abbreviations
description: When abbreviating compound proper names or surnames in identifiers, capitalize each initial letter rather than using mixed lower-camel casing
rubric: readability
severity: should_fix
---

**Rule.** When abbreviating compound proper names or multi-surname algorithms
(such as Liang-Barsky) in variable, parameter, or helper function identifiers,
capitalize each initial letter (e.g., use 'LB', not 'Lb'). Do not format
multi-name initialisms in mixed lower camelCase as if they were a single word.

**Why.** In compound names or multi-author algorithms, each letter in the
initialism represents a separate proper noun. Lowercasing subsequent initials
(such as 'Lb') obscures the proper name boundaries, suggests a single camelCase
word, and risks confusion with standard units or words (such as 'lb' for
pounds).

## Examples

Bad:

```cpp
// Bad: 'Lb' lowercases the initial of Barsky and looks like 'pounds'.
bool matchesLb = false;
auto expectedClipLb = reference_clip(line, minX, minY, maxX, maxY);
```

Good:

```cpp
// Good: 'LB' preserves the capitalized initials of both Liang and Barsky.
bool matchesLB = false;
auto expectedClipLB = reference_clip(line, minX, minY, maxX, maxY);
```
