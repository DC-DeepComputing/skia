---
name: skia/gpu/omit-redundant-post-creation-setters
description: When creating objects via factory methods or constructors that accept property parameters, omit redundant post-construction setter calls for those properties
rubric: readability
severity: nit
---

**Rule.** Do not invoke redundant setter calls (such as setLabel()) on newly
created objects when the factory method, constructor, or creation helper already
accepts and initializes that property. When refactoring creation helpers to take
configuration parameters directly, remove leftover manual setter invocations at
call sites.

**Why.** Passing a property to a creation function initializes the instance at
construction time, rendering subsequent setter calls redundant dead code.
Leaving redundant setters in place clutters call sites and misleadingly suggests
that the property required post-construction reconfiguration.

## Examples

Bad:

```cpp
// Redundant setter call after createTexture already accepted label
if (auto tex = this->createTexture(dimensions, info, label)) {
    fResourceCache->insertResource(tex.get(), key, budgeted, shareable);
    tex->setLabel(std::move(label));
    return tex;
}
```

Good:

```cpp
// createTexture initializes label upon construction; redundant setter omitted
if (auto tex = this->createTexture(dimensions, info, label)) {
    fResourceCache->insertResource(tex.get(), key, budgeted, shareable);
    return tex;
}
```
