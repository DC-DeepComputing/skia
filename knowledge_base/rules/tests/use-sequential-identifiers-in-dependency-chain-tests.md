---
name: skia/tests/use-sequential-identifiers-in-dependency-chain-tests
description: In unit tests modeling multi-stage dependency chains or task execution order, name sequential resources with alphabetical identifiers instead of arbitrary role names
rubric: readability
severity: nit
---

**Rule.** When writing unit tests or accompanying diagrams that model
multi-stage dependency chains, task ordering, or data flow across sequential
resources (such as surfaces, canvases, or images), name the resources with
sequential or alphabetical identifiers (such as `surfaceA`, `surfaceB`,
`surfaceC` or `A`, `B`, `C`). Avoid arbitrary role names (such as `scratch`,
`main`, or `final`) that obscure the pipeline creation order, dependency
direction, and execution sequence.

**Why.** Role names like 'scratch', 'main', or 'final' can be ambiguous or
misleading regarding data-flow order in multi-stage pipelines (e.g., when a
'scratch' surface is drawn first, sampled by 'main', and then snapshotted into
'final'). Sequential alphabetical identifiers align the code directly with test
comments and task graphs, making the resource creation order, dependency
direction, and task execution sequence immediately obvious.

## Examples

Bad:

```cpp
// Ambiguous role names obscure pipeline dependency sequence
sk_sp<SkSurface> scratchSurface = SkSurfaces::RenderTarget(recorder.get(), info);
sk_sp<SkImage> scratchImage = SkSurfaces::AsImage(scratchSurface);

sk_sp<SkSurface> mainSurface = SkSurfaces::RenderTarget(recorder.get(), info);
mainSurface->getCanvas()->drawImage(scratchImage, 0, 0);

sk_sp<SkSurface> finalSurface = SkSurfaces::RenderTarget(recorder.get(), info);
finalSurface->getCanvas()->drawImage(mainSurface->makeImageSnapshot(), 0, 0);
```

Good:

```cpp
// Sequential identifiers make dependency and task order immediately clear
// "A"
sk_sp<SkSurface> surfaceA = SkSurfaces::RenderTarget(recorder.get(), info);
sk_sp<SkImage> imageA = SkSurfaces::AsImage(surfaceA);

// "B" -> depends on A
sk_sp<SkSurface> surfaceB = SkSurfaces::RenderTarget(recorder.get(), info);
surfaceB->getCanvas()->drawImage(imageA, 0, 0);

// "C" -> depends on B
sk_sp<SkSurface> surfaceC = SkSurfaces::RenderTarget(recorder.get(), info);
surfaceC->getCanvas()->drawImage(surfaceB->makeImageSnapshot(), 0, 0);
```
