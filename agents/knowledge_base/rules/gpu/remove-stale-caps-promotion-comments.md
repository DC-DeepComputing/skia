---
name: skia/gpu/remove-stale-caps-promotion-comments
description: When promoting capability flags or state from backend subclasses to base Caps, update comments to remove stale references treating front-end Caps as a future destination
rubric: readability
severity: nit
---

**Rule.** When promoting capability flags or state from backend-specific
subclasses (such as VulkanCaps, DawnCaps, or MtlCaps) to the base Caps class,
update inline comments and TODOs to remove stale references treating the
front-end or base class as a future destination. Ensure comments directly
describe how the capability system utilizes the state rather than describing
speculative future migration to the class it already resides in.

**Why.** In Graphite and Ganesh GPU subsystems, the base Caps class represents
the front-end and backend-agnostic capability abstraction. Comments drafted in
backend-specific classes that mention moving state to 'front-end Caps' become
contradictory and confusing once the members are promoted to the base class,
misleading readers about where the capability is implemented.

## Examples

Bad:

```cpp
// Eventually, these should be moved to the front-end Caps so that it
// can create fewer pipelines in the first place.
bool fUseBasicDynamicState = false;
```

Good:

```cpp
// Eventually, these can be used to create fewer pipelines in the first
// place.
bool fUseBasicDynamicState = false;
```
