---
name: skia/gpu/scope-display-presentation-workarounds-to-kpresent
description: When implementing display-presentation workarounds in Ganesh, condition state transitions on BackendSurfaceAccess::kPresent in prepareSurfacesForBackendAccessAndStateUpdates rather than running them in onSubmitToGpu
rubric: design
severity: should_fix
---

**Rule.** When implementing display-presentation driver workarounds or state
transitions in Ganesh backends, implement them inside
prepareSurfacesForBackendAccessAndStateUpdates() conditioned on
SkSurfaces::BackendSurfaceAccess::kPresent. Do not apply display-specific state
transitions globally within onSubmitToGpu(), which executes on every GPU
submission including offscreen draws.

**Why.** Submissions to the GPU occur frequently for offscreen rendering, image
filtering, and intermediate passes. Unconditionally altering driver state (such
as binding default framebuffer 0) in onSubmitToGpu() disrupts offscreen drawing
and introduces redundant state churn, whereas BackendSurfaceAccess::kPresent
specifically signals display presentation flushes from window contexts.

## Examples

Bad:

```cpp
// In GrGLGpu::onSubmitToGpu():
if (this->glCaps().bindDefaultFramebufferOnSubmit() && fBoundDrawFramebuffer != 0) {
    this->bindFramebuffer(GR_GL_DRAW_FRAMEBUFFER, 0);
}
```

Good:

```cpp
// In GrGLGpu::prepareSurfacesForBackendAccessAndStateUpdates():
if (!proxies.empty() &&
    access == SkSurfaces::BackendSurfaceAccess::kPresent &&
    this->glCaps().bindDefaultFramebufferOnPresent()) {
    SkASSERT(proxies.size() == 1);
    this->bindFramebuffer(GR_GL_DRAW_FRAMEBUFFER, 0);
}
```
