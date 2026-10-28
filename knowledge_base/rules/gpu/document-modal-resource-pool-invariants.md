---
name: skia/gpu/document-modal-resource-pool-invariants
description: When maintaining dual resource pools or modal execution states in GPU managers, document why both modes exist and what transitions are required before switching
rubric: readability
severity: should_fix
---

**Rule.** When maintaining dual resource pools or modal execution states in GPU
managers (such as protected vs. non-protected command buffers), document the
class-level invariants and transition rules in the class header. Explicitly
explain why both modes are required within a single context, clarify which
operations route to each pool, and state any caller obligations (such as calling
submitToGpu or flushing pending work) before switching modes.

**Why.** Subtle GPU API and hardware constraints (such as forbidding protected
memory access in vertex shaders) often necessitate maintaining separate resource
pools even in protected contexts. Without clear header documentation of modal
requirements and transition boundaries, callers cannot deduce from method
signatures that intermixing modal operations without an intervening submit
causes invalid execution states or driver validation errors.

## Examples

Bad:

```cpp
class QueueManager {
public:
    virtual ~QueueManager();
    [[nodiscard]] bool addRecording(const InsertRecordingInfo&, Context*);
    [[nodiscard]] bool addTask(Task*, Context*, Protected);
    ...
private:
    std::vector<std::unique_ptr<CommandBuffer>> fAvailableCommandBuffers;
    std::vector<std::unique_ptr<CommandBuffer>> fAvailableProtectedCommandBuffers;
};
```

Good:

```cpp
/**
 * QueueManager manages command buffers and ensures proper submission ordering.
 * Supports both protected and non-protected command buffers within protected contexts
 * (e.g., vertex shaders forbid protected memory, requiring non-protected buffers and copies).
 * Intermixing protected and non-protected recording without submitting is unsupported;
 * callers must call submitToGpu before switching command buffer modes.
 */
class QueueManager {
public:
    virtual ~QueueManager();
    [[nodiscard]] bool addRecording(const InsertRecordingInfo&, Context*);
    [[nodiscard]] bool addTask(Task*, Context*, Protected);
    ...
private:
    std::vector<std::unique_ptr<CommandBuffer>> fAvailableCommandBuffers;
    std::vector<std::unique_ptr<CommandBuffer>> fAvailableProtectedCommandBuffers;
};
```
