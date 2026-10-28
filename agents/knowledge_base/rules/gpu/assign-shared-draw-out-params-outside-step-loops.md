---
name: skia/gpu/assign-shared-draw-out-params-outside-step-loops
description: When capturing shared draw parameters via an out-parameter during Graphite draw recording, assign the out-pointer once outside the render-step loop rather than repeatedly inside it
rubric: readability
severity: should_fix
---

**Rule.** When capturing a shared object (such as DrawParams) via an
out-parameter during draw recording, assign the out-pointer once outside the
loop over constituent render steps rather than repeatedly inside the loop.

**Why.** A shared draw parameter record is allocated once per draw command and
shared across all render steps. Reassigning the out-pointer on every iteration
of the render step loop is redundant and misleadingly suggests that the captured
object varies per render step.

## Examples

Bad:

```cpp
for (int stepIndex = 0; stepIndex < renderer->numRenderSteps(); ++stepIndex) {
    if (captureParams) {
        this->recordDepthOnly(...);
        *captureParams = drawParams;
    }
}
```

Good:

```cpp
if (captureParams) {
    *captureParams = drawParams;
}

for (int stepIndex = 0; stepIndex < renderer->numRenderSteps(); ++stepIndex) {
    if (captureParams) {
        this->recordDepthOnly(...);
    }
}
```
