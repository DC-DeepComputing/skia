---
name: skia/api/omit-duplicate-base-comments-on-overrides
description: When declaring or implementing virtual function overrides, do not duplicate base-class interface doc comments or restate standard contract requirements in method bodies
rubric: readability
severity: nit
---

**Rule.** Do not duplicate documentation comments on overridden virtual function
declarations or inline comments in method implementation bodies when the base
class or header already documents the method contract. Omit redundant doc
comments on override declarations, and avoid inline comments that merely restate
standard interface contract handling (such as null-buffer skips or return
conventions); only document derived overrides when introducing subclass-specific
behaviors, implementation invariants, or non-obvious caveats.

**Why.** A virtual method's interface contract and behavioral documentation
belong on the base class declaration where callers and implementers consult
them. Duplicating comments on derived class declarations or within
implementation bodies clutters code and creates redundant text that risks
drifting out of sync when the interface changes.

## Examples

Bad:

```cpp
// Bad (redundant doc comment on override declaration):
class DawnGraphicsPipeline : public GraphicsPipeline {
public:
    // If the compilation failed, this will return an error message.
    std::optional<std::string> didAsyncCompilationFail() const override;
};

// Bad (restating base interface contract inside implementation):
size_t SkMemoryStream::read(void* buffer, size_t size) {
    // If `buffer` is null, advance the pointer without writing to the output.
    // `skip()` relies on this.
    if (buffer) {
        sk_careful_memcpy(buffer, fData->bytes() + fOffset, size);
    }
    fOffset += size;
    return size;
}
```

Good:

```cpp
// Good (omit redundant doc comments on overrides):
class DawnGraphicsPipeline : public GraphicsPipeline {
public:
    std::optional<std::string> didAsyncCompilationFail() const override;
};

// Good (implement interface contract directly without restating header docs):
size_t SkMemoryStream::read(void* buffer, size_t size) {
    if (buffer) {
        sk_careful_memcpy(buffer, fData->bytes() + fOffset, size);
    }
    fOffset += size;
    return size;
}
```
