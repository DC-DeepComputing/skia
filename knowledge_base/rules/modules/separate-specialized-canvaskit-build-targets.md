---
name: skia/modules/separate-specialized-canvaskit-build-targets
description: When modifying CanvasKit Makefile targets, do not alter the default release target with specialized build flags; introduce a dedicated variant target instead
rubric: design
severity: should_fix
---

**Rule.** Do not modify the default release build target in CanvasKit Makefiles
with specialized or experimental build flags (such as feature-specific shapers,
tracing, or omitted modules). Introduce a dedicated variant target (e.g.,
release_<variant>) instead to preserve the canonical release build for standard
workflows.

**Why.** CanvasKit's default release target is invoked by standard build
workflows, clean recipes, and documentation instructions. Mutating the default
target with specialized flags alters the output artifact for all consumers,
whereas dedicated targets allow opt-in variants without breaking downstream
expectations.

## Examples

Bad:

```cpp
release:
	# Does an incremental build where possible.
	./compile.sh client_unicode force_tracing # no_paragraph
```

Good:

```cpp
release:
	# Does an incremental build where possible.
	./compile.sh

release_bidi:
	# Does an incremental build where possible.
	./compile.sh client_bidi no_paragraph no_skottie
```
