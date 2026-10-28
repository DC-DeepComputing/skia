---
name: skia/gpu/collapse-dst-primitive-blend
description: When emitting Graphite primitive color keys, collapse SkBlendMode::kDst directly when skipColorXform is set, and forward configured skipColorXform rather than hardcoding booleans
rubric: correctness
severity: should_fix
---

**Rule.** When constructing shader pipeline keys in Graphite (such as in
PaintParams and PaintOption), check if primitive blending ignores source paint
color (such as SkBlendMode::kDst). When color transformation is explicitly
skipped (fSkipColorXform), collapse the pipeline to emit the surviving primitive
color block directly rather than emitting a full Blend block with dummy inputs
(such as BuiltInCodeSnippetID::kPriorOutput). Gate this simplification strictly
on fSkipColorXform, and avoid using identity color space transforms
(KeyGenFlags::kEnableIdentityColorSpaceXform) to reconcile topologies. In
fallback blend paths, forward the configured fSkipColorXform to
AddPrimitiveColor rather than hardcoding boolean literals like false.

**Why.** Emitting full Blend blocks with dummy inputs bloats shader pipelines,
while applying blend collapsing unconditionally creates asymmetrical key
topologies across primitive draws. Furthermore, hardcoding false for
skipColorXform in fallback blend paths ignores the configured fSkipColorXform
for non-kDst blend modes, causing unexpected color space conversions and shader
key divergence between PaintOption precompilation and runtime PaintParams.

## Examples

Bad:

```cpp
// Bad 1: Emitting a full Blend block with a dummy/passthrough input for kDst
Blend(keyContext, keyBuilder, gatherer,
      /*addBlendToKey=*/[&]() { AddFixedBlendMode(keyContext, keyBuilder, gatherer, SkBlendMode::kDst); },
      /*addSrcToKey=*/[&]() { keyBuilder->addBlock(BuiltInCodeSnippetID::kPriorOutput); },
      /*addDstToKey=*/[&]() { AddPrimitiveColor(keyContext, keyBuilder, gatherer, fSkipColorXform); });

// Bad 2: Hardcoding false in fallback blend path instead of forwarding fSkipColorXform
Blend(keyContext, keyBuilder, gatherer,
      /*addBlendToKey=*/[&]() { AddToKey(keyContext, keyBuilder, gatherer, fPrimitiveBlender.get()); },
      /*addSrcToKey=*/[&]() { this->addPaintColorToKey(keyContext, keyBuilder, gatherer); },
      /*addDstToKey=*/[&]() { AddPrimitiveColor(keyContext, keyBuilder, gatherer, /*skipColorXform=*/false); });
```

Good:

```cpp
const bool canSkipBlendStep =
    fSkipColorXform &&
    as_BB(fPrimitiveBlender.get())->asBlendMode().has_value() &&
    as_BB(fPrimitiveBlender.get())->asBlendMode().value() == SkBlendMode::kDst;

if (canSkipBlendStep) {
    AddPrimitiveColor(keyContext, keyBuilder, gatherer, /*skipColorXform=*/true);
} else {
    Blend(keyContext, keyBuilder, gatherer,
          /*addBlendToKey=*/[&]() { AddToKey(keyContext, keyBuilder, gatherer, fPrimitiveBlender.get()); },
          /*addSrcToKey=*/[&]() { this->addPaintColorToKey(keyContext, keyBuilder, gatherer); },
          /*addDstToKey=*/[&]() { AddPrimitiveColor(keyContext, keyBuilder, gatherer, fSkipColorXform); });
}
```
