---
name: skia/avoid-fictional-helpers-in-doc-examples
description: When writing example code snippets in header doc comments, use generic variable placeholders or ellipses instead of inventing fictional helper functions
rubric: readability
severity: nit
---

**Rule.** When illustrating API usage with code snippets in header documentation
comments, do not define or call fictional helper functions to construct inputs.
Use generic variable placeholders (such as `Type* var = ...;` or ellipses) to
represent prerequisites so readers do not mistake non-existent helpers for class
or library methods.

**Why.** Fictional helper functions in example snippets (such as make_panel())
look like actual convenience methods provided by the class or library, confusing
consumers who attempt to call them. Generic variable placeholders or ellipses
make required parameter types obvious without implying the existence of
non-existent helper APIs.

## Examples

Bad:

```cpp
// Example usage:
//   SkBitmap make_panel() { ... }
//
//   DebugStageBuilder stageBuilder;
//   stageBuilder.add(make_panel(), SkRasterPipelineOp::debug_x,
//                    make_panel(), SkRasterPipelineOp::debug_y);
```

Good:

```cpp
// Example usage:
//   SkBitmap* panels = ...;  // acquired or constructed by caller
//
//   DebugStageBuilder stageBuilder;
//   stageBuilder.add(panels[0], SkRasterPipelineOp::debug_x,
//                    panels[1], SkRasterPipelineOp::debug_y);
```
