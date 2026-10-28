---
name: skia/tools/document-developer-workflow-in-viewer-slides
description: When authoring interactive visualization slides or debugging tools in tools/viewer/, document the step-by-step developer workflow inline for customizing the harness for new test cases
rubric: readability
severity: nit
---

**Rule.** When implementing interactive visualization slides, diagnostic
inspection tools, or debugging harnesses in tools/viewer/, document the
step-by-step developer workflow in an inline comment above the setup routine.
Explicitly explain the sequence of steps required to adapt, configure, or
rebuild the harness for investigating custom shaders, pipelines, or test cases.

**Why.** Interactive visualization slides and diagnostic harnesses frequently
involve non-obvious multi-step workflows—such as inspecting stdout stage dumps,
matching operations to debug panels, or editing stage builders. Documenting the
recipe inline enables other developers to reuse and customize the diagnostic
tool without reverse-engineering its setup process.

## Examples

Bad:

```cpp
// Bad: Diagnostic slide sets up a specific pipeline without explaining how to adapt it.
SkRasterPipelineVisualizer::DebugStageBuilder stageBuilder;
stageBuilder.add(panel1, SkRasterPipelineOp::stage_a)
            .add(panel2, SkRasterPipelineOp::stage_b);
```

Good:

```cpp
// Good: Step-by-step recipe documents how to customize the harness for other shaders.
// Suggested flow for customizing this harness:
// 1) Assign the shader of interest to a paint.
// 2) Pass an empty DebugStageBuilder to CreateBlitter to dump pipeline stages to stdout.
// 3) Add stageBuilder entries for the target stages to inspect.
// 4) Rebuild and run to inspect custom stage lanes.
SkRasterPipelineVisualizer::DebugStageBuilder stageBuilder;
stageBuilder.add(panel1, SkRasterPipelineOp::stage_a);
```
