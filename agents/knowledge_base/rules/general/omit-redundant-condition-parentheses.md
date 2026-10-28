---
name: skia/omit-redundant-condition-parentheses
description: When writing conditions in if, while, or SkASSERT statements, omit redundant outer parentheses wrapping the expression unless required for operator precedence or assignment
rubric: readability
severity: nit
---

**Rule.** Do not wrap entire conditional expressions in `if`, `else if`,
`while`, or assertion macros like `SkASSERT` with redundant outer parentheses.
Preserve extra parentheses only when required for operator precedence between
sub-expressions or when intentionally enclosing an assignment in a condition to
suppress compiler warnings.

**Why.** Redundant outer parentheses add visual clutter to control flow and
assertion statements without altering evaluation order or precedence. Skia style
avoids superfluous parentheses around boolean expressions, and formatters do not
strip them automatically.

## Examples

Bad:

```cpp
if ((a == b && c == d)) {
    do_something();
} else if ((imgData.fTileModes.first == SkTileMode::kClamp &&
            imgData.fTileModes.second == SkTileMode::kClamp)) {
    handle_clamp();
}

SkASSERT((s0.upper().y <= s1.upper().y));
```

Good:

```cpp
if (a == b && c == d) {
    do_something();
} else if (imgData.fTileModes.first == SkTileMode::kClamp &&
           imgData.fTileModes.second == SkTileMode::kClamp) {
    handle_clamp();
}

SkASSERT(s0.upper().y <= s1.upper().y);
```
