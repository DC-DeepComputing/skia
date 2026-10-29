`GrContextOptions::fSkipGLErrorChecks` is deprecated and will be removed. Use the new
`fSkipGLResourceErrorChecks` and `fSkipGLCommandErrorChecks` options instead; setting
`fSkipGLErrorChecks` is equivalent to setting both to the same value.

  * `fSkipGLResourceErrorChecks` controls GL error checks after resource allocations and
    framebuffer completeness checks when creating render targets. Setting it explicitly to
    `kNo` also enables new checks after texture uploads and transfers: an upload that raises a
    GL error then makes the write (e.g. `SkSurface::writePixels()`) and the following flush
    report failure. With `kDefault`, uploads are not checked, as before.
  * `fSkipGLCommandErrorChecks` controls the GL error check at the end of each GPU submission.
    Clients that only need allocation and upload failures can skip these per-submit
    `glGetError()` calls.

GL errors other than `GL_OUT_OF_MEMORY` that Skia observes are now recorded and can be read (and
reset) with the new `GrDirectContext::checkAndResetBackendError()`. Out of memory errors are still
reported by `GrDirectContext::oomed()`.
