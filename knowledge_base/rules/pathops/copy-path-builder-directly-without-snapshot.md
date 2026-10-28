---
name: skia/pathops/copy-path-builder-directly-without-snapshot
description: When storing or copying an SkPathBuilder into an SkPathBuilder container or variable, copy or move the builder directly instead of calling snapshot()
rubric: safety
severity: should_fix
---

**Rule.** When inserting, copying, or assigning an SkPathBuilder into a
container or variable of SkPathBuilder, copy or move the builder instance
directly. Do not call .snapshot() or construct an intermediate SkPath when the
destination expects an SkPathBuilder.

**Why.** SkPathBuilder provides default copy and move constructors that copy
internal verb and point buffers directly. Calling .snapshot() allocates an
immutable SkPath (along with ref-counted heap storage like SkPathRef), and
converting from SkPath back to SkPathBuilder replays all verbs, incurring
unnecessary heap allocation and redundant verb processing overhead.

## Examples

Bad:

```cpp
// Bad: Materializes an intermediate SkPath, allocating heap storage and replaying verbs.
fPartials.push_back(fCurrent.snapshot());
```

Good:

```cpp
// Good: Copies the builder directly using its copy constructor.
fPartials.push_back(fCurrent);
```
