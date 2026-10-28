---
name: skia/group-mutable-member-variables
description: When declaring C++ class member variables, group mutable fields together in the member section rather than interspersing them among const or regular data members
rubric: readability
severity: nit
---

**Rule.** When declaring member variables in C++ classes or structs, group
mutable fields together (typically at the end of the private member data
section) rather than interspersing them among const or regular member variables.
Keep any thread-safety annotations or lazy initialization comments adjacent to
the grouped mutable fields.

**Why.** Mutable member variables bypass C++ const-correctness guarantees and
can be mutated inside const methods. Grouping them together clarifies class
memory layout and allows maintainers to readily audit which state is modified
during const operations or requires synchronization.

## Examples

Bad:

```cpp
class AnimationBuilder {
private:
    Animation::Builder::Stats*     fStats;
    mutable std::vector<LayerInfo> fLayerInfo; // Interspersed among regular and const fields
    const SkSize                   fCompSize;
    mutable AnimatorScope*         fCurrentAnimatorScope;
    const uint32_t                 fFlags;
};
```

Good:

```cpp
class AnimationBuilder {
private:
    Animation::Builder::Stats*     fStats;
    const SkSize                   fCompSize;
    const uint32_t                 fFlags;

    // Group mutable fields together for clear const-correctness and thread-safety inspection:
    mutable std::vector<LayerInfo> fLayerInfo;
    mutable AnimatorScope*         fCurrentAnimatorScope;
};
```
