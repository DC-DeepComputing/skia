---
name: skia/prefix-commit-message-title-with-subsystem
description: When authoring commit message titles, prefix the subject line with a bracketed lowercase subsystem tag such as [graphite], [ganesh], or [sksl]
rubric: readability
severity: should_fix
---

**Rule.** Prefix the commit message subject line with a bracketed lowercase
subsystem tag (e.g., `[graphite]`, `[ganesh]`, `[sksl]`) indicating the primary
subsystem modified. Keep the tag concise and lowercase, followed by a space
before the subject description.

**Why.** Subsystem tags in commit subjects categorize changes across git
history, mailing lists, and notification logs, allowing developers and tooling
to quickly identify which component or backend is modified without inspecting
the full diff.

## Examples

Bad:

```cpp
// Missing subsystem tag in commit title:
Add isTracked checks to Graphite CommandBuffers

// Uppercase or unbracketed subsystem prefix:
GRAPHITE: Add isTracked checks to CommandBuffers
```

Good:

```cpp
// Lowercase subsystem tag in brackets:
[graphite] Add isTracked checks to CommandBuffers
[ganesh] Support external textures in GrDirectContext
[sksl] Optimize vector swizzle expressions
```
