---
name: skia/gpu/scope-key-gen-flags-to-child-context
description: When deriving child KeyContexts in Graphite, scope flags locally, forward the modified context to enclosed child blocks, and use per-child contexts for runtime effects
rubric: correctness
severity: should_fix
---

**Rule.** When configuring KeyGenFlags for specific shader stages or snippets in
Graphite key generation, scope flags to that stage using a child KeyContext
(such as keyContext.withExtraFlags(...)) rather than mutating root draw
contexts. When creating a scoped context wrapper (such as
KeyContextWithLocalMatrix) around child shader blocks, pass the modified child
context to all enclosed child block BeginBlock or addToKey calls rather than the
parent KeyContext. Additionally, when binding children for runtime effects,
assert expected child counts with SkASSERT(effect->children().size() == N) and
construct distinct child contexts via keyContext.forRuntimeEffect(effect,
childIndex) instead of reusing child 0 across sibling effects.

**Why.** Root KeyContext flags persist across the entire draw and leak into
unrelated paint stages, triggering shader explosion. Passing outer parent
contexts to enclosed child blocks drops newly established local matrices or
flags, producing corrupted paint keys and uniform data. Furthermore, runtime
effects configure child sampling flags per child; reusing child 0 context
applies mismatched flags to subsequent children.

## Examples

Bad:

```cpp
// Bad: Leaking stage flags into root context for entire draw
if (p.skipColorXform()) {
    keyGenFlags |= KeyGenFlags::kSpecializeColorSpaceXform;
}
KeyContext keyContext(recorder, local2Dev, targetColorInfo, keyGenFlags, p.color());
p.toKey(keyContext, builder, gatherer);

// Bad: Passing outer parent context instead of modified wrapper context to enclosed child block
KeyContextWithLocalMatrix lmContext(keyContext, originMatrix);
LocalMatrixShaderBlock::BeginBlock(lmContext, builder, gatherer, &lmShaderData);
    YUVImageShaderBlock::BeginBlock(keyContext, builder, gatherer, &imgData);
    builder->endBlock();
builder->endBlock();

// Bad: Reusing child 0 context across multiple runtime effect children without asserting count
KeyContext childContext = keyContext.forRuntimeEffect(effect, /*child=*/0);
fDisplacement->priv().addToKey(childContext, desiredDisplacementCombination);
fColor->priv().addToKey(childContext, desiredColorCombination);
```

Good:

```cpp
// Good: Scope stage flags locally using withExtraFlags
KeyContext childContext = keyContext.withExtraFlags(KeyGenFlags::kSpecializeColorSpaceXform);
AddToKey(childContext, keyBuilder, gatherer, childEffect);

// Good: Pass the modified wrapper context to all enclosed child blocks
KeyContextWithLocalMatrix lmContext(keyContext, originMatrix);
LocalMatrixShaderBlock::BeginBlock(lmContext, builder, gatherer, &lmShaderData);
    YUVImageShaderBlock::BeginBlock(lmContext, builder, gatherer, &imgData);
    builder->endBlock();
builder->endBlock();

// Good: Assert expected child count and construct distinct contexts per child index
SkASSERT(effect->children().size() == 2);
fDisplacement->priv().addToKey(keyContext.forRuntimeEffect(effect, /*child=*/0),
                               desiredDisplacementCombination);
fColor->priv().addToKey(keyContext.forRuntimeEffect(effect, /*child=*/1),
                        desiredColorCombination);
```
