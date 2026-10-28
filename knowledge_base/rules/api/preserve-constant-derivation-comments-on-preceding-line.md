---
name: skia/api/preserve-constant-derivation-comments-on-preceding-line
description: When hardcoding constants derived from non-constexpr formulas or external limits, place derivation comments on the preceding line rather than deleting them to satisfy line-length limits
rubric: readability
severity: nit
---

**Rule.** When a constant must be hardcoded (such as to avoid circular header
dependencies or non-constexpr expressions), document its derivation, formula, or
relationship to other constants in an explanatory comment. If a trailing inline
comment exceeds or approaches the project's 100-column line-length limit, place
the comment on the line immediately preceding the declaration rather than
deleting it or leaving an overlong line.

**Why.** Hardcoded numeric constants obscure the mathematical relationships or
cross-component limits they represent. Preserving the derivation formula in a
preceding comment maintains essential context for future maintenance while
cleanly respecting Skia's 100-column line-wrapping convention.

## Examples

Bad:

```cpp
// Bad: deleting derivation comment, or leaving an overlong trailing inline comment:
static constexpr Index kInvalidIndex = 4096;

// or:
static constexpr Index kInvalidIndex = 4096; // 1 << SkNextLog2_portable(DrawList::kMaxRenderSteps);
```

Good:

```cpp
// Good: place the derivation comment on the preceding line to preserve context and stay within column limits:
// 1 << SkNextLog2_portable(DrawList::kMaxRenderSteps);
static constexpr Index kInvalidIndex = 4096;
```
