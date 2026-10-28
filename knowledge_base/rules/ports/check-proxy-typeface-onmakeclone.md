---
name: skia/ports/check-proxy-typeface-onmakeclone
description: When overriding onMakeClone in typeface wrappers, re-wrap the cloned delegate to preserve synthetic attributes, check for nullptr, or document deferred wrapping with a TODO
rubric: readability
severity: should_fix
---

**Rule.** When overriding onMakeClone() in typeface wrappers or proxies (such as
subclasses of SkTypeface_proxy), re-wrap the cloned underlying typeface to
preserve wrapper-level state and synthetic attributes (such as FC_MATRIX or
FC_EMBOLDEN) rather than returning the unwrapped delegate directly. Verify that
cloning the underlying typeface succeeded before passing the result to wrapper
factories, returning nullptr if it fails. If complete re-wrapping is deferred
during incremental refactoring, document the missing wrapper attributes with an
attributed TODO comment.

**Why.** Returning an unwrapped underlying clone drops proxy-level synthetic
transformations, bolding, and custom metadata. Furthermore, cloning underlying
typefaces is fallible; passing an unchecked null clone into a wrapper factory
causes null pointer dereferences, while leaving deferred re-wrapping
undocumented creates silent visual regressions.

## Examples

Bad:

```cpp
// Anti-pattern 1: Returning unwrapped delegate drops synthetic attributes
sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
    return fProxy->onMakeClone(args);
}

// Anti-pattern 2: Wrapping unchecked null delegate clone
sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
    return SkTypeface_Wrapper::Make(
            SkTypeface_proxy::onMakeClone(args),
            fFamilyName,
            this->fontStyle());
}
```

Good:

```cpp
// Pattern 1: Check nullptr and re-wrap to preserve wrapper state
sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
    sk_sp<SkTypeface> proxy = SkTypeface_proxy::onMakeClone(args);
    if (!proxy) {
        return nullptr;
    }
    return SkTypeface_Wrapper::Make(
            std::move(proxy),
            fFamilyName,
            this->fontStyle());
}

// Pattern 2: Document deferred re-wrapping when staged
sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
    // TODO(bug): need to re-wrap to preserve FC_MATRIX and FC_EMBOLDEN
    return fProxy->onMakeClone(args);
}
```
