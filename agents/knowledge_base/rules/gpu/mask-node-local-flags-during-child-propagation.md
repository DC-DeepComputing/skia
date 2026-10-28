---
name: skia/gpu/mask-node-local-flags-during-child-propagation
description: When propagating snippet requirement flags upward from child ShaderNodes to parent nodes in Graphite, mask off node-local behavioral flags like kPassthroughLocalCoords and kLiftExpression
rubric: correctness
severity: should_fix
---

**Rule.** When propagating snippet requirement flags upward from child
ShaderNodes to parent nodes in an effect tree, mask off node-local behavioral
flags (such as SnippetRequirementFlags::kPassthroughLocalCoords and
kLiftExpression). Ensure parent nodes only aggregate transitive input
requirements rather than inheriting child-local properties.

**Why.** Node-specific flags represent local behavioral properties of a
particular snippet rather than transitive requirements that bubble up to
ancestors. Allowing them to propagate upward causes parent nodes to falsely
inherit coordinate passthrough or expression lifting states, leading to invalid
varying allocation and coordinate lifting decisions.

## Examples

Bad:

```cpp
SkEnumBitMask<SnippetRequirementFlags> mask = SnippetRequirementFlags::kNone;
if (isRuntimeEffect || (isCompose && child == children.back())) {
    mask = SnippetRequirementFlags::kLocalCoords |
           SnippetRequirementFlags::kPriorStageOutput |
           SnippetRequirementFlags::kBlenderDstColor;
}
fRequiredFlags |= (child->requiredFlags() & ~mask);
```

Good:

```cpp
SkEnumBitMask<SnippetRequirementFlags> mask =
        SnippetRequirementFlags::kPassthroughLocalCoords |
        SnippetRequirementFlags::kLiftExpression;
if (isRuntimeEffect || (isCompose && child == children.back())) {
    mask |= SnippetRequirementFlags::kLocalCoords |
            SnippetRequirementFlags::kPriorStageOutput |
            SnippetRequirementFlags::kBlenderDstColor;
}
fRequiredFlags |= (child->requiredFlags() & ~mask);
```
