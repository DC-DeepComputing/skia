---
name: skia/place-group-scoped-todos-above-all-declarations
description: When adding a TODO or cleanup comment affecting multiple adjacent declarations or member variables, place the comment above the entire group with explicit plural scope
rubric: readability
severity: nit
---

**Rule.** When a TODO, cleanup, or removal comment applies to multiple adjacent
member variables or declarations, place the comment above the entire affected
group rather than immediately preceding the last item. Use explicit plural
phrasing (such as 'remove these' or 'cleanup the following members') to clarify
the scope of the planned refactoring.

**Why.** Placing a cleanup or removal TODO directly above only the final member
of a related group misleadingly implies that preceding members are permanent,
obscuring that the entire group forms temporary state. Hoisting the comment
above all affected declarations and using plural phrasing ensures the full scope
of upcoming refactorings is immediately obvious to future maintainers.

## Examples

Bad:

```cpp
    std::unique_ptr<BoundsManager> fColorDepthBoundsManager;
    // Tracks disjoint stencil indices for all recorded draws
    // TODO (thomsmit): remove when layering is added
    std::unique_ptr<IntersectionTreeSet> fDisjointStencilSet;
```

Good:

```cpp
    // TODO (thomsmit): remove these when layering is added
    // Tracks accumulated intersections for ordering dependent use of the attachments
    std::unique_ptr<BoundsManager> fColorDepthBoundsManager;
    // Tracks disjoint stencil indices for all recorded draws
    std::unique_ptr<IntersectionTreeSet> fDisjointStencilSet;
```
