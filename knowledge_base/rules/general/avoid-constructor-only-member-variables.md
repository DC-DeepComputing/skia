---
name: skia/avoid-constructor-only-member-variables
description: When declaring class members, do not store temporary computation, initialization state, or mutable scratch buffers in member fields; scope or allocate working state locally
rubric: readability
severity: should_fix
---

**Rule.** Do not declare class member variables to hold temporary computation
state, constructor-only initialization state, or ephemeral working buffers. In
particular, do not mark member fields mutable solely to allow const methods to
mutate working or scratch state during execution; clone or allocate the working
state locally inside the method (such as on the stack via AutoSTMalloc or local
containers) instead. For values only needed to initialize other members in a
constructor, duplicate simple expressions directly across member initializers or
use delegating constructors.

**Why.** Member variables persist for the full lifetime of the object, so
storing temporary method scratchpad data, one-time construction artifacts, or
mutable execution buffers in member fields expands object layout size, clutters
headers with private types, and introduces thread-safety and reentrancy hazards
across concurrent or recursive method calls while violating logical constness.
Scoping or allocating working state locally confines data to where it is needed
and keeps class definitions clean.

## Examples

Bad:

```cpp
// Bad: Temporary computation buffer declared as class member
class ParagraphBuilderImpl {
    ...
    skia_private::TArray<TextIndex, true> fUTF8IndexForUTF16Index;
};

// Bad: Storing constructor-only flag as member to initialize subsequent fields
class DrawWriter::Appender {
    Appender(DrawWriter& w, SkEnumBitMask<RenderStateFlags> flags)
            : fDrawer(w)
            , fAppendVertices(SkToBool(flags & RenderStateFlags::kAppendVertices))
            , fTarget(fAppendVertices ? w.fVertices : w.fInstances) {}
    bool fAppendVertices;
    BindBufferInfo& fTarget;
};

// Bad: Mutable member used for execution scratchpad in const method
class SkRasterPipeline {
    void run(...) const;
    mutable skia_private::STArray<2, MemoryCtxPatch> fMemoryCtxPatches;
};
```

Good:

```cpp
// Good: Scratch buffer scoped locally inside the only method using it
std::unique_ptr<Paragraph> ParagraphBuilderImpl::BuildWithClientInfo(...) {
    skia_private::TArray<TextIndex, true> utf8IndexForUTF16Index;
    ...
}

// Good: Expression evaluated directly across initializers without extra member
class DrawWriter::Appender {
    Appender(DrawWriter& w, SkEnumBitMask<RenderStateFlags> flags)
            : fDrawer(w)
            , fTarget(SkToBool(flags & RenderStateFlags::kAppendVertices) ?
                               w.fVertices : w.fInstances) {}
    BindBufferInfo& fTarget;
};

// Good: Allocate execution scratchpad locally on the stack in const method
void SkRasterPipeline::run(...) const {
    AutoSTMalloc<2, MemoryCtxPatch> patches(fMemoryCtxPatches.size());
    memcpy(patches.data(), fMemoryCtxPatches.data(), fMemoryCtxPatches.size_bytes());
    start_pipeline(..., patches, fMemoryCtxPatches.size(), scratch.get());
}
```
