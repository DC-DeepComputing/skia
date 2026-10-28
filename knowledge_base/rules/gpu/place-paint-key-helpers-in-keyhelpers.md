---
name: skia/gpu/place-paint-key-helpers-in-keyhelpers
description: When defining PaintParamsKey block-building helper templates or functions in Graphite, place them in KeyHelpers.h rather than caller headers like PaintParams.h
rubric: design
severity: should_fix
---

**Rule.** Place PaintParamsKey block-building helper functions and templates
(such as Blend and Compose) in KeyHelpers.h and KeyHelpers.cpp rather than in
higher-level state headers like PaintParams.h. Keep key-generation utilities
centralized in KeyHelpers to avoid improper coupling and unnecessary header
includes in state descriptors.

**Why.** KeyHelpers is Graphite's designated utility module for constructing
shader and pipeline dictionary key blocks. Defining key-construction helpers in
higher-level state headers improperly couples high-level parameter descriptors
to low-level key-building blocks, forcing them to pull in KeyHelpers.h and
PaintParamsKey.h.

## Examples

Bad:

```cpp
// In src/gpu/graphite/PaintParams.h
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"

template <typename AddBlendToKeyT, typename AddSrcToKeyT, typename AddDstToKeyT>
void Blend(const KeyContext& keyContext,
           PaintParamsKeyBuilder* keyBuilder,
           PipelineDataGatherer* gatherer,
           const AddBlendToKeyT& addBlendToKey,
           const AddSrcToKeyT& addSrcToKey,
           const AddDstToKeyT& addDstToKey) {
    BlendComposeBlock::BeginBlock(keyContext, keyBuilder, gatherer);
    addSrcToKey();
    addDstToKey();
    addBlendToKey();
    keyBuilder->endBlock();
}
```

Good:

```cpp
// In src/gpu/graphite/KeyHelpers.h
template <typename AddBlendToKeyT, typename AddSrcToKeyT, typename AddDstToKeyT>
void Blend(const KeyContext& keyContext,
           PaintParamsKeyBuilder* keyBuilder,
           PipelineDataGatherer* gatherer,
           AddBlendToKeyT addBlendToKey,
           AddSrcToKeyT addSrcToKey,
           AddDstToKeyT addDstToKey) {
    BlendComposeBlock::BeginBlock(keyContext, keyBuilder, gatherer);
    addSrcToKey();
    addDstToKey();
    addBlendToKey();
    keyBuilder->endBlock();
}
```
