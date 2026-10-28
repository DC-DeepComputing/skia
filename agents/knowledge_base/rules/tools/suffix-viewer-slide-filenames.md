---
name: skia/tools/suffix-viewer-slide-filenames
description: When authoring Viewer slide implementation source files in tools/viewer/, suffix filenames with Slide.cpp to maintain consistent subsystem file naming
rubric: readability
severity: nit
---

**Rule.** When authoring Viewer slide implementations in tools/viewer/, suffix
the source filename with Slide.cpp (e.g., <Name>Slide.cpp). Do not omit the
Slide suffix or name slide implementation files with bare feature names.

**Why.** Viewer slide implementations follow a strict convention where source
files end with the Slide.cpp suffix (such as AnimBlurSlide.cpp and
EdgeBuilderVizSlide.cpp). Consistent suffixing distinguishes slide files from
framework harnesses, helpers, and utilities across tools/viewer/.

## Examples

Bad:

```cpp
// File: tools/viewer/RasterPipelineViz.cpp
#include "tools/viewer/ClickHandlerSlide.h"

class RPVizSlide : public ClickHandlerSlide { ... };
DEF_SLIDE(return new RPVizSlide();)
```

Good:

```cpp
// File: tools/viewer/RasterPipelineVizSlide.cpp
#include "tools/viewer/ClickHandlerSlide.h"

class RPVizSlide : public ClickHandlerSlide { ... };
DEF_SLIDE(return new RPVizSlide();)
```
