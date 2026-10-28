---
name: skia/core/extract-shared-blitter-pipeline-setup
description: When implementing specialized or debugging blitter factories that intercept or augment raster pipelines, extract shared pipeline initialization into a common helper
rubric: design
severity: should_fix
---

**Rule.** When implementing specialized, debugging, or visualizer blitter
factories that intercept or augment SkRasterPipeline stages, extract shared
pipeline initialization—such as paint color conversion, shader root stage
appending, and opacity resolution—into a common static helper. Ensure both
production and diagnostic blitters share the same pipeline assembly routine to
prevent configuration divergence.

**Why.** Duplicating pipeline setup logic across blitter creation entry points
leads to behavioral divergence when shader stage appending, color translation,
or alpha modulation rules change. Factoring setup into a shared helper
guarantees consistent pipeline configuration across production and visualization
blitter variants.

## Examples

Bad:

```cpp
// Duplicating pipeline creation in both SkCreateRasterPipelineBlitter and CreateBlitter:
SkBlitter* CreateBlitter(...) {
    SkColor4f dstPaintColor = paint_color_to_dst(paint, dst);
    auto shader = as_SB(paint.getShader());
    SkRasterPipeline_<256> shaderPipeline;
    if (!shader) {
        shaderPipeline.appendConstantColor(alloc, dstPaintColor.premul().vec());
        ...
    }
    if (shader->appendRootStages(SkStageRec{&shaderPipeline, alloc, dstCT, dstCS, dstPaintColor, props}, ctm)) {
        if (dstPaintColor.fA != 1.0f) {
            shaderPipeline.append(SkRasterPipelineOp::scale_1_float, alloc->make<float>(dstPaintColor.fA));
        }
        ...
    }
}
```

Good:

```cpp
static bool create_pipeline_for_blitter(const SkPixmap& dst,
                                        const SkPaint& paint,
                                        const SkMatrix& ctm,
                                        SkArenaAlloc* alloc,
                                        const SkSurfaceProps& props,
                                        SkRasterPipeline* shaderPipeline,
                                        SkColor4f* dstPaintColor,
                                        bool* isOpaqueOut,
                                        bool* isConstantOut) {
    *dstPaintColor = paint_color_to_dst(paint, dst);
    ...
}

SkBlitter* CreateBlitter(...) {
    SkRasterPipeline_<256> shaderPipeline;
    SkColor4f dstPaintColor;
    bool is_opaque, is_constant;
    if (!create_pipeline_for_blitter(dst, paint, ctm, alloc, props,
                                     &shaderPipeline, &dstPaintColor,
                                     &is_opaque, &is_constant)) {
        return nullptr;
    }
    // Inspect or augment stages in shaderPipeline...
}
```
