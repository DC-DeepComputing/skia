---
name: skia/api/name-asymmetric-predicates-directionally
description: When defining predicate methods for asymmetric compatibility or fulfillment checks, use directional names like canBeFulfilledBy rather than ambiguous names like isCompatible
rubric: readability
severity: should_fix
---

**Rule.** When defining predicate methods that verify asymmetric compatibility,
fulfillment, or subset matching between objects, name them with directional
verbs and specific domain actions (such as canBeFulfilledBy or satisfies)
instead of symmetric or vague names like isCompatible. Make subject and object
roles unambiguous so call sites clearly indicate which operand provides the
requirement and which provides the capability.

**Why.** Generic names like isCompatible are symmetric and ambiguous: they
obscure which object is the specification and which is the candidate fulfilling
it. Directional names clarify asymmetric requirements, make call sites
self-documenting, and prevent confusion when comparing subsets, supersets, or
promise requirements.

## Examples

Bad:

```cpp
// Ambiguous: does 'this' fulfill 'that', or does 'that' fulfill 'this'?
bool isCompatible(const TextureInfo& that) const;
```

Good:

```cpp
// Clear direction: 'that' provides resources satisfying 'this' specification
bool canBeFulfilledBy(const TextureInfo& that) const;
```
