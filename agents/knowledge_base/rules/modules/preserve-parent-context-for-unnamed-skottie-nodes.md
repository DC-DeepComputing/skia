---
name: skia/modules/preserve-parent-context-for-unnamed-skottie-nodes
description: When tracking scoped property or node context in Skottie, inherit the enclosing parent context instead of resetting to nullptr when child nodes lack identifier attributes
rubric: correctness
severity: should_fix
---

**Rule.** When managing hierarchical context in Skottie scoped trackers (such as
AutoPropertyTracker), fall back to inheriting the parent context (fPrevContext)
rather than resetting the active context to nullptr when a child element lacks
its own name or slot identifier attributes ('nm' or 'sid').

**Why.** Child elements such as color fills, strokes, or transforms often omit
explicit names or slot IDs. Resetting the observer context to null clobbers the
enclosing shape or layer name, preventing downstream property observers and
client callbacks from identifying the target element hierarchy.

## Examples

Bad:

```cpp
if (const skjson::StringValue* slotID = obj["sid"]) {
    fBuilder->fPropertyObserverContext = slotID->begin();
} else {
    const skjson::StringValue* name = obj["nm"];
    // Flaw: Resets context to nullptr for unnamed child elements, clobbering parent context
    fBuilder->fPropertyObserverContext = name ? name->begin() : nullptr;
}
```

Good:

```cpp
if (const skjson::StringValue* slotID = obj["sid"]) {
    fBuilder->fPropertyObserverContext = slotID->begin();
} else {
    const skjson::StringValue* name = obj["nm"];
    // Preserves enclosing scope context for unnamed children
    fBuilder->fPropertyObserverContext = name ? name->begin() : fPrevContext;
}
```
