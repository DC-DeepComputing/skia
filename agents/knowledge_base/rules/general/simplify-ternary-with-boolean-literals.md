---
name: skia/simplify-ternary-with-boolean-literals
description: When evaluating conditions or assertions with ternary expressions returning boolean literals, simplify them to standard boolean operations like !cond || expr or cond && expr
rubric: readability
severity: nit
---

**Rule.** Do not use ternary expressions that evaluate to boolean literals (such
as `cond ? expr : true` or `cond ? expr : false`). Simplify them to standard
boolean operations: use `!cond || expr` for conditional implication (`cond ?
expr : true`), `cond && expr` for conjunction (`cond ? expr : false`), and
`cond` or `!cond` when both branches are boolean literals.

**Why.** Ternary expressions that yield boolean literals introduce unnecessary
branching syntax for expressions that are cleanly expressed with standard
boolean operators (`||`, `&&`, `!`). Direct boolean operations are more
idiomatic in C++, eliminate redundant ternary syntax, and improve readability in
assertions, conditionals, and return statements.

## Examples

Bad:

```cpp
// Material implication in assertion
SkASSERT(isProtected ? (accessPattern != AccessPattern::kGpuOnlyCopySrc) : true);

// Redundant ternary returning boolean literals
bool isValid = hasCapability ? checkFormat() : false;
```

Good:

```cpp
// Material implication in assertion
SkASSERT(!isProtected || accessPattern != AccessPattern::kGpuOnlyCopySrc);

// Direct boolean conjunction
bool isValid = hasCapability && checkFormat();
```
