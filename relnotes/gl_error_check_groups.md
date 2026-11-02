`GrContextOptions::fGLErrorChecks` selects which GL errors Skia checks for:

  * `kDefault`: Skia chooses, as before (no checks over a command buffer or on WebGL; otherwise
    checks after resource allocations and at the end of each GPU submission). Texture uploads are
    not checked.
  * `kDisabled`: no GL error checks.
  * `kResources`: checks after resource allocations, framebuffer completeness checks when creating
    render targets, and new checks after texture uploads and transfers, without per-submit
    `glGetError()` calls. An upload that raises a GL error makes the write (e.g.
    `SkSurface::writePixels()`) and the following flush report failure.
  * `kAll`: `kResources` plus the check at the end of each GPU submission.

Texture uploads are only checked when `fGLErrorChecks` is explicitly `kResources` or `kAll`.

`GrContextOptions::fSkipGLErrorChecks` is deprecated and will be removed; use `fGLErrorChecks`
instead. `kYes` behaves like `kDisabled`, and `kNo` keeps the existing checks without upload checks.

GL errors other than `GL_OUT_OF_MEMORY` that Skia observes are now recorded and can be read (and
reset) with the new `GrDirectContext::checkAndResetBackendError()`. Out of memory errors are still
reported by `GrDirectContext::oomed()`.
