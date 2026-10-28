---
name: skia/gpu/mask-unused-varyings-when-lifting
description: When lifting shader expressions to vertex varyings in Graphite, clear unneeded base inputs and restrict color expressions to root nodes without fragment work
rubric: safety
severity: should_fix
---

**Rule.** When lifting shader expressions to vertex varyings in Graphite, clear
or mask off underlying input requirements (such as stepLocalCoords) if consuming
descendant nodes receive the lifted varying instead. Additionally, do not lift
nested color expressions into varyings when subsequent or sibling nodes still
require fragment shader work; restrict color expression lifting to root nodes
where fragment shader execution can actually be avoided.

**Why.** Lifting an expression satisfies descendant inputs, so failing to clear
base requirements wastes interpolation bandwidth on redundant varyings.
Furthermore, reading solid color uniforms in the fragment shader is already
cached; lifting nested color expressions adds varying interpolation overhead
without eliminating fragment shader execution.

## Examples

Bad:

```cpp
// 1. Failing to clear base local coords when lifting coordinate expressions:
if (*availableVaryings > 0 && curNeedsLocalCoords &&
    node->entry()->fLiftableExpressionType == ShaderSnippet::LiftableExpressionType::kLocalCoords) {
    --*availableVaryings;
    node->setLiftExpressionFlag();
    // curNeedsLocalCoords remains true, causing redundant base varying interpolation.
}

// 2. Lifting nested color expressions when fragment shader work remains:
for (ShaderNode* node : nodes) {
    if (node->entry()->fLiftableExpressionType == ShaderSnippet::LiftableExpressionType::kPriorStageOutput) {
        node->setLiftExpressionFlag();
        --*availableVaryings;
    }
    lift_color_expressions(node->children(), availableVaryings);
}
```

Good:

```cpp
// 1. Clear base local coords requirement when coordinate expression is lifted:
if (*availableVaryings > 0 && curNeedsLocalCoords &&
    node->entry()->fLiftableExpressionType == ShaderSnippet::LiftableExpressionType::kLocalCoords) {
    --*availableVaryings;
    node->setLiftExpressionFlag();
    curNeedsLocalCoords = false;
    node->unsetLocalCoordsFlag();
}

// 2. Only lift color expressions on root nodes to avoid varying interpolation overhead:
for (ShaderNode* node : nodes) {
    if (node->entry()->fLiftableExpressionType == ShaderSnippet::LiftableExpressionType::kPriorStageOutput) {
        node->setLiftExpressionFlag();
        --*availableVaryings;
    }
}
```
