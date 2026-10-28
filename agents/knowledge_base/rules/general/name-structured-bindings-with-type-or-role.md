---
name: skia/name-structured-bindings-with-type-or-role
description: When using structured bindings (auto [...]), name decomposed variables explicitly to convey their concrete type or semantic role rather than ambiguous, single-letter, or colliding abbreviations
rubric: readability
severity: should_fix
---

**Rule.** When destructuring pairs, tuples, or structs using structured bindings
(auto [...] or const auto& [...]), name decomposed variables explicitly to
convey their concrete type (e.g., suffixing Rect for an SkRect) or
entity-specific semantic role (e.g., formatSupport instead of vague supports).
Avoid ambiguous, overly terse, or single-letter abbreviations (such as [l0, t0,
r0, b0] for bounding box coordinates vs. [u0, l0] for segment endpoints) that
obscure data types or collide with different concepts across adjacent scopes in
the same function; spell out identifiers (such as left0, top0, right0, bottom0)
instead. Keep decomposed variable names consistent across call sites of the same
helper.

**Why.** Structured bindings hide concrete types behind auto. Suffixing
decomposed variables with their concrete type or qualifying them with the
specific entity being represented (such as drawDstRect or formatSupport)
self-documents data flow and avoids confusion when interacting with related
concepts. Spelling out abbreviations prevents type confusion and identifier
collisions across scopes within the same function (such as distinguishing scalar
bounds from point endpoints).

## Examples

Bad:

```cpp
// Ambiguous name under auto deduction (could be SkRect, SkPoint, etc.)
auto [drawDst, shader] = SkImageShader::MakeForDrawRect(...);

// Vague identifier obscures which entity's capabilities are returned
auto [supports, sampleCounts] = this->getTextureSupport(format, Tiling::kOptimal);

// Cryptic abbreviations collide across scopes (l0 as scalar left vs Point lower)
{
    auto [l0, t0, r0, b0] = s0.bounds();
    if (r0 < l0) return false;
}
auto [u0, l0] = s0;
```

Good:

```cpp
// Type suffix 'Rect' explicitly documents that drawDstRect is an SkRect
auto [drawDstRect, shader] = SkImageShader::MakeForDrawRect(...);

// Entity-qualified 'formatSupport' clarifies the role across call sites
auto [formatSupport, sampleCounts] = this->getTextureSupport(format, Tiling::kOptimal);

// Spelled-out names clarify types and avoid abbreviation collisions across scopes
{
    auto [left0, top0, right0, bottom0] = s0.bounds();
    if (right0 < left0) return false;
}
auto [u0, l0] = s0;
```
