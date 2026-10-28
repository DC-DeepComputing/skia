---
name: skia/api/parenthesize-exclusion-clauses-in-doc-comments
description: When writing doc comments or function descriptions with exclusion clauses or exceptions, enclose the exclusions in parentheses to prevent modifier ambiguity
rubric: readability
severity: nit
---

**Rule.** In doc comments and function descriptions that detail operations or
lists alongside exclusions or exceptions (such as 'excluding' or 'except for'),
enclose the exclusion clause in parentheses. Do not append exclusion lists with
bare commas where trailing items can be ambiguously parsed as additional primary
actions rather than excluded items.

**Why.** Appending an exclusion list to an action description with commas (e.g.,
'Attaches A, excluding B, C, and D') leaves it grammatically ambiguous whether
trailing items are part of the action or part of the exclusion. Enclosing the
exclusion clause in parentheses explicitly scopes the modifier and eliminates
grammatical ambiguity.

## Examples

Bad:

```cpp
// Attaches the layer content, excluding motion blur, top level layer controller, and mattes.
sk_sp<sksg::RenderNode> buildContentTree(const AnimationBuilder&, CompositionBuilder*);
```

Good:

```cpp
// Attaches the layer content (excluding motion blur, layer controller, and mattes).
sk_sp<sksg::RenderNode> buildContentTree(const AnimationBuilder&, CompositionBuilder*);
```
