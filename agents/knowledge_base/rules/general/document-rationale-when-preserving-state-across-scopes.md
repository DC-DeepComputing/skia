---
name: skia/document-rationale-when-preserving-state-across-scopes
description: When selectively preserving state or making exceptions across isolated contexts, document the rationale and downstream consumers rather than merely narrating what is preserved
rubric: readability
severity: nit
---

**Rule.** When writing comments for non-obvious logic, exceptions, or state
preservation (such as selectively retaining state across an otherwise isolated
or reset context), explain the underlying rationale and which downstream
operations or consumers depend on it. Do not merely narrate or restate what the
code does.

**Why.** Comments that only restate what code does add little value beyond
reading the code itself. Explaining why state is preserved and identifying which
downstream operations depend on it prevents future maintainers from
inadvertently refactoring away or breaking necessary context.

## Examples

Bad:

```cpp
// However, we do preserve the OBB scope.
SkSVGPresentationContext pctx;
SkSVGRenderContext local_ctx(fCanvas, fFontMgr, fResourceProvider, fIDMapper,
                             *fLengthContext, pctx, fOBBScope);
```

Good:

```cpp
// Preserve the OBB scope because some paints use object bounding box coords
// (e.g. gradient control points), which requires access to the render context
// and node being rendered.
SkSVGPresentationContext pctx;
SkSVGRenderContext local_ctx(fCanvas, fFontMgr, fResourceProvider, fIDMapper,
                             *fLengthContext, pctx, fOBBScope);
```
