---
name: skia/opts/use-portable-vector-alias
description: When defining vector types in SkRasterPipeline_opts.h or skcms, use the portable Vec<N, T> alias template instead of compiler-specific __attribute__((ext_vector_type(N)))
rubric: correctness
severity: should_fix
---

**Rule.** When defining stage vector types or vector aliases in
SkRasterPipeline_opts.h or skcms, use the portable Vec<N, T> alias template
(such as template <typename T> using V = Vec<N, T>;) rather than
compiler-specific __attribute__((ext_vector_type(N))) or raw vector_size
attributes. Do not define stage vector types with Clang-only vector attributes.

**Why.** The ext_vector_type attribute is a Clang-specific extension that fails
compilation under GCC, which requires __attribute__((vector_size)) with struct
indirection. SkRasterPipeline_opts.h and skcms define the cross-compiler Vec<N,
T> alias template specifically to abstract these toolchain differences portably.

## Examples

Bad:

```cpp
#elif defined(JUMPER_IS_LASX)
    template <typename T> using V = T __attribute__((ext_vector_type(8)));
```

Good:

```cpp
#elif defined(JUMPER_IS_LASX)
    template <typename T> using V = Vec<8, T>;
```
