---
name: skia/prefer-empty-over-size-zero
description: When checking container or string emptiness, call .empty() or !.empty() instead of comparing .size() or .length() against zero, and use .empty() without leftover comparisons
rubric: readability
severity: should_fix
---

**Rule.** When testing whether a container, span, string, or collection is empty
or non-empty, prefer calling container.empty() or !container.empty() instead of
comparing container.size() or string.length() against zero (such as size() == 0,
length() == 0, size() > 0, or size() != 0) or evaluating size() as a boolean
(such as !container.size() or if (container.size())). Additionally, evaluate the
boolean result of .empty() directly without retaining leftover relational
comparisons against integer literals (such as !container.empty() > 0).

**Why.** Calling .empty() directly conveys the intent of checking emptiness
rather than performing an arithmetic comparison or relying on implicit
integer-to-boolean conversion, and .empty() is guaranteed O(1) across standard
and custom container types. Retaining relational comparison operators like > 0
after refactoring to .empty() relies on confusing implicit bool-to-int
conversion and introduces redundant syntax.

## Examples

Bad:

```cpp
if (items.size() == 0) {
    return;
}
if (structNamePrefix.length() == 0) {
    return false;
}
if (!partPartials.size()) {
    continue;
}
SkASSERT(arguments.size() > 0);
SkASSERT(!fPathVerbs.empty() > 0 && fPathVerbs.end()[-1] == SkPath::kDone_Verb);
```

Good:

```cpp
if (items.empty()) {
    return;
}
if (structNamePrefix.empty()) {
    return false;
}
if (partPartials.empty()) {
    continue;
}
SkASSERT(!arguments.empty());
SkASSERT(!fPathVerbs.empty() && fPathVerbs.back() == SkPath::kDone_Verb);
```
