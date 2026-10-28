---
name: skia/api/document-placeholder-stubs-with-todo
description: When implementing placeholder or stub methods with empty bodies or dummy return values, annotate them with an attributed TODO comment clarifying future implementation
rubric: readability
severity: nit
---

**Rule.** When providing placeholder or stub method implementations (such as
empty method bodies or virtual overrides returning nullptr or dummy values),
annotate them with an attributed TODO comment referencing a tracking issue
(e.g., TODO(b/...) or TODO(https://issues.skia.org/...)) describing the planned
work. Clarify that the stub is a temporary implementation awaiting future work,
distinguishing it from an intentional permanent no-op or dead code.

**Why.** Leaving an empty method body or returning default dummy values without
explanation creates ambiguity about whether the method is an accidental
omission, dead code, or intentionally stubbed out for follow-up work. An
attributed TODO documents architectural intent, clarifies expectations for
reviewers, and tracks the remaining implementation work.

## Examples

Bad:

```cpp
// Bad: Empty stub method body without explanation
void skottie::SlotManager::setImageSlot(SlotID slotID, sk_sp<SkImage> i) {
}

// Bad: Ambiguous stub override returning dummy value without a TODO
class SkCPURecorder final : public SkRecorder {
public:
    SkCanvas* makeCaptureCanvas(SkCanvas*) final { return nullptr; }
};
```

Good:

```cpp
// Good: Attributed TODO clarifies pending implementation for empty stub
void skottie::SlotManager::setImageSlot(SlotID slotID, sk_sp<SkImage> i) {
    // TODO(b/123456789): Implement image slot tracking.
}

// Good: Attributed TODO clarifies that the override is a temporary placeholder
class SkCPURecorder final : public SkRecorder {
public:
    // TODO(b/412351769): Implement this so we can capture from a CPU Recorder.
    SkCanvas* makeCaptureCanvas(SkCanvas*) final { return nullptr; }
};
```
