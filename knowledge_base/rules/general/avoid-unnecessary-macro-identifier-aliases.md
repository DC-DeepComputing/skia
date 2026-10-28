---
name: skia/avoid-unnecessary-macro-identifier-aliases
description: When referencing variables or symbols, do not introduce trivial preprocessor macro aliases; access the underlying identifiers directly without unnecessary indirection
rubric: design
severity: should_fix
---

**Rule.** Do not introduce preprocessor macro aliases for variable, function, or
symbol names unless they provide necessary configurability, conditional
compilation, or platform abstraction. Reference the underlying identifiers
directly instead of defining redundant 1:1 macro wrappers.

**Why.** Macros that merely alias existing variable or symbol names without
parameterization, conditional compilation, or platform abstraction introduce
unnecessary indirection. They obscure variable origins from code search and
refactoring tools and make downstream code harder to read and trace.

## Examples

Bad:

```cpp
// Bad: trivial macro aliases introduce preprocessor indirection without configurability
#define RP_BLITTER_TRACE_NAME gSkRPBlitterTrace
#define RP_BLITTER_TRACE_CAPTURE gSkRPBlitterTraceCapture

#define SAVE_BLITTER_CAPTURE \
    RP_BLITTER_TRACE_CAPTURE = RP_BLITTER_TRACE_NAME
```

Good:

```cpp
// Good: reference underlying identifiers directly in macros or call sites
#define SAVE_BLITTER_CAPTURE \
    gSkRPBlitterTraceCapture = gSkRPBlitterTrace
```
