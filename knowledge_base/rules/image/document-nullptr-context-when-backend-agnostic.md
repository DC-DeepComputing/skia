---
name: skia/image/document-nullptr-context-when-backend-agnostic
description: When passing nullptr for a context pointer to a backend method because the implementation is backend-agnostic, document that the context is unused
rubric: readability
severity: should_fix
---

**Rule.** When invoking a backend-specific method (such as a Ganesh or Graphite
context overload) and passing nullptr for the context parameter because the
underlying implementation is backend-agnostic, add an inline explanatory
comment. Clarify that the callee does not use the context and explain why
delegating with nullptr is safe.

**Why.** Methods accepting backend context pointers like GrDirectContext* or
Recorder* appear to require a valid context to avoid null-pointer dereferences.
Documenting that an implementation is backend-agnostic and safely ignores the
context parameter prevents confusion, avoids mistaken refactorings, and
reassures reviewers that the call is intentional.

## Examples

Bad:

```cpp
sk_sp<SkImage> SkImage_Picture::onMakeSubset(skgpu::graphite::Recorder* recorder,
                                             const SkIRect& subset,
                                             RequiredProperties props) const {
    // Bad: passing nullptr context to a backend method without documenting that it is unused.
    sk_sp<SkImage> image = this->onMakeSubset(nullptr, subset);
    return image;
}
```

Good:

```cpp
sk_sp<SkImage> SkImage_Picture::onMakeSubset(skgpu::graphite::Recorder* recorder,
                                             const SkIRect& subset,
                                             RequiredProperties props) const {
    // Good: documents that the Ganesh overload does not use GrDirectContext.
    // The Ganesh version doesn't make use of GrDirectContext so we can use it to
    // generate our initial subset.
    sk_sp<SkImage> image = this->onMakeSubset(nullptr, subset);
    return image;
}
```
