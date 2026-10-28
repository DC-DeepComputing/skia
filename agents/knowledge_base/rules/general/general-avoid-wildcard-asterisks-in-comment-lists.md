---
name: skia/general-avoid-wildcard-asterisks-in-comment-lists
description: When enumerating identifiers, flags, or settings in comments, list names explicitly rather than using trailing wildcard asterisks that resemble footnotes
rubric: readability
severity: nit
---

**Rule.** When documenting or enumerating identifiers, flags, configuration
options, or state properties in comments, list each name explicitly rather than
using trailing wildcard asterisks (such as 'prefix*'). Avoid trailing asterisks
on identifiers—especially in bulleted lists—where they can be confused with
footnote references, markdown emphasis, or placeholder markers.

**Why.** Trailing asterisks on identifier names in comments are ambiguous;
readers and reviewers can easily misinterpret them as footnote markers,
placeholder notes, or pointer types rather than glob patterns. Listing
identifiers explicitly clarifies exactly which settings or states are covered
and ensures the comment remains discoverable via code search.

## Examples

Bad:

```cpp
// The following state are constants in Graphite:
// * lineWidth
// * depthBias*
```

Good:

```cpp
// The following state are constants in Graphite:
// * lineWidth
// * depthBiasEnable, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor
```
