---
name: skia/modules/skottie-encapsulate-slot-tracking-in-dispatch-helpers
description: When binding animatable properties in Skottie, encapsulate slot ID extraction and SlotManager tracking inside dispatchProperty helpers instead of inlining boilerplate at attachment sites
rubric: design
severity: should_fix
---

**Rule.** In Skottie animation property builders, push slot ID (sid) extraction
and SlotManager tracking down into the dedicated dispatch*Property helper
functions (such as dispatchOpacityProperty or dispatchColorProperty) rather than
duplicating slot lookup and registration boilerplate across individual property
attachment call sites. Attachment methods should pass the JSON property node
directly to the dispatch helper and let it handle both property observer
dispatch and slot tracking.

**Why.** Centralizing slot extraction and SlotManager registration inside
property dispatch helpers avoids duplicating JSON property parsing logic across
multiple node attachment call sites, keeps attachment functions concise, and
ensures property observer notifications and slot registrations remain
consistently coordinated in one place.

## Examples

Bad:

```cpp
// In AnimationBuilder::attachOpacity()
auto dispatched = this->dispatchOpacityProperty(adapter->node(), jobject["o"]);
if (const skjson::ObjectValue* opacityProp = jobject["o"]) {
    if (const skjson::StringValue* slotID = (*opacityProp)["sid"]) {
        fSlotManager->trackScalarValue(SkString(slotID->begin()),
                                       &(adapter->fOpacity),
                                       adapter);
        dispatched = true;
    }
}
```

Good:

```cpp
// In AnimationBuilder::attachOpacity()
auto dispatched = this->dispatchOpacityProperty(adapter->node(), jobject["o"]);

// Inside AnimationBuilder::dispatchOpacityProperty(node, jprop):
if (jprop) {
    if (const skjson::StringValue* slotID = (*jprop)["sid"]) {
        fSlotManager->trackScalarValue(SkString(slotID->begin()), ...);
        dispatched = true;
    }
}
```
