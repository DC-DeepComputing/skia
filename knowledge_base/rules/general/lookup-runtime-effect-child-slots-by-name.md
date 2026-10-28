---
name: skia/lookup-runtime-effect-child-slots-by-name
description: When storing or binding children for an SkRuntimeEffect without SkRuntimeEffectBuilder, use SkRuntimeEffect::ChildPtr rather than sk_sp<SkFlattenable> and index slots by name via findChild
rubric: readability
severity: should_fix
---

**Rule.** When storing or passing child shaders, color filters, or blenders for
an SkRuntimeEffect, use SkRuntimeEffect::ChildPtr instead of generic
sk_sp<SkFlattenable>. Additionally, when binding children without
SkRuntimeEffectBuilder, look up each child's slot by name using
effect->findChild(name)->index rather than assuming sequential input or SkSL
declaration order, and pre-size the ChildPtr container to
effect->children().size().

**Why.** SkRuntimeEffect::ChildPtr is the designated type-safe wrapper that
validates child objects are shaders, color filters, or blenders and matches
makeShader parameters without intermediate conversions. Furthermore, runtime
effects expect child effects strictly indexed by each child's slot
(Child::index); relying on declaration or input order causes children to be
bound to incorrect slots or fail to sample properly.

## Examples

Bad:

```cpp
struct ChildData {
    SkString name;
    sk_sp<SkFlattenable> child;
};

// Assumes children match declaration order and uses raw flattenables
std::vector<sk_sp<SkFlattenable>> fChildren;
fChildren.push_back(image->makeShader(sampling));

sk_sp<SkShader> shader =
        fEffect->makeShader(uniforms, fChildren.data(), fChildren.size());
```

Good:

```cpp
struct ChildData {
    SkString name;
    SkRuntimeEffect::ChildPtr child;
};

// Store ChildPtr and index child slots dynamically by name
std::vector<SkRuntimeEffect::ChildPtr> children(fEffect->children().size());
for (const auto& c : fChildren) {
    if (auto meta = fEffect->findChild(c.name)) {
        children[meta->index] = c.child;
    }
}
sk_sp<SkShader> shader = fEffect->makeShader(uniforms, SkSpan(children));
```
