---
name: skia/modules/accept-element-identifiers-in-document-containers
description: When designing document container APIs that target specific nodes, accept element identifiers rather than direct node references to ensure document containment
rubric: design
severity: should_fix
---

**Rule.** In document container APIs (such as SkSVGDOM), accept document-scoped
element identifiers (such as string IDs or IRIs) rather than direct node
references (such as SkSVGNode&) for targeted element operations. Resolve nodes
internally within the document's own context and ID mapper.

**Why.** Accepting direct node references allows callers to pass arbitrary,
detached, or foreign nodes from different documents, violating document
containment and rendering context invariants. Requiring element IDs guarantees
that the targeted node is resolved and validated within the owning document.

## Examples

Bad:

```cpp
// Bad: Direct node reference allows injecting nodes from foreign documents.
class SkSVGDOM : public SkRefCnt {
public:
    void renderNode(SkCanvas*, SkSVGPresentationContext&, const SkSVGNode& node) const;
};
```

Good:

```cpp
// Good: Element identifier ensures node resolution within this document's context.
class SkSVGDOM : public SkRefCnt {
public:
    void renderNode(SkCanvas*, SkSVGPresentationContext&, const char* id) const;
};
```
