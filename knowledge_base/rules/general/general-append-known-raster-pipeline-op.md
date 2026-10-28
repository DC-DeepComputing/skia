---
name: skia/general-append-known-raster-pipeline-op
description: When appending a compile-time constant blend mode to an SkRasterPipeline, append the SkRasterPipelineOp directly instead of calling SkBlendMode_AppendStages
rubric: readability
severity: nit
---

**Rule.** When appending a statically known or compile-time constant blend mode
to an SkRasterPipeline, append the specific SkRasterPipelineOp directly (e.g.,
`pipeline->append(SkRasterPipelineOp::srcover)`) instead of routing through
`SkBlendMode_AppendStages`. Reserve `SkBlendMode_AppendStages` strictly for
dynamic, runtime-variable SkBlendMode values.

**Why.** Calling `SkBlendMode_AppendStages` with a compile-time constant blend
mode adds unnecessary runtime switch dispatch and forces inclusion of private
headers like `src/core/SkBlendModePriv.h`. Directly appending the specific
`SkRasterPipelineOp` avoids indirection, trims internal header dependencies, and
makes pipeline assembly clear and direct.

## Examples

Bad:

```cpp
// When the blend mode is known statically at compile time:
#include "src/core/SkBlendModePriv.h"
...
SkBlendMode_AppendStages(SkBlendMode::kSrcOver, fStage.fPipeline);
```

Good:

```cpp
// Directly append the specific raster pipeline op:
fStage.fPipeline->append(SkRasterPipelineOp::srcover);
```
