---
name: skia/modules/skottie-return-builder-artifacts-in-result-struct
description: When an animation builder or parser returns a result struct, bundle new output artifacts into that struct instead of adding post-operation getter methods
rubric: design
severity: should_fix
---

**Rule.** When an animation builder or parser method produces and returns a
result struct holding output artifacts, extend that return struct with new
outputs instead of adding separate post-operation getter methods to the builder.
Ensure all parsed artifacts and subordinate managers are transferred directly
via the result struct.

**Why.** Returning all generated artifacts together in a result struct keeps the
builder or parser API cohesive and stateless across invocations, avoids temporal
coupling where callers must query stateful post-execution getters, and makes
ownership transfer explicit.

## Examples

Bad:

```cpp
// In AnimationBuilder:
const sk_sp<SlotManager>& getSlotManager() const { return fSlotManager; }

// At caller site:
auto ainfo = builder.parse(json);
fSlotManager = builder.getSlotManager();
```

Good:

```cpp
// In AnimationInfo result struct:
struct AnimationInfo {
    ...
    sk_sp<SlotManager> fSlotManager;
};

// At caller site:
auto ainfo = builder.parse(json);
fSlotManager = ainfo.fSlotManager;
```
