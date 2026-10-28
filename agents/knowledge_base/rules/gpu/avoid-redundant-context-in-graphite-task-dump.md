---
name: skia/gpu/avoid-redundant-context-in-graphite-task-dump
description: When implementing task dumping (SK_DUMP_TASKS) in Graphite tasks, do not plumb enclosing DrawContext pointers; dump the Task address and TextureProxy label instead
rubric: design
severity: should_fix
---

**Rule.** Do not plumb enclosing DrawContext pointers into Graphite tasks for
debug task dumping (SK_DUMP_TASKS). Instead, print the Task instance's own
address ('this') to track task reuse and the target TextureProxy's label() and
pointer to identify draw context provenance.

**Why.** In Graphite, a DrawContext maintains a 1-to-1 relationship with its
target TextureProxy, making an explicit DrawContext pointer redundant when the
target proxy is already known. TextureProxy labels provide rich diagnostic
provenance (such as client SkSurface names or child device tags), while printing
the task pointer identifies task instances referenced across multiple recording
graph paths.

## Examples

Bad:

```cpp
#if defined(SK_DUMP_TASKS)
    explicit DrawTask(sk_sp<TextureProxy> target, DrawContext* recordingDC);
    DrawContext* fRecordingDC;
    void dump(int index, const char* prefix) const override {
        SkDebugf("%s%d: Draw Task (target=%p) (DC=%p)\n", prefix, index, fTarget.get(), fRecordingDC);
    }
#endif
```

Good:

```cpp
explicit DrawTask(sk_sp<TextureProxy> target);
#if defined(SK_DUMP_TASKS)
    void dump(int index, const char* prefix) const override {
        if (fTarget) {
            SkDebugf("%s%d: Draw Task=%p (Target=%p) (Label=%s)\n", prefix, index, this,
                     fTarget.get(), fTarget->label());
        } else {
            SkDebugf("%s%d: Draw Task=%p (Target=%p)\n", prefix, index, this, fTarget.get());
        }
    }
#endif
```
