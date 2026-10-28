---
name: skia/prefix-raii-guards-with-auto
description: When declaring RAII scope guard or state-management helper classes, prefix the class name with Auto (e.g., AutoRestore, AutoCaptureFrame) to signal scoped lifetime management
rubric: readability
severity: should_fix
---

**Rule.** Name RAII scope guard and state-management helper classes with an Auto
prefix (or SkAuto for public or core classes, e.g., AutoRestore,
AutoCaptureFrame, SkAutoCanvasRestore) rather than unprefixed names or generic
suffixes like Guard or Scope. Ensure the class signals scoped lifetime
management and pairs resource acquisition or state alteration in constructors
with automatic cleanup or restoration in the destructor.

**Why.** In Skia conventions, the Auto prefix immediately signals to reviewers
and readers that an object is an RAII stack-scoped guard managing lifetime,
state changes, or cleanup upon block exit. Using unprefixed names or arbitrary
suffixes obscures the object's scoped lifecycle and risks callers misusing it as
a persistent handle.

## Examples

Bad:

```cpp
// Bad: Unprefixed name obscuring RAII scope guard semantics
class [[nodiscard]] RenderDocCapture {
public:
    RenderDocCapture(RenderDoc* renderdoc) : fRenderDoc(renderdoc) {
        if (fRenderDoc) {
            fRenderDoc->startFrame();
        }
    }
    ~RenderDocCapture() {
        if (fRenderDoc) {
            fRenderDoc->endFrame();
        }
    }
private:
    RenderDoc* fRenderDoc;
};
```

Good:

```cpp
// Good: Prefixed with Auto to clearly signal RAII scoped lifetime
class [[nodiscard]] AutoCaptureFrame {
public:
    AutoCaptureFrame(ContextPriv context, bool capture)
            : fContext(context), fCapture(capture) {
        if (fCapture) {
            fContext.startCapture();
        }
    }
    ~AutoCaptureFrame() {
        if (fCapture) {
            fContext.stopCapture();
        }
    }
private:
    ContextPriv fContext;
    bool fCapture = false;
};
```
