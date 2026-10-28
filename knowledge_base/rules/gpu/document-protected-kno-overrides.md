---
name: skia/gpu/document-protected-kno-overrides
description: When passing hardcoded Protected::kNo for GPU resources, base constructors, tasks, or probe allocations, document the backend limitation, hardware constraint, or safety rationale
rubric: readability
severity: should_fix
---

**Rule.** When explicitly specifying Protected::kNo in base GPU resource or
command constructors (such as in backends lacking protected memory support), for
specific tasks or allocations, or in localized probe and format-determination
routines rather than inheriting or threading context protectedness, document the
rationale with an explanatory comment. Clarify whether the fixed value stems
from backend limitations, hardware constraints (such as vertex/index buffers
prohibiting protected access), or probe safety invariants (such as probe
resources never submitting work to GPU queues and format compatibility not
differing across protected modes).

**Why.** In protected GPU contexts, resources, tasks, and allocations normally
inherit or match the protected status of the context, while multi-backend base
classes accept dynamic protectedness. Unconditionally passing Protected::kNo
without an explanatory comment appears to be an accidental omission, oversight,
or violation of strictProtectedness invariants, risking inadvertent refactorings
that introduce unsupported allocations or driver validation errors.

## Examples

Bad:

```cpp
// 1. Backend constructors lacking protected memory support:
DawnBuffer::DawnBuffer(const DawnSharedContext* ctx, size_t size, wgpu::Buffer buffer)
        : Buffer(ctx, size, Protected::kNo, /*usageRefs=*/...) {}

// 2. Specific tasks or allocations overriding ambient context protectedness:
auto copyTask = CopyBufferToBufferTask::Make(src, srcOffset, dst, dstOffset, size);
if (!queueManager->addTask(copyTask.get(), context, Protected::kNo)) {
    return false;
}

// 3. Probe or format-determination allocations using fixed Protected::kNo without comment:
GrProtected kNotProtected = skgpu::Protected::kNo;
GrGLuint colorID = this->createTexture({kSize, kSize}, format, GR_GL_TEXTURE_2D,
                                       GrRenderable::kYes, nullptr, 1, kNotProtected);
```

Good:

```cpp
// 1. Backend constructors lacking protected memory support:
DawnBuffer::DawnBuffer(const DawnSharedContext* ctx, size_t size, wgpu::Buffer buffer)
        : Buffer(ctx, size, Protected::kNo /*Dawn lacks protected support*/, /*usageRefs=*/...) {}

// 2. Specific tasks or allocations overriding ambient context protectedness:
auto copyTask = CopyBufferToBufferTask::Make(src, srcOffset, dst, dstOffset, size);
// Static vertex/index buffers prohibit protected memory access in shaders; uploads must be non-protected.
if (!queueManager->addTask(copyTask.get(), context, Protected::kNo)) {
    return false;
}

// 3. Probe or format-determination allocations explaining why Protected::kNo is safe:
// Determining format only; acceptable format does not differ for protected buffers
// and Protected::kNo will not violate strictProtectedness since no work is queued.
GrProtected kNotProtected = skgpu::Protected::kNo;
GrGLuint colorID = this->createTexture({kSize, kSize}, format, GR_GL_TEXTURE_2D,
                                       GrRenderable::kYes, nullptr, 1, kNotProtected);
```
