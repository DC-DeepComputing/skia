---
name: skia/remove-superseded-private-enums
description: When refactoring methods to use shared parameter structs or common enums, remove superseded private enums and obsolete internal type declarations
rubric: readability
severity: should_fix
---

**Rule.** When refactoring methods or functions to accept shared parameter
structs, descriptors, or common enums, delete superseded private enums and
internal type declarations that are rendered obsolete. Do not retain unused
private enum definitions or type aliases in class definitions when all call
sites have migrated to shared types.

**Why.** Leaving superseded private enums or internal type definitions in place
creates dead code and visual clutter. Stale declarations mislead readers into
believing the class still relies on custom internal states or separate
synchronization mechanisms rather than the shared types.

## Examples

Bad:

```cpp
class GrVkGpu {
private:
    enum SyncQueue {
        kForce_SyncQueue,
        kSkip_SyncQueue
    };

    bool submitCommandBuffer(const GrSubmitInfo& submitInfo);
};
```

Good:

```cpp
class GrVkGpu {
private:
    // SyncQueue enum deleted after submitCommandBuffer migrated to GrSubmitInfo
    bool submitCommandBuffer(const GrSubmitInfo& submitInfo);
};
```
