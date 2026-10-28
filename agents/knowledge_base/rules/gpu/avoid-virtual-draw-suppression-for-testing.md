---
name: skia/gpu/avoid-virtual-draw-suppression-for-testing
description: When implementing GPU command recording helpers, do not add virtual methods or test subclasses to suppress recording commands into in-memory command lists during testing
rubric: design
severity: should_fix
---

**Rule.** Do not introduce virtual methods, template flags, or test subclasses
into GPU command writers to conditionally suppress recording commands for
headless testing. Allow commands to be recorded normally into the in-memory
command list without GPU submission.

**Why.** In headless testing, in-memory command lists are stored in CPU memory
and never submitted to the GPU context for execution. Adding polymorphism or
template flags to bypass recording introduces unnecessary abstraction overhead
and prevents test suites from inspecting or verifying the recorded command
stream.

## Examples

Bad:

```cpp
template <bool ShouldDraw>
void DrawWriter::issueDrawsInternal(...) {
    if constexpr (ShouldDraw) {
        fCommandList->draw(...);
    }
}
class HeadlessDrawWriter : public DrawWriter {
    void issueDraws(...) override { this->issueDrawsInternal<false>(...); }
};
```

Good:

```cpp
void DrawWriter::flushInternal(...) {
    fCommandList->draw(...);
}
```
