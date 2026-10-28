---
name: skia/modules/name-node-rendering-methods-explicitly
description: When declaring methods on DOM or scene-graph container classes to draw specific elements by ID, use explicit names like renderNode rather than overloading render
rubric: readability
severity: nit
---

**Rule.** In DOM and scene-graph container classes (such as SkSVGDOM),
explicitly name methods that target an individual element or node (e.g.,
renderNode) rather than overloading generic container-level operations (e.g.,
render). Keep method names consistent with the underlying delegated element
methods they invoke.

**Why.** Overloading container-level methods like render with an element ID
obscures at call sites that only an isolated sub-tree or element is drawn rather
than the whole document. Explicit names like renderNode clarify the scope of
drawing and maintain consistency across delegating container classes and their
root elements.

## Examples

Bad:

```cpp
class SkSVGDOM {
    void render(SkCanvas*) const;
    // Ambiguous overload obscures that only a single element is rendered:
    void render(SkCanvas*, SkSVGPresentationContext&, const char* id) const;
};
```

Good:

```cpp
class SkSVGDOM {
    void render(SkCanvas*) const;
    // Explicit name clarifies target element and matches SkSVGSVG::renderNode:
    void renderNode(SkCanvas*, SkSVGPresentationContext&, const char* id) const;
};
```
