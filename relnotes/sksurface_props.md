`SkSurface::makeSurface` now takes an optional SkSurfaceProps. If this is omitted or nullptr,
the original surface's `props` will be used (as it was before).
`SkSurfaceProps` now has a new flag kGpuBudgeted_Flag which is used by `SkSurface::makeSurface`
and `SkCanvas::makeSurface` instead of never assigning the newly created surface against the
GPU cache budget. In other APIs (e.g. ones that take a skgpu::Budgeted parameter) this flag is
ignored.