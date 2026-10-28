---
name: skia/gpu/extract-optional-pipeline-stages-to-static-helpers
description: When implementing complex GPU pipeline creation routines, extract large optional feature branches and sub-stages into file-static helper functions instead of inlining multi-step setup
rubric: design
severity: should_fix
---

**Rule.** When implementing complex GPU pipeline or resource creation routines,
extract multi-step setup for optional extensions, feature branches, or
self-contained configuration stages into file-static helper functions (such as
create_* or setup_*). Keep the mainline creation function focused on top-level
orchestration, resource provider interaction, and error propagation rather than
inlining extensive branch-specific descriptor configuration.

**Why.** Inlining multi-step descriptor setup and extension-specific API calls
bloats the primary creation routine and obscures the standard path taken when
the feature is disabled or unavailable. Isolating optional branches and
sub-stages in dedicated static helpers encapsulates branch-specific local state
and error logging while keeping mainline construction readable.

## Examples

Bad:

```cpp
// In MakePipeline:
if (shadersPipeline && sharedContext->caps()->usePipelineLibraries()) {
    VkGraphicsPipelineCreateInfo shadersPipelineCreateInfo = {};
    // ... dozens of lines setting up descriptors and calling CreateGraphicsPipelines ...
    shadersLibraryInfo.libraryCount = 1;
    shadersLibraryInfo.pLibraries = shadersPipeline;
    AddToPNextChain(&pipelineCreateInfo, &shadersLibraryInfo);
}
```

Good:

```cpp
// In MakePipeline:
if (shadersPipeline && sharedContext->caps()->usePipelineLibraries()) {
    if (!create_shaders_pipeline(sharedContext,
                                 rsrcProvider->pipelineCache(),
                                 pipelineCreateInfo,
                                 shadersLibraryInfo,
                                 libraryInfo,
                                 shadersPipeline)) {
        SKGPU_LOG_E("Failed to create pipeline library\n");
        return VK_NULL_HANDLE;
    }
}
```
