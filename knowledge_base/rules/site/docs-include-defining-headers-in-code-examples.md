---
name: skia/site/docs-include-defining-headers-in-code-examples
description: 'When authoring C++ code examples in Markdown documentation, directly include defining headers for all referenced APIs and eliminate duplicate #include directives'
rubric: readability
severity: nit
---

**Rule.** When writing or modifying C++ code examples in Markdown documentation,
ensure all referenced APIs and factory methods (such as backend-specific
SkSurfaces factory functions like SkSurfaces::RenderTarget) explicitly include
their defining headers (e.g., include/gpu/ganesh/SkSurfaceGanesh.h) rather than
relying on core umbrella headers. Additionally, check for and remove duplicate
#include directives.

**Why.** Documentation code snippets are not compiled or validated by CI builds,
allowing missing header includes and duplicate directives to slip through
unnoticed. Ensuring complete and deduplicated includes prevents build errors and
confusion for developers copying example snippets into their projects.

## Examples

Bad:

```cpp
<!--?prettify lang=cc?-->

    #include "include/gpu/ganesh/GrDirectContext.h"
    #include "include/gpu/ganesh/gl/GrGLInterface.h"
    #include "include/gpu/ganesh/gl/GrGLInterface.h"
    #include "include/core/SkSurface.h"

    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(context.get(), skgpu::Budgeted::kNo, info);
```

Good:

```cpp
<!--?prettify lang=cc?-->

    #include "include/gpu/ganesh/GrDirectContext.h"
    #include "include/gpu/ganesh/gl/GrGLInterface.h"
    #include "include/gpu/ganesh/SkSurfaceGanesh.h"
    #include "include/core/SkSurface.h"

    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(context.get(), skgpu::Budgeted::kNo, info);
```
