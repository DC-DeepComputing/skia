---
name: skia/prefer-negated-conjunction-over-disjunctive-negations
description: When verifying that a guarded condition does not hold, prefer a single outer negated conjunction !(a && b) over distributed disjunctions like !a || !b
rubric: readability
severity: nit
---

**Rule.** When checking that an optional pointer, handle, or guarded property
condition does not hold, prefer a single outer negation of a conjunction (e.g.,
`!(ptr && ptr->condition())`) rather than distributing negations across a
disjunction (e.g., `!ptr || !ptr->condition()`). Keep positive conditions
grouped together within the conjunction to clearly express the combined
prerequisite and property being rejected.

**Why.** Evaluating multiple distributed negations across a disjunction
increases cognitive load, requiring readers to mentally compute De Morgan's laws
to understand the rejection criteria. Grouping positive conditions inside a
single outer negation mirrors the affirmative prerequisite check, making
conditional logic easier to parse and maintain.

## Examples

Bad:

```cpp
if (numTextures == 2 && (!samplerDescArr || !samplerDescArr->at(0).isImmutable())) {
    return false;
}
```

Good:

```cpp
if (numTextures == 2 && !(samplerDescArr && samplerDescArr->at(0).isImmutable())) {
    return false;
}
```
