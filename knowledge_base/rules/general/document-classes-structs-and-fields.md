---
name: skia/document-classes-structs-and-fields
description: When introducing classes, structs, enums, type aliases, or non-obvious member fields, document their purpose, lifecycles, and invariants at declaration
rubric: readability
severity: should_fix
---

**Rule.** Document every newly introduced class, struct, enum, type alias
(`using` or `typedef`), and non-obvious member variable with a comment
describing its purpose, responsibilities, step-by-step lifecycle, and key safety
invariants. Document shared types and domain mechanisms at their declaration in
the defining header rather than burying their design or prioritizing container
capacity tuning details on downstream member variable usages. For member fields,
explain what state they hold and any coordinate space or unit conventions, but
do not replace type-level documentation with field comments. This is not
required for self-describing members where the name and type make the purpose
unambiguous.

**Why.** Undocumented types, aliases, and data members force future maintainers
to reverse-engineer design invariants, relationships, and lifecycle assumptions
from usage across multiple files. Documenting shared concepts, architectural
mechanisms, and lifecycles at their declaration header ensures visibility for
all callers, while clear member comments prevent accidental invariant violations
and misinterpretation of state.

## Examples

Bad:

```cpp
// Variant 1: member field missing purpose
class Recorder {
    SkTHashSet<uint32_t> fFlushSources;
};

// Variant 2: class missing purpose and architecture comment
class AlphaAtlasManager {
public:
    AlphaAllocation allocate(int width, int height);
};

// Variant 3: type concept documented on downstream field instead of header alias
// In Consumer.cpp:
// Current index of the specialization function in the call stack.
SpecializationIndex fInheritedSpecializationIndex = kUnspecialized;

// Variant 4: struct mechanism documented with container tuning on member field
// In SkRasterPipeline.h:
// ~90% of pipelines use two memory contexts; fewer than 1 in 2 million use more.
mutable skia_private::STArray<2, SkRasterPipeline_MemoryCtxPatch> fMemoryCtxPatches;
// In SkRasterPipelineOpContexts.h:
struct SkRasterPipeline_MemoryCtxPatch {
    SkRasterPipeline_MemoryCtx* context = nullptr;
    void* backup = nullptr;
};
```

Good:

```cpp
// Variant 1: documented member field purpose
class Recorder {
    // Tracks caller IDs of flushTrackedDevices to diagnose redundant flushes.
    SkTHashSet<uint32_t> fFlushSources;
};

// Variant 2: documented class responsibility
// Manages GPU texture atlas pages dedicated to alpha masks for sparse strip rendering.
class AlphaAtlasManager {
public:
    AlphaAllocation allocate(int width, int height);
};

// Variant 3: documented at type alias declaration in defining header
// In Specialization.h:
// Current index of the specialization function in the call stack.
using SpecializationIndex = size_t;

// Variant 4: mechanism lifecycle documented at struct declaration, tuning kept brief
// In SkRasterPipelineOpContexts.h:
// SIMT tail handling: copies tail pixels to scratch, patches MemoryCtx to point to scratch,
// executes pipeline safely, restores copied results, and un-patches contexts.
struct SkRasterPipeline_MemoryCtxPatch {
    SkRasterPipeline_MemoryCtx* context;
    void* backup;  // Remembers context->pixels so we can restore it
};
// In SkRasterPipeline.h:
// Only 1 in 2M pipelines need >2 patches (see SkRasterPipelineOpContexts.h for patching).
skia_private::STArray<2, SkRasterPipeline_MemoryCtxPatch> fMemoryCtxPatches;
```
