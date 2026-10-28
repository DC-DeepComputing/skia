---
name: skia/modules/avoid-child-revalidation-during-animator-sync
description: When implementing Skottie adapters and scene graph nodes, do not revalidate child nodes or instantiate shaders during onSync; defer to onRevalidate
rubric: design
severity: should_fix
---

**Rule.** In Skottie adapters and scene graph nodes, do not revalidate child
nodes or instantiate dependent shaders during the animator sync tick (onSync()).
Instead, expose required properties as SG_ATTRIBUTEs on the render node and
defer child revalidation and shader instantiation to
sksg::RenderNode::onRevalidate().

**Why.** Animator evaluation order during onSync() is arbitrary, so querying
child invalidation (such as hasChildrenInval()) during onSync can run before
child animators have updated, yielding false negatives. Furthermore, onSync()
lacks the active InvalidationController and Context Transformation Matrix (CTM),
forcing the caller to pass an identity matrix and losing layer transforms,
whereas onRevalidate() is guaranteed to execute post-sync with the correct IC
and CTM.

## Examples

Bad:

```cpp
// In SkSLShaderAdapter::onSync():
sk_sp<SkShader> shader = fEffect->makeShader(buildUniformData(), SkSpan(buildChildrenData(this->node())));
this->node()->setShader(std::move(shader));

// In SkSLShaderNode:
sk_sp<SkShader> contentShader() {
    if (!fContentShader || this->hasChildrenInval()) {
        const auto& child = this->children()[0];
        child->revalidate(nullptr, SkMatrix::I());
        // ...
    }
}
```

Good:

```cpp
// Push uniforms as SG_ATTRIBUTEs to the scene-graph node, and instantiate
// dependent shaders during onRevalidate where CTM and IC are available:
SkRect SkSLShaderNode::onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
    const auto& child = this->children()[0];
    const auto bounds = child->revalidate(ic, ctm);
    // Instantiate shader using current CTM and updated child content
    return bounds;
}
```
